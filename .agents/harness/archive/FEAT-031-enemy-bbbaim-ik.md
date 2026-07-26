# FEAT-031 敌人 BBBAimIK 瞄准系统

**状态：** done  
**创建：** 2026-06-13  
**关闭：** 2026-06-18-session40  
**最后更新：** 2026-06-18-session40

> 用户确认（session40）：敌人上半身瞄准 IK PIE 验证成功。Session35 因 BUG-030-002 的临时禁用已于 session40 随 FEAT-032 启用感知一并恢复（`HumanoidAIController` 的 `OnTargetPerceptionUpdated` 与 `Tick` 已重写，`bIsAiming` 在 Aim 状态下正常驱动）。
> ⚠️ 注：session40 重写的 `Tick` 用 `Player->GetActorLocation()` 写 `AimTargetWorld`（非本档原设计的 `GetActorEyesViewPoint`）。当前验证 OK；若日后觉得瞄得偏低，把 AIController Tick / `AHumanoidEnemy::AimAtTarget` 的取点改回眼睛位置即可。

---

## 目标

用 BBBAimIK 插件的 CCD Aim IK 节点驱动人形怪脊柱骨骼链（spine_01→spine_02→spine_03）在战斗状态下朝向玩家，实现上半身跟随瞄准效果。

---

## 架构决策

**沿用 ABP_FirearmBase 的相同模式，用于敌人骨骼链**

- `AimSourceBoneName = hand_r`（枪挂在此，必须是骨骼链末端的后代 ✓）
- `AimSourceLocalTransform`：BeginPlay 或首帧武器有效时计算一次（muzzle socket 相对 hand_r 的局部变换）
- `AimTarget`（组件空间）：每帧由 AIController 更新 `AimTargetWorld`，AnimInstance 转换
- `bIsAiming`：AIState == Combat 时为 true，控制 IK Alpha

**依赖 FEAT-032 完成**：`AimTargetWorld` 由 AIController 在发现玩家后每帧写入

---

## C++ 实现（Session33/34，已编译通过）

### HumanoidEnemy.h/.cpp
- 新增 `AimTargetWorld`（FVector，BlueprintReadWrite）
- 新增 `bIsAiming`（bool，BlueprintReadWrite）

### HumanoidAIController.cpp
- `Tick`：`GetActorEyesViewPoint` 获取玩家眼睛位置写入 `AimTargetWorld`（替代 GetActorLocation，避免瞄准腰部偏低）
- `bIsAiming = (Target != nullptr)`

### HumanoidEnemyAnimInstance.h/.cpp
- 新增 `AimTargetComponentSpace / AimSourceLocalTransform / AimAxisSocketName / AimAxis / bHasValidAimTarget / bIsAiming / AimAlpha / AimAlphaInterpSpeed`
- 首帧武器 Muzzle socket 有效时：计算 `AimSourceLocalTransform`；从 `AimAxisSocketName`（默认 `AimSocket`）插槽计算 `AimAxis`（hand_r 局部空间的 +X 方向）
- 每帧：AimTargetWorld → Component Space；AimAlpha FInterpTo 平滑

---

## 编辑器操作（Session34 完成）

**骨骼编辑器：**
- `hand_r` 下新增插槽 `AimSocket`，+X 轴朝向枪管方向

**ABP_HumanoidEnemy AnimGraph：**
```
PatrolSM → CS转换 → Two-Bone IK(hand_l) → BBBAimIK → LS转换 → Output
```
BBBAimIK 节点：
- BoneChain: spine_01(0.2) / spine_02(0.4) / spine_03(0.6)
- AimSourceBoneName: `AimSocket`（非 hand_r，绕过 UE4/UE5 骨骼轴向差异）
- AimAxis: `(1, 0, 0)`（固定，使用插槽的 +X）
- AimSourceLocalTransform ← 变量 `AimSourceLocalTransform`
- AimTarget ← 变量 `AimTargetComponentSpace`
- Alpha ← 变量 `AimAlpha`

---

## 关键决策记录

**AimSourceBoneName 用 AimSocket 而非 hand_r：**
UE4 Mannequin 的 `hand_r` 骨骼本地轴向与 UE5 不同，若用 `hand_r` + `AimAxis(1,0,0)` 会导致脊柱瞄准方向向右偏移。解决方案：在 `hand_r` 骨骼上添加子插槽 `AimSocket` 并手动设好朝向，BBBAimIK 用插槽名（`GetSocketTransform` 骨骼/插槽均支持）；AimAxis 固定 (1,0,0) 使用插槽的 +X，无需修改插件源码。

**AimTarget 用 GetActorEyesViewPoint：**
`GetActorLocation()` 返回 Capsule 中心（腰部），FPS 摄像机在头部，偏差约 60cm。改用 `GetActorEyesViewPoint` 获取眼睛/摄像机位置，瞄准高度正确。

---

## 待验证

- [ ] PIE：AIState=Aim 时脊柱链跟随玩家旋转，Patrol 时 AimAlpha 归 0 恢复中立（依赖 BUG-030-002 先修复）

---

## 临时禁用记录（Session35）

**原因：** Patrol 状态下 Two-Bone IK 出现手肘过伸，根因是单 socket 无法同时适配 Patrol / Aim 两种武器持枪位置（见 BUG-030-002）。在解决该问题前暂时关闭 BBBAimIK。

**修改位置：** `HumanoidAIController.cpp`

```cpp
// Tick：bIsAiming 强制 false
Enemy->bIsAiming = false;

// OnTargetPerceptionUpdated：函数体清空，感知不触发状态切换
```

**恢复条件：** BUG-030-002 两 socket 方案实现完成后，还原上述两处改动并重新 PIE 验证。

**→ 已恢复（Session40）：** 启用 FEAT-032 感知时重写了 `OnTargetPerceptionUpdated` 与 `Tick`（`bIsAiming` 在 Aim 下正常驱动），用户 PIE 确认上半身瞄准 IK 成功。详见顶部说明。

---

## Bug 日志

（无）
