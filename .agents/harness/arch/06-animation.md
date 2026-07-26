# 动画实例

**何时读取：** 新增动画变量、修改动画状态机所需的驱动参数时。

> 完整 ABP 架构（层结构、Slot、节点流程、扩展策略、旧双骨骼系统）见 `.agents/harness/arch/12-anim-blueprint.md`

**基类（骨骼无关）：**

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Animation/BaseLocomotionAnimInstance.h` | 所有动画实例的 C++ 基类；输出 Speed / Velocity_Z（垂直速度，上升+下落-）/ bIsFalling（离地即 true）/ AimPitch（[-1,1] 归一化）/ Direction |

> **session47 起 `AimPitch` 是玩家上下瞄准的唯一来源**：身体已关物理俯仰（`bUseControllerRotationPitch=false`），看上下不再转 mesh，须由 ABP 用 `AimPitch` 驱动上半身 AimOffset/脊柱 Modify Bone 绕肩俯仰（FEAT-039）。`bIsFalling`/`Velocity_Z` 驱动跳跃状态机过渡。

**玩家动画：**

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h` | 玩家动画实例（FEAT-041 由 `UFPSArmsAnimInstance` 改名）。挂在 **`GetMesh()`**，驱动身体/影子/腿三件套共享的全身姿势。session63 改为 UE 模板式普通 locomotion：Idle 与 Walk/Run BlendSpace 按 `Speed` / `Direction` 直接混合，不再做专门停步动画。当前子类只在基类变量之外补 `AccelDirection` / `bHasAcceleration`，保留为玩家专属扩展点。DefaultEngine.ini 有 CoreRedirect 保旧 ABP 父类链接 |
| `Source/TheManTest/Public/Equipment/Animation/EquipmentAnimInstance.h` | 装备/武器动画层通用父类。输出 Speed / Direction / Velocity_Z / bIsFalling，供通用装备上半身 Idle/Walk/Run/Jump 动画层使用。FEAT-046 session75 回退为普通 1D BlendSpace 方案后，不再暴露 Start/End 状态机专用临时变量 |
| `Source/TheManTest/Public/Equipment/Firearms/FirearmAnimInstance.h` | 4 个 AimIK 变量：AimSourceLocalTransform / AimTargetComponentSpace / bHasValidAimTarget / bIsAiming |

**玩家 locomotion（session63 简化）：**
- 不再使用 `StopAnimIndex` / `bShouldStop` / `bShouldMove` / 脚相位 / 延迟停 / 保持原速滑行。
- ABP 应采用最小状态机：`Idle <-> WalkRun BlendSpace`，常用条件为 `Speed > 3` 与 `Speed <= 3`；跳跃可继续用 `bIsFalling` / `Velocity_Z`。
- `UFPSCharacterAnimInstance` 继承 `UBaseLocomotionAnimInstance`，保留 `AccelDirection` / `bHasAcceleration` 给起步 Lean 或后续玩家专属动画使用。若 ABP 不需要 Lean，可以完全不读这两个变量。
- session64+66 新增玩家原地转身触发变量：`bIsTurningInPlace` / `TurnInPlaceAngle` / `TurnInPlaceIndex`（0=左45，1=右45）/ `TurnInPlacePlayRate`。触发逻辑由 `AFPSCharacterBase` 的 `BodyVisualYaw` 驱动：Pawn/相机实时跟控制器 yaw，下半身/影子用 `BodyRoot` 的视觉 yaw，空闲低速时先滞后，超过 `BodyTurnInPlaceThreshold`（默认 30）后锁定一次 45 度左/右目标 yaw，不再追实时相机，转完后再用新的腿部朝向与相机朝向判断下一次。视觉 yaw 的推进不再由 C++ 匀速估算，而是由 `UFPSCharacterAnimInstance` 读取 Turn 动画上的 `TurnRootYaw` 曲线帧差并调用 `AFPSCharacterBase::ApplyBodyTurnRootYawDelta()`；左转曲线应为 `0 -> -45`，右转曲线应为 `0 -> +45`。`BodyTurnInPlacePlayRate` 同时缩放 C++ 锁定时长和 ABP Sequence Player Play Rate。
- 若蓝图仍报缺变量，说明 ABP 里残留 FEAT-039 停步节点，需要删除 Stop 状态、Stop 过渡和相关变量引用。

**人形怪动画：**

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.h` | 继承基类；AIState / bIsTurning / TurnAngle / TurnAnimIndex；bIsPatrolScanning / PatrolScanAnimIndex；**无 Stopping 状态**：StopDecelerationRate / VirtualSpeed / bIsVirtualDecelerating 驱动虚拟减速；**AimIK**：AimTargetComponentSpace / AimSourceLocalTransform / AimAxisSocketName / AimAxis / bHasValidAimTarget / bIsAiming / AimAlpha / AimAlphaInterpSpeed；**左手 IK**：LeftHandIKTarget / bHasWeapon / WeaponGripLeftSocket |
| `Source/TheManTest/Private/Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.cpp` | NativeUpdateAnimation：巡逻停车（bIsStoppingAtPoint 上升沿）和 Aim 停车（RealSpeed 骤降，bIsAimStopping 标志）共用同一套虚拟减速；bAimSourceInitialized 首帧从武器 `Muzzle` socket 计算 AimSourceLocalTransform，同时从骨骼 `AimAxisSocketName`（默认 `AimSocket`，挂在 `hand_r` 下，+X 朝枪管方向）计算 AimAxis；AimAlpha 用 FInterpTo 平滑 |
| `Source/TheManTest/Public/Characters/Enemy/Humanoid/AnimNotify_FootDown.h` | 已弃用空壳（FEAT-029 移除 Stopping 状态后废弃），代码保留 |

**ABP_HumanoidEnemy AnimGraph 节点链：**

```
PatrolSM（状态机）
  → Convert to Component Space
  → Two-Bone IK（hand_l，左手贴前握把）
      IKBone = hand_l
      Joint Target Space = Bone Space，Joint Target Bone = lowerarm_l，Location = (0,0,0)
      Effector = LeftHandIKTarget（Component Space）
      Alpha = bHasWeapon（float）
  → BBBAimIK（脊柱瞄准）
      AimSourceBoneName = AimSocket（hand_r 子插槽，+X 朝枪管）
      AimAxis = (1,0,0)
      AimSourceLocalTransform ← AimSourceLocalTransform 变量
      AimTarget ← AimTargetComponentSpace 变量
      Alpha ← AimAlpha 变量
  → Convert to Local Space
  → Output Pose
```

**ABP_HumanoidEnemy 状态机结构（PatrolSM 内）：**

```
SA_PatrolStates（State Alias，含巡逻类状态）→ Aim：AIState == Aim
SA_AnyState（含所有状态含 Aim）→ Dead：IsDead
Aim → Idle：AIState != Aim，Blend 0.2s

内部状态：
  Idle / Patrol_Walk / Scan / Turn系列  ← 巡逻层（SA_CanTurn 子状态机）
  Aim                                   ← 锁定玩家状态，使用 BS_HumanoidCombat（Direction/Speed）
  Dead                                  ← 死亡
```

**EHumanoidEnemyAIState 枚举（HumanoidEnemyTypes.h）：**

| 值 | 含义 |
|---|---|
| Patrol | 巡逻（C++ 路点循环自驱） |
| Aim | 锁定玩家追击（BT 接管，SetFocus 驱动朝向） |
| SearchRush | 冲向消失点（预留） |
| SearchScan | 原地晃头搜索（预留） |
| Dead | 死亡 |

**已删除（FEAT-041）：** 旧 `UTheManAnimInstanceBase`（绑定旧 `ATheManCharacterBase`）已删除。对应旧 ABP（`ABP_MainCharacter` / `ABP_FirstPerson_MainCharacter`）需在编辑器一并删除。
