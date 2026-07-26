# 动画蓝图架构（ABP 层 / Slot / 节点图）

**何时读取：** 搭建或修改 ABP 层结构、Linked Anim Layer、Slot 蒙太奇插槽、AimIK 节点链、武器动画扩展时。

> 本文是 `06-animation.md` 的详细版：06 速查 C++ AnimInstance 类与变量，本文讲 ABP 资产的层/Slot/节点图与扩展策略。
> **当前 ABP = 单一骨骼（FPS Arms）。** 旧双骨骼系统的 C++ 已于 FEAT-041 删除（文末旧系统节仅作历史参考）。

> 当前方向（session63）：不再使用 Motion Matching，也不再做专门停步动画。玩家全身主 ABP 走 UE 模板式普通 locomotion：Idle 与 Walk/Run BlendSpace 直接按 `Speed` / `Direction` 混合；跳跃用 `bIsFalling` / `Velocity_Z`。`HeadCamera -> ViewmodelRoot -> ArmsViewMesh` 独立 FP 手臂结构保留，装备/开火蒙太奇仍通过 `GetArmsMesh()` 走 FP 手臂。

---

## 总体原则

- **当前 ABP 系统：纯第一人称手臂（FPS Arms，单一骨骼）**，Locomotion 共享基础动画，武器差异化通过 Linked Anim Layer 注入。
- 武器动画遵循 **“共享基础 + 按需覆盖”**：普通武器只实现持枪姿态和瞄准层，特殊武器（需要专属跑跳动画）在其 Layer 内自带 StateMachine。
- **旧双骨骼系统（`ATheManCharacterBase` / `UTheManAnimInstanceBase`）C++ 已删（FEAT-041）**；`ABP_MainCharacter` 等旧蓝图待编辑器删除。文末旧节点图仅作历史参考。

---

## 核心文件（当前活跃）

| 资产 / 类 | 路径 | 作用 |
|---|---|---|
| `ABP_FPS_Arm_MainCharacter` | `Content/Characters/CharacterBase/Animations/Logic/` | FPS 手臂基础动画蓝图，含 Locomotion StateMachine 和层路由 |
| `ALI_WeaponAnim` | `Content/Weapons/_Shared/Animations/Interface/` | 武器动画层接口，定义两个层：`WeaponAimOffset`、`WeaponUpperBody` |
| `ABP_FirearmBase` | `Content/Weapons/_Shared/Animations/Firearm/` | 枪械动画层实现基类，内含 AimIK 节点 |
| `UFPSCharacterAnimInstance` | `Source/.../Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h` | 玩家 ABP 的 C++ 父类（FEAT-041 由 `UFPSArmsAnimInstance` 改名），持有 Locomotion 变量（继承基类）；CoreRedirect 保旧链接 |
| `UFirearmAnimInstance` | `Source/.../Equipment/Firearms/FirearmAnimInstance.h` | `ABP_FirearmBase` 的 C++ 父类，持有四个 AimIK 变量 |

> 敌人 ABP（`ABP_HumanoidEnemy`）见下方“敌人动画架构”节及 `11-enemy-ai.md`。

---

## ABP_BodyLocomotion（玩家全身主 ABP，普通 locomotion）

> `ABP_BodyLocomotion` 是项目自己的玩家全身主 ABP，不应因为放弃 Motion Matching 而直接删除。若它曾在实验中加入 `PoseSearchHistoryCollector` / Motion Matching / Stop 状态，需要在编辑器里清掉这些节点，保留为普通 locomotion 模板。父类推荐继续用 `UFPSCharacterAnimInstance`，该类现在是很薄的玩家专属子类，只比 `UBaseLocomotionAnimInstance` 多 `AccelDirection` / `bHasAcceleration`。

**路径：** `Content/Characters/CharacterBase/Animations/Logic/ABP_BodyLocomotion`

**推荐 LocomotionSM：**

```
Idle ──(Speed>3)──> Run/Walk(BS Direction×Speed) ──(Speed<=3)──> Idle
State Alias "To Falling"（= Idle / Run/Walk）
   ├─(bIsFalling && Velocity_Z>0)─> Jump_Start ─(JumpStart 剩余时间<0.1)─> Jump_Loop
   └─(bIsFalling && Velocity_Z<=0)────────────────────────────────────> Jump_Loop
State Alias "To Land"（= Jump_Loop）─(自动规则: 落地)─> Jump_End
Jump_End ─(Speed<3 && Land 剩余<0.3)─> Idle ；─(Speed>3)─> Run/Walk
```

- **Jump_Start / Jump_Loop / Jump_End** 三状态各用 Sequence Player 播 JumpStart / Fall_Loop / Land（子 ABP Asset Override 填各角色动画；模板里不填）。
- **不再使用 Stop 状态**：删除 `StopAnimIndex`、`bShouldStop`、`bShouldMove`、`bLeftFootUp`、`StopDirectionIndex` 等变量节点和过渡。
- ⚠️ **状态机直接播的 clip 必须 No Additive**（被当差值叠加 → 塌进地里，见 BUG-039-002，最初 Land 动画踩过）。
- **根运动**：普通 Idle/WalkRun 方案建议 `Root Motion from Montages Only`，地面移动交给 CharacterMovement。原地 locomotion clip 的 Enable Root Motion 应关闭。
- AnimGraph 输出链（待 FEAT-039 后半段）：LocomotionSM → Layered blend per bone(spine 起) 叠加上半身武器 Linked Layer（`ALI_WeaponAnim`，含 BBBAimIK + `AimPitch` 驱动俯仰）。

---

## ABP_FPS_Arm_MainCharacter AnimGraph 流程

> ⚠️ 旧手臂主 ABP，FEAT-039 全身主 ABP（上方 `ABP_BodyLocomotion`）验证通过后将取代它。下面节点图在迁移期仍作参考。

```
Locomotion StateMachine（Idle / Run/Walk / Jump_Start / Jump_Loop / Jump_End）
         │
         ▼
SaveCachedPose "Cache_Locomotion" → UseCachedPose
         │
         ▼
Slot "DefaultSlot"          ← 全身蒙太奇（装备切换等通用动作）
         │
         ▼
WeaponAimOffset (LinkedAnimLayer, ALI_WeaponAnim)   ← 输入 WeaponAimInPose ← DefaultSlot.Pose
         │
         ▼
Slot "UpperBodySlot"        ← 上半身武器动作蒙太奇（开火、换弹）
         │
         ▼
WeaponUpperBody (LinkedAnimLayer, ALI_WeaponAnim)   ← 输入 UpperBodyInPose ← UpperBodySlot.Pose
         │
         ▼
Slot "FullBodySlot"         ← 全身覆盖蒙太奇（预留，死亡/特殊动作）
         │
         ▼
Output Pose (Root)
```

主 ABP 不含任何 IK 节点，AimIK 完全封装在武器层（`ABP_FirearmBase`）内。

---

## 武器动画扩展策略

### 装备动画父类选择

```text
UEquipmentAnimInstance
  └─ UFirearmAnimInstance
```

- 所有装备通用上半身动画层优先继承 `UEquipmentAnimInstance`，可直接读取 `Speed` / `Direction` / `Velocity_Z` / `bIsFalling`。
- FEAT-046 session75 起，步枪上半身回退为普通 1D BlendSpace：保留 `SM_FirearmUpperBody` 壳，内部用 `Idle <-> Locomotion`，`Locomotion` 中的 1D BlendSpace Player 只读 `Speed`。不要再依赖已删除的 Start/End 状态机专用变量。
- 枪械动画层继承 `UFirearmAnimInstance`，在通用移动变量之外额外获得 AimIK 变量：`AimSourceLocalTransform` / `AimTargetComponentSpace` / `bHasValidAimTarget` / `bIsAiming`。
- 步枪 Template ABP 若需要 BBBAimIK，应以 `UFirearmAnimInstance` 为父类；非枪械装备可用 `UEquipmentAnimInstance`。

### 普通武器（共享 Locomotion）
- 新建武器 AnimBP，实现 `ALI_WeaponAnim`。
- `WeaponAimOffset` 层：持枪姿态 + 瞄准 IK，直接用 `WeaponAimInPose` 输入。
- `WeaponUpperBody` 层：上半身持握叠加。
- 跑跳动画自动复用基础 ABP 的 StateMachine。

### 特殊武器（需专属跑跳动画）
- `WeaponAimOffset` 层内不用 `WeaponAimInPose` 输入，自带完整 StateMachine（武器专属 Idle/Walk/Run/Jump）。

### Slot 使用约定

| Slot 名 | 蒙太奇类型 | 示例 |
|---|---|---|
| `DefaultSlot` | 全身，在瞄准前播放 | 装备切换、死亡 |
| `UpperBodySlot` | 上半身，瞄准 IK 后叠加 | 开火、换弹 |
| `FullBodySlot` | 全身覆盖，最高优先级 | 特殊动作（预留） |

> Slot 名称需在 ArmsMesh 使用的骨骼资产的 **Anim Slot Manager** 中注册。

---

## ALI_WeaponAnim 接口层

| 层名 | 输入引脚 | 作用 |
|---|---|---|
| `WeaponAimOffset` | `WeaponAimInPose`（Pose） | 武器持枪姿态 + 瞄准 IK |
| `WeaponUpperBody` | `UpperBodyInPose`（Pose） | 上半身武器持握叠加 |

## 玩家身体/手臂共用主 ABP + 双 mesh 武器层（session70）

`GetMesh()` 和 `ArmsViewMesh` 可以使用同一个主 ABP（如 `ABP_BodyLocomotion`），因为两者骨骼一致。`EquipmentAnimLayerClass` 装备时同时链接到这两个 mesh 的 AnimInstance：

```
GetMesh.AnimClass      = ABP_BodyLocomotion
ArmsViewMesh.AnimClass = ABP_BodyLocomotion

ABP_BodyLocomotion:
  Locomotion
  → WeaponAimOffset（Linked Anim Layer，装备时替换）
  → WeaponUpperBody（Linked Anim Layer，装备时替换）
  → Output Pose
```

- `ArmsViewMesh` 与 `GetMesh()` 各自有独立 AnimInstance。BBBAimIK 使用同一相机射线目标，但会分别转换到各自 Component Space，因此第一人称手臂和身体/影子都能瞄准相机指向位置。
- `ShadowBodyMesh` / `LegsMesh` 不设置自己的 AnimClass，继续 `SetLeaderPoseComponent(GetMesh())`。
- 不使用 `Copy Pose From Mesh`。武器切换仍靠武器 BP 的 `EquipmentAnimLayerClass`。

---

## UFPSCharacterAnimInstance 变量

继承 `UBaseLocomotionAnimInstance`：

| 变量 | 类型 | 说明 |
|---|---|---|
| `Speed` | `float` | 地面移动速度（Size2D） |
| `Velocity_Z` | `float` | 垂直速度 |
| `bIsFalling` | `bool` | 是否在空中 |
| `AimPitch` | `float` | 瞄准俯仰角，归一化到 [-1, 1] |
| `Direction` | `float` | 移动方向角 [-180, 180] |

**玩家专属追加（session63 清理后）**：

| 变量 | 类型 | 说明 |
|---|---|---|
| `AccelDirection` | `float` | 加速度方向角 [-180,180]，可选用于起步 Lean |
| `bHasAcceleration` | `bool` | 当前是否有移动输入加速度 |
| `bIsTurningInPlace` | `bool` | 角色 `BodyVisualYaw` 正在补向 PawnYaw 时触发原地转身状态 |
| `TurnInPlaceAngle` | `float` | 触发原地转身的有符号累计角，负数=左，正数=右 |
| `TurnInPlaceIndex` | `int32` | 0=左 45 度，1=右 45 度 |
| `TurnInPlacePlayRate` | `float` | 接到 Turn_L45 / Turn_R45 Sequence Player 的 Play Rate；同一个值也缩放 C++ 重触发锁定时长 |
| `TurnRootYawCurveName` | `FName` | 默认 `TurnRootYaw`；AnimInstance 每帧读取该曲线的增量驱动 `BodyVisualYaw` |
| `TurnRootYawPoseFixRotation` | `FRotator` | 等于 `-TurnRootYaw` 的 yaw Rotator。仅用于 `LocomotionSM` 外侧全局 `Transform Modify Bone(root)`，不要放进 Turn 状态内部 |

> 走跑档由 `AFPSCharacterBase::StartSprint/StopSprint` 改 `CharacterMovement->MaxWalkSpeed`，ABP 通常只读 `Speed` 进入同一个 Walk/Run BlendSpace。

**原地转身（session66 曲线驱动版）：**
- 使用 `Content/RTG/RTG_W2_Stand_Aim_L_45` 与 `Content/RTG/RTG_W2_Stand_Aim_R_45` 这类非 Root Motion 的 45 度转身资产。资产必须带 `TurnRootYaw` 曲线：左转 `0 -> -45`，右转 `0 -> +45`。
- `LocomotionSM` 中从 `Idle` 进入 Turn 状态条件为 `bIsTurningInPlace && TurnInPlaceIndex == 0/1`。Turn 状态可按 `Relevant Anim Time Remaining < 0.1` 正常回 `Idle`；C++ 会在 Turn 结束时把 `BodyVisualYaw` 对齐到触发时锁定的 `PawnYaw`，因此回 Idle 的落点就是最终朝向。Sequence Player 建议非循环。
- Turn Sequence Player 的 `Play Rate` 接 `TurnInPlacePlayRate`。角色 BP 只调 `BodyTurnInPlacePlayRate`，它会同时影响曲线播放速度和 C++ 下一段 45 度 turn 的重触发锁定时长。
- 若 turn 资产姿势里本身带 root bone yaw，必须把抵消放在 `LocomotionSM` 外面对最终混合姿势全局应用一次：`LocomotionSM -> Local To Component -> Transform Modify Bone(root) -> Component To Local -> 后续 Slot/Layer/Output`。`Transform Modify Bone` 设置 `Rotation Mode=Add to Existing`，`Rotation` 接 `TurnRootYawPoseFixRotation`。不要把该节点放进 `Turn_L45` / `Turn_R45` 状态内部，否则 Turn->Idle 过渡时抵消会随状态权重混掉，出现“回 Idle 再拉回”。Turn 状态内部只保留对应 Sequence Player。
- 当前 `AFPSCharacterBase` 仍 `bUseControllerRotationYaw=true`，胶囊体/相机跟随控制器 yaw；下半身/影子由 `BodyRoot` 使用 `BodyVisualYaw` 视觉朝向。触发 Turn 时 C++ 锁定当前 `PawnYaw` 为 `BodyTurnTargetYaw`，用 `TurnRootYaw` 曲线帧差作为速度曲线，并按 `abs(YawDelta) / BodyTurnInPlaceStepAngle` 缩放曲线增量；Turn 结束时 `BodyVisualYaw` 直接对齐锁定目标。方向过滤会忽略状态退出时曲线归零产生的反向增量。
- Root Motion 版本暂不启用，避免和胶囊体旋转叠加。

---

## ABP_FirearmBase（WeaponAimOffset 层实现）内部流程

```
WeaponAimInPose（输入 Pose）
         │  LocalToComponentSpace
         ▼
BBBAimIK 节点
  ├─ BoneChain：spine_01(0.2) → spine_02(0.4) → spine_03(0.6)
  ├─ AimSourceBoneName：hand_r
  ├─ AimAxis：(1, 0, 0)
  ├─ AimSourceLocalTransform ← UFirearmAnimInstance.AimSourceLocalTransform
  ├─ AimTarget            ← UFirearmAnimInstance.AimTargetComponentSpace
  ├─ bHasValidAimTarget   ← UFirearmAnimInstance.bHasValidAimTarget
  └─ Alpha                ← UFirearmAnimInstance.bIsAiming（bool→float）
         │  ComponentToLocalSpace
         ▼
输出 Pose
```

> 玩家武器层用 `AimSourceBoneName = hand_r`；敌人因 UE4/UE5 骨骼轴向差异改用 `hand_r` 子插槽 `AimSocket`（见敌人节 / `11-enemy-ai.md`）。

## UFirearmAnimInstance 变量

| 变量 | 类型 | 更新时机 | 说明 |
|---|---|---|---|
| `AimSourceLocalTransform` | `FTransform` | 装备时写入一次 | 枪口相对 hand_r 骨骼的局部偏移 |
| `AimTargetComponentSpace` | `FVector` | 每帧 NativeUpdateAnimation | 目标在 Component Space 的坐标 |
| `bHasValidAimTarget` | `bool` | 每帧 | 射线是否命中有效目标 |
| `bIsAiming` | `bool` | 每帧 | 是否处于瞄准状态，控制 IK Alpha |

---

## 武器动画扩展指南

- **新增不需要 AimIK 的武器**：新建该武器 `WeaponAimOffset` 层实现，直接 pass-through 输入 Pose，不放 BBBAimIK 节点。
- **新增需要 AimIK 的武器**：让该武器动画层 ABP 继承（或直接复用）`ABP_FirearmBase`。
- **新增需要专属跑跳的武器**：在 `WeaponAimOffset` 层内自建 StateMachine，忽略 `WeaponAimInPose` 输入。
- **修改骨骼链权重**：在 `ABP_FirearmBase` 的 BBBAimIK 节点 BoneChain 属性里调各骨骼 Weight。

---

## 敌人动画架构（人形怪）

### 总体原则

- **Template ABP 不绑骨骼**：`ABP_BaseLocomotion` 作为无骨骼 Template，所有角色/敌人 ABP 继承它，状态机逻辑零重复。
- **人形怪 ABP 用 Asset Override 填动画**：子 ABP 在 Asset Override Editor 覆盖各 Sequence Player 资产，骨骼在子 ABP 指定。
- **C++ AnimInstance 驱动变量**：蓝图状态机只读变量，逻辑判断在 C++。
- **无 Stopping 状态**：Walk→Idle 通过虚拟减速（`VirtualSpeed`）在 blend space 内平滑完成。
- **Turn 用原地动画 + 代码驱动旋转**。

### 核心文件（人形怪）

| 资产 / 类 | 路径 | 作用 |
|---|---|---|
| `ABP_BaseLocomotion` | `Content/Characters/CharacterBase/Animations/Logic/` | Template ABP，无骨骼，Locomotion 状态机框架 |
| `ABP_HumanoidEnemy` | `Content/Enemy/Phantom/Animations/Logic/`（ABP_Phantom 等） | 人形怪 ABP，Parent Class = `UHumanoidEnemyAnimInstance` |
| `UBaseLocomotionAnimInstance` | `Source/.../Characters/Animation/BaseLocomotionAnimInstance.h` | 所有 AnimInstance 的 C++ 基类 |
| `UHumanoidEnemyAnimInstance` | `Source/.../Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.h` | 人形怪专属 AnimInstance |
| `UAnimNotify_TurnComplete` | `Source/.../Characters/Enemy/Humanoid/AnimNotify_TurnComplete.h` | Turn 动画末尾 → `AHumanoidEnemy::OnTurnComplete()` |

### ABP_HumanoidEnemy AnimGraph 节点链

```
PatrolSM（状态机）
  → Convert to Component Space
  → Two-Bone IK（hand_l 左手贴前握把）
      IKBone=hand_l；Joint Target=Bone Space/lowerarm_l；Effector=LeftHandIKTarget（CS）；Alpha=bHasWeapon
  → BBBAimIK（脊柱瞄准）
      AimSourceBoneName=AimSocket（hand_r 子插槽，+X 朝枪管）；AimAxis=(1,0,0)
      AimSourceLocalTransform ← 变量；AimTarget ← AimTargetComponentSpace；Alpha ← AimAlpha
  → Convert to Local Space
  → Output Pose
```

### PatrolSM 状态机结构

```
SA_CanTurn（State Alias = Idle + Patrol_Walk_Run）
   → Turn_L45/L90/L135 / TurnAround / R45/R90/R135 : bIsTurning && TurnAnimIndex==0..6
SA_Turning（全部 Turn 状态）→ Idle : bIsTurning==false（AnimNotify_TurnComplete 触发）
Idle ↔ Patrol_Walk_Run : Speed >/< 10（虚拟减速驱动，无 Stopping 状态）
Idle ↔ PatrolScan : bIsPatrolScanning
SA_PatrolStates → Aim : AIState==Aim（Aim 状态用 BS_HumanoidCombat，Direction/Speed 驱动）
Aim → Idle : AIState!=Aim（Blend 0.2s）
Any State → Dead : bIsDead
```

### Patrol_Walk_Run（BlendSpace1D，X=Speed）
| Speed | 动画 |
|---|---|
| 0 | Idle（过渡缓冲） |
| ~150 | Walk 循环 |
| ~350 | Run 循环（预留） |

**停步**：PathFollowing 到点强制 CMC 速度归零；AnimInstance 检测 `bIsStoppingAtPoint` 上升沿，`VirtualSpeed` 从 `LastWalkSpeed` 以 `StopDecelerationRate`(默认 300 cm/s²) 线性降到 0 并覆盖 `Speed`，blend space 平滑过渡，Speed<10 自动切 Idle。Aim 停车（速度骤降）共用同一套虚拟减速。

### PatrolScan（Blend Poses by Int，Active Child=PatrolScanAnimIndex）
- C++ 在 `bIsPatrolScanning` 上升沿随机生成 index（1 ~ `PatrolScanAnimCount`，从 1 避免 A-Pose）。
- `PatrolScanAnimCount` 在子 ABP Class Defaults 设置。

### Turn（7 个原地动画，无 Root Motion）
| TurnAnimIndex | 状态 | 角度范围 |
|---|---|---|
| 0 | Turn_L45 | (-67.5°, 0°) |
| 1 | Turn_L90 | (-112.5°, -67.5°) |
| 2 | Turn_L135 | (-157.5°, -112.5°) |
| 3 | TurnAround | \|角\| ≥ 157.5° |
| 4 | Turn_R45 | (0°, 67.5°) |
| 5 | Turn_R90 | (67.5°, 112.5°) |
| 6 | Turn_R135 | (112.5°, 157.5°) |

实际旋转由代码动画期间平滑插值 Actor Yaw；`AnimNotify_TurnComplete` 挂动画末尾通知结束。

### UHumanoidEnemyAnimInstance 变量

继承 `UBaseLocomotionAnimInstance`（Speed/Direction/bIsFalling/AimPitch），追加：

| 变量 | 类型 | 说明 |
|---|---|---|
| `AIState` | `EHumanoidEnemyAIState` | Patrol / Aim / SearchRush / SearchScan / Dead |
| `bIsTurning` / `TurnAngle` / `TurnAnimIndex` | bool/float/int32 | 转身触发 / 有符号角度 / 0-6 索引 |
| `bIsDead` | bool | Any State → Dead |
| `bIsPatrolScanning` / `PatrolScanAnimIndex` / `PatrolScanAnimCount` | bool/int32/int32 | 扫视开关 / 随机索引(从1) / 总数(EditDefaultsOnly) |
| `StopDecelerationRate` | float | 虚拟减速率（cm/s²，默认 300） |
| 左手 IK：`LeftHandIKTarget` / `bHasWeapon` / `WeaponGripLeftSocket` | FVector/bool/FName | Two-Bone IK 目标 / Alpha / 握把 socket(grip_l) |
| AimIK：`AimTargetComponentSpace` / `AimSourceLocalTransform` / `AimAxisSocketName` / `AimAxis` / `bHasValidAimTarget` / `bIsAiming` / `AimAlpha` / `AimAlphaInterpSpeed` | — | BBBAimIK 脊柱瞄准（详见 11-enemy-ai.md / FEAT-031） |

**私有内部变量（不暴露 ABP）：** `LastWalkSpeed` / `VirtualSpeed` / `bIsVirtualDecelerating`（虚拟减速）；`bAimSourceInitialized`（AimSource 首帧初始化标志）等。

### State Alias 约定
| Alias | 代表状态 | 用途 |
|---|---|---|
| `SA_CanTurn` | Idle / Patrol_Walk_Run | 这些状态下可发起转身 |
| `SA_Turning` | 全部 Turn 状态 | 转身结束统一回 Idle |

### 扩展指南（人形怪）
- **新增具体人形怪**：继承 `ABP_HumanoidEnemy`，Asset Override 覆盖动画，Class Defaults 设扫视数量，无需重搭状态机。
- **Search 状态（FEAT-026）**：状态机加新大状态，过渡基于 `AIState`，新变量在 AnimInstance 追加。
- **射击蒙太奇插槽（FEAT-027）**：AnimGraph 输出链末尾插 Slot 节点（`EnemyUpperBody` / `EnemyFullBody`），骨骼 Slot Manager 注册同名。

---

## 旧系统（双骨骼，C++ 已删除，仅历史参考）

> `ABP_MainCharacter` 曾对应 `ATheManCharacterBase`（双骨骼：GetMesh 全身 + FirstPersonMesh 手臂 + HeadCamera 挂 head 插槽）。
> **C++（`ATheManCharacterBase` / `UTheManAnimInstanceBase`）已于 FEAT-041 删除**（备份 scratchpad/deprecated-char-backup-session43）；`ABP_MainCharacter` / `ABP_FirstPerson_MainCharacter` 蓝图待编辑器删除。下文仅作历史参考。

旧 AnimGraph 流程（已不使用）：
```
Cache_Locomotion → WeaponUpperBody → Slot"UpperBody" → WeaponAimOffset → Slot"FullBody" → Root
```

旧动画实例 `UTheManAnimInstanceBase`（已删除）变量（仅 Locomotion）：

| 变量 | 类型 | 说明 |
|---|---|---|
| `Speed` | `float` | 地面移动速度（Size2D） |
| `Velocity_Z` | `float` | 垂直速度 |
| `bIsFalling` | `bool` | 是否在空中 |
| `AimPitch` | `float` | 瞄准俯仰角，归一化到 [-1, 1] |
| `Direction` | `float` | 移动方向角 [-180, 180] |
