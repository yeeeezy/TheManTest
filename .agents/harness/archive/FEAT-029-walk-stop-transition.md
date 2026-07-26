# FEAT-029 Walk→Stop 过渡优化

**状态：** done  
**创建：** 2026-06-13  
**最后更新：** 2026-06-13-session29  
**关闭：** 2026-06-13-session28（用户确认）

---

## 目标

修复人形怪巡逻行走到停步动画衔接不自然的问题。根本原因有两个：
1. 过渡时机不对：`bIsStopping` 一为 true 就立刻切换，不管走路动画走到哪一帧
2. 速度快照为零：`OnPatrolMoveCompleted` 触发时速度已归零，原 `FrozenSpeed = Speed` 快照到的是 0，混合空间立刻塌向 Idle

---

## 架构决策

**方案：三路径分治 + RequestDirectMove 维速 + 软保底防跳帧**

- 单一 Stopping 状态，4 个动画（Walk_RU / Walk_LU / Run_RU / Run_LU），ABP 用 `bReadyToStop` 作进入条件
- `bPendingStop` 门控：`bIsStoppingAtPoint` 上升沿置 true，等待脚步 Notify 触发才锁定索引并置 `bReadyToStop`
- `LastWalkSpeed`：非停步期且 Speed>10 时每帧更新，防止快照到 0
- **软保底**（取代已废弃的 FrozenSpeed 方案）：普通停步时，若 `Speed < LastWalkSpeed * 0.5f` 则 `Speed = LastWalkSpeed`，防混合空间一帧跳到 Idle

**三路径行为（Tick + AnimInstance 协同）**

| 场景 | Tick RequestDirectMove | AnimInstance 软保底 | bReadyToStop 触发 |
|---|---|---|---|
| 普通停步（等 Notify）| ✅ 维持 PatrolWalkSpeed | ✅ 防跳帧 | AnimNotify_FootDown |
| 扫视（WaitTime≥MinScan）| ❌ 不调用 | ❌ Speed 自然归零 | 立即设为 true，同帧计算 StoppingAnimIndex |
| 转身 | ❌ 不调用 | ❌ Speed 自然归零 | 不需要 |

**AnimNotify 方案：C++ UAnimNotify 类**

- `UAnimNotify_FootDown`，`bIsLeftFoot` 参数区分左右脚
- 触发时 cast `GetAnimInstance()` → `SetIsLeftFootForward(bIsLeftFoot)`
- Walk 动画需各放一个左脚 Notify（`bIsLeftFoot=true`）和右脚 Notify（`bIsLeftFoot=false`），让 bIsLeftFootForward 每步更新

**StoppingAnimIndex 公式**

```
(bStoppingFromRun ? 2 : 0) + (bIsLeftFootForward ? 0 : 1)
// 左脚落地（左脚在地，右脚在空）→ Walk_RU(0) / Run_RU(2)
// 右脚落地（右脚在地，左脚在空）→ Walk_LU(1) / Run_LU(3)
```

- 普通停步：`bPendingStop=true` 时由 `SetIsLeftFootForward()` 计算
- 扫视停步：进入扫视分支时立即用当前 `bIsLeftFootForward` + `LastWalkSpeed` 计算（`bPendingStop` 在同帧设置后被清除，无法走 Notify 路径）

---

## C++ 实现（已完成）

### 新建文件

| 文件 | 说明 |
|---|---|
| `Public/Characters/Enemy/Humanoid/AnimNotify_FootDown.h` | UAnimNotify 子类，bIsLeftFoot 参数 |
| `Private/Characters/Enemy/Humanoid/AnimNotify_FootDown.cpp` | cast AnimInstance → SetIsLeftFootForward |

### 修改文件

| 文件 | 变更 |
|---|---|
| `HumanoidEnemy.h` | 已有（无新增） |
| `HumanoidEnemy.cpp` | include HumanoidEnemyAnimInstance.h；Tick 加 RequestDirectMove（bIsStoppingAtPoint && !bPendingTurn && !bIsPatrolScanning 时维持速度） |
| `HumanoidEnemyAnimInstance.h` | 新增 bIsLeftFootForward / bReadyToStop / bIsStopping / bStoppingFromRun / StoppingAnimIndex / RunSpeedThreshold；私有 bPendingStop / LastWalkSpeed；公开 SetIsLeftFootForward() / IsReadyToStop() |
| `HumanoidEnemyAnimInstance.cpp` | SetIsLeftFootForward 门控逻辑；NativeUpdateAnimation 软保底（排除 bIsTurning / bIsPatrolScanning）；扫视分支立即计算 StoppingAnimIndex + bReadyToStop |

### 核心逻辑流

```
普通停步：
  敌人到达路点 → bIsStoppingAtPoint=true
  Tick：RequestDirectMove(ForwardVec * PatrolWalkSpeed)  ← 维持物理速度
  AnimInstance 软保底：Speed ≥ LastWalkSpeed（防跳帧）
  Walk 动画播到 Notify 帧 → SetIsLeftFootForward() → bReadyToStop=true
  Tick：IsReadyToStop()=true → 停止 RequestDirectMove → CMC 正常减速
  ABP：bReadyToStop==true → 进入 Stopping

扫视停步：
  bIsStoppingAtPoint=true，bIsPatrolScanning=true
  Tick：跳过 RequestDirectMove（!bIsPatrolScanning 为 false）
  AnimInstance：软保底跳过（bIsPatrolScanning=true）→ Speed 自然归零
  同帧：StoppingAnimIndex 用当前 bIsLeftFootForward 计算，bReadyToStop=true
  ABP：bReadyToStop==true → 进入 Stopping → Idle → Scan
```

---

## 待完成（蓝图 / 编辑器操作）

- [x] 编译 C++（Development Editor / Win64）
- [x] ABP `ABP_HumanoidEnemy`：Stopping 进入条件确认为 `bReadyToStop == true`
- [x] ABP：Walk→Stopping 过渡改用 Inertialization（消除位置跳变）
- [x] ABP：新增 Walk→Idle 过渡（Speed < 10，摩擦力测试遗留，已保留）
- [ ] Walk 动画序列放置两个 `AnimNotify_FootDown`：左脚落地帧（`bIsLeftFoot=true`）、右脚落地帧（`bIsLeftFoot=false`）
- [ ] 确认 Stopping 动画设计速度是否接近 PatrolWalkSpeed = 150 cm/s（看文件名是 Walk_Stop 还是 Run_Stop）
- [ ] 若速度不匹配：考虑 Sync Group 方案（Walk BlendSpace = Leader，Stopping = Follower，加 LeftFootDown/RightFootDown Sync Marker）
- [ ] PIE 全流程验证：Walk→Stop 衔接自然无绊脚感；扫视正常进入；转身 bIsTurning 正常

---

## Bug 日志

### BUG-029-001 Walk→Stopping 位置跳变（已缓解，残留绊脚感）

**Session27 更新：**
- **尝试过**：摩擦力方案（注释 RequestDirectMove + 软保底）→ AI PathFollowing 在回调前已将速度降到 0，BrakingDecelerationWalking 无法生效，方案放弃
- **尝试过**：`Velocity = FVector::ZeroVector`（bReadyToStop 时清速度）→ 造成绊脚感，已撤回
- **有效改善**：ABP Walk→Stopping 过渡改用 **Inertialization**，位置跳变基本消除
- **残留问题**：过渡时仍有轻微绊脚感（"被拌了一下"）

**当前疑似根因：** Stopping 动画设计入口速度与 `PatrolWalkSpeed = 150 cm/s` 不匹配（动画可能是给跑步设计的），Root Motion 减速曲线和实际速度不一致。

**下一步修复方向：**
1. 确认 Stopping 动画文件名/设计速度（Walk_Stop vs Run_Stop）
2. 若速度不匹配 → 用 **Sync Group** 让 Walk 和 Stopping 自动对齐脚相
   - Walk BlendSpace：Sync Group = `LowerBody`，Role = `Can Be Leader`
   - Stopping 动画：Sync Group = `LowerBody`，Role = `Follower`
   - 两套动画加相同 Sync Marker：`LeftFootDown` / `RightFootDown`

---

## 已知风险

- `RequestDirectMove` 维速期间角色会在路点附近轻微过冲（Walk 动画完成一步约 0~150 cm），可接受。
- Walk 动画若无 AnimNotify，普通停步会永久等待：`bReadyToStop` 不会置 true，角色无限 RequestDirectMove。需确保编辑器里放了 Notify。
