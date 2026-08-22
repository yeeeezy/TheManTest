# 动画蓝图架构（ABP 层 / Slot / 节点图）

**何时读取：** 搭建或修改 ABP 层结构、Linked Anim Layer、Slot 蒙太奇插槽、AimIK 节点链、武器动画扩展时。

> 本文是 `06-animation.md` 的详细版：06 速查 C++ AnimInstance 类与变量，本文讲 ABP 资产的层/Slot/节点图与扩展策略。
> **当前玩家 ABP = 玩家统一 Skeleton。** `GetMesh()`、`ArmsViewMesh` 与武器 Linked Anim Layer 共用玩家 Skeleton；Enemy 可使用各自动画原始 Skeleton，通过无骨架 Template AnimBP 派生对应子 AnimBP。旧双骨骼系统的 C++ 已于 FEAT-041 删除（文末旧系统节仅作历史参考）。

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
| `TABP_CharacterBase_BodyLocomotion` | `Content/Characters/CharacterBase/Animations/Body/Logic/` | 无骨架玩家身体 locomotion 模板；MaintenanceWorker 最终身体子类为其角色目录下的 `ABP_MaintenanceWorker_Body` |
| `TABP_CharacterBase_FirstPerson` | `Content/Characters/CharacterBase/Animations/FirstPerson/Logic/` | 无骨架第一人称宿主模板；拥有基础 locomotion/Jump、武器层路由及最终 Lean/Look 图 |

> CharacterBase 的 Body/FirstPerson Template AnimBP 不得直接绑定具体角色动画资产。模板保留 `Sequence Player` 与专用 `Blend Space Player` 占位节点，具体 Sequence/BlendSpace 由角色子 AnimBP 的 Parent Asset Overrides 提供。第一人称模板状态机名为 `FirstPersonLocomotionSM`；资产为空时，跳跃过渡使用状态机原生自动剩余时间规则，避免在模板中保留指向具体 Sequence Player 的 Getter。
| `ABP_MaintenanceWorker_FirstPerson` | `Content/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/` | 绑定 MaintenanceWorker Skeleton 的模板子类；无本地 AnimGraph，通过继承模板作为 ArmsViewMesh 常驻宿主 |
| `ALI_WeaponAnim` | `Content/Weapons/_Shared/Animations/Interfaces/` | 武器动画层接口，定义两个层：`WeaponAimOffset`、`WeaponUpperBody` |
| `TABP_FirstPersonFirearmBase` | `Content/Weapons/_Shared/Animations/Templates/` | 第一人称枪械 Template AnimBP，实现 `ALI_WeaponAnim`；统一持枪 Idle/WalkRun 状态逻辑，具体武器子类替换动画资产 |

> `TABP_FirstPersonFirearmBase.WeaponUpperBody` 的 Idle 使用空 `Sequence Player`，WalkRun 使用空 `Blend Space Player` 并由 `Speed` 驱动 X；具体枪械子类通过 Parent Asset Overrides 分别提供静止 Sequence 和移动 BlendSpace。不要用 Run Sequence 代替 WalkRun BlendSpace，也不要保留断线的旧播放器或输入节点。
| `UCharacterBaseAnimInstance` | `Source/.../Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h` | 第一人称宿主/枪械模板共享的 C++ 数据基类；统一映射移动、腾空与通用 Lean/Look 变量 |
| `UFPSCharacterAnimInstance` | `Source/.../Characters/CharacterBase/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h` | 身体 locomotion 模板仍在使用的 C++ 父类；旧 `UFPSArmsAnimInstance` 已无源码和 CoreRedirect |
| `UFirearmAnimInstance` | `Source/.../Equipment/Firearms/FirearmAnimInstance.h` | 当前枪械动画层模板 `TABP_Firearm_UpperBodyBase` 的 C++ 父类，持有四个 AimIK 变量 |

> 敌人 ABP（`ABP_HumanoidEnemy`）见下方“敌人动画架构”节及 `11-enemy-ai.md`。

---

## ABP_BodyLocomotion（玩家全身主 ABP，普通 locomotion）

> `ABP_BodyLocomotion` 是项目自己的玩家全身主 ABP，不应因为放弃 Motion Matching 而直接删除。若它曾在实验中加入 `PoseSearchHistoryCollector` / Motion Matching / Stop 状态，需要在编辑器里清掉这些节点，保留为普通 locomotion 模板。父类推荐继续用 `UFPSCharacterAnimInstance`，该类现在是很薄的玩家专属子类，只比 `UBaseLocomotionAnimInstance` 多 `AccelDirection` / `bHasAcceleration`。

**当前身体路径：** `Content/Characters/CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion`

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
- AnimGraph 输出链：LocomotionSM → DefaultSlot → 主 ABP 唯一的 Layered Blend per Bone.BasePose；`WeaponUpperBody` Linked Layer 只产出纯武器 Pose并接 BlendPose（当前纯武器实现不读取 `UpperBodyInPose`）→ WeaponAimOffset → UpperBodySlot → FullBodySlot → `Two Way Blend(A=Pose Snapshot "WeaponTransitionPose", B=FullBodySlot, Alpha=WeaponTransitionAlpha)` → Output。武器层图 `WeaponAimOffset` / `WeaponUpperBody` 的 Graph Blend 保持默认关闭；末端不使用 Inertialization。分层与切枪 Pose 事务由主 ABP 统一拥有，武器层不得重复混合全身 locomotion。注意 AnimGraph 的 Pose 输出不能一对多，不能把 `DefaultSlot.Pose` 同时直接接给 BasePose 与 Linked Layer 输入，否则后一次连接会顶掉前一条。

---

## 旧 ABP_FPS_Arm_MainCharacter AnimGraph 流程（仅迁移历史）

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

当前主 ABP 不含武器专属 IK 节点；AimIK 应封装在当前武器层模板 `TABP_Firearm_UpperBodyBase` 或其骨架兼容子层内。

---

## 武器动画扩展策略

### 装备动画父类选择

```text
UEquipmentAnimInstance
  └─ UFirearmAnimInstance
```

- 所有装备通用上半身动画层优先继承 `UEquipmentAnimInstance`，可直接读取 `Speed` / `Direction` / `Velocity_Z` / `bIsFalling`。
- FEAT-046 原计划把步枪上半身改为普通 1D BlendSpace，但 session80 MCP 复核发现实际仍为 `Idle <-> WalkRun`，且现有 `BS_Rifle_UpperBody_IdleWalkRun` 是 2D、0 samples；该切片已转为 `needs_improvement`，后续由 FEAT-051 按用户保留的动画资产重新落地。不要依赖已删除的 Start/End 状态机专用变量。
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

> **FEAT-077 已替代本节旧的“共用主 ABP”运行方案。** 当前仅保留两个 AnimBP 类：Arms 独立 FP AnimBP；Body 完整 locomotion AnimBP 在最终输出前执行 `Copy Pose From Mesh(FirstPersonPoseSource)`，并以 `Layered Blend Per Bone(spine_01)` 合成上半身。Copy Pose 使用局部骨骼变换，不复制 root/pelvis，也不复制 ArmsViewMesh 的相机空间组件 Transform。Arms 必须先于 Body Tick，装备层只链接 Arms。

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
- 主 `TABP_BodyLocomotion` 是武器上半身分层的唯一所有者：它把基础 locomotion 与 `WeaponUpperBody` 产出的纯武器 Pose 从 `spine_01` 以 Depth 4 渐进混合，并开启 `Mesh Space Rotation Blend`。武器模板不再放置第二个 `Layered Blend per Bone`；这样所有武器统一避免 pelvis/root 横移旋转带偏枪口，并保持身体、影子和 FP 手臂共用同一 locomotion 时序。影子腰部缺口若来自身体模型的几何分段，应修改模型，不能用动画混合作为修复手段。

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

> 走跑档由 `AFPSCharacterBase::StartSprint/StopSprint` 改 `CharacterMovement->MaxWalkSpeed`，ABP 通常只读 `Speed` 进入同一个 Walk/Run BlendSpace。

**MaintenanceWorker 步速标定（FEAT-070）：** `BS_RunWalk_MaintenanceWorker` 的 Speed 轴为 0~300，Idle=0、Walk=100、Jog=300；`BP_MaintenanceWorker` 同步覆盖 WalkSpeed=100 / SprintSpeed=300。全部方向样本保持 RateScale=1.0，使 Walk/Jog 在对应速度点完整播放原动画，不被 Idle/Jog 权重稀释；不能把这组值直接用于其他动画集。

**原地转身：** session89 起暂停。玩家 ABP 不应包含 Turn 状态、`TurnRootYaw` 曲线读取或 root-bone 转向抵消节点；Pawn 与 `BodyRoot` 直接跟随 Controller/Actor yaw。后续转体方案另立功能重新设计。

未来重新设计时可参考本机 `D:\Unreal Projects\GameAnimationSample`，但不要直接移植整套 Motion Matching。该样例通过 TurnInPlace/Pivot/Spin Pose Search Database、未来轨迹、左右支撑脚动画版本、`Offset Root Bone`、`Orientation Warping` 和 `Foot Placement` 共同减少滑脚；适合提炼为“脚相选择 + 左右脚转身动画 + 明确的 Actor/Root 同步 + Foot IK”方案。

---

## TABP_Firearm_UpperBodyBase（武器层模板）AimIK 目标流程

> **FEAT-074 session148 临时边界：** `TABP_BodyLocomotion` 的第三人称身体输出已旁路 `WeaponUpperBody/AimOffset` 支路，改由 `DefaultSlot.Pose` 直接进入 `UpperBodySlot.Source`。原因是现有 RepairGun Linked Layer 在稳定状态返回参考姿势并覆盖 spine_01 以上，造成 CharacterMesh0/影子 T-Pose。第一人称 ArmsViewMesh 已独立使用 `ABP_VFXPack_FirstPerson`。在武器层能够保证有效 Pose 输出前，不得重新把该支路接回身体最终输出。

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
- **新增需要 AimIK 的武器**：让该武器动画层 ABP 继承或复用与玩家 Skeleton 兼容的 `TABP_Firearm_UpperBodyBase` 路线。
- **新增需要专属跑跳的武器**：在 `WeaponAimOffset` 层内自建 StateMachine，忽略 `WeaponAimInPose` 输入。
- **修改骨骼链权重**：在实际使用的武器层模板/子 AnimBP 的 BBBAimIK 节点 BoneChain 属性里调各骨骼 Weight。

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
| `TABP_CharacterBase_BodyLocomotion` | `Content/Characters/CharacterBase/Animations/Body/Logic/` | 无骨架玩家身体 Template ABP，提供完整 Locomotion 状态机与上半身 Pose 合成框架 |
| `ABP_HumanoidEnemy` | `Content/Enemy/Phantom/Animations/Logic/`（ABP_Phantom 等） | 人形怪 ABP，Parent Class = `UHumanoidEnemyAnimInstance` |
| `UBaseLocomotionAnimInstance` | `Source/.../Characters/_Shared/Animation/BaseLocomotionAnimInstance.h` | 所有 AnimInstance 的 C++ 基类 |
| `UHumanoidEnemyAnimInstance` | `Source/.../Enemy/Humanoid/HumanoidEnemyAnimInstance.h` | 人形怪专属 AnimInstance |
| `UAnimNotify_TurnComplete` | `Source/.../Enemy/Humanoid/AnimNotify_TurnComplete.h` | Turn 动画末尾 → `AHumanoidEnemy::OnTurnComplete()` |

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

## 动画重定向项目边界

- 主项目 `TheManTest` 只保存已经完成重定向并验收的最终动画资源。
- 禁止在主项目中创建或执行 IK Retargeter、批量重定向、源骨架/源 Mesh 导入等操作，也不得保留重定向中间目录。
- 重定向必须在资源来源项目中完成，验证后仅将最终动画迁移至主项目。
- 外部资源项目：`D:\Unreal Projects\TMIIR`、`D:\Unreal Projects\FPSShooter1`。
- Shooter 动画必须在 `FPSShooter1` 内完成所需重定向，再迁移最终产物。

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
## 2026-08-01 session149 — 玩家统一 AnimBP 修正

- MaintenanceWorker 的 `CharacterMesh0` 与 `ArmsViewMesh` 必须统一使用 `ABP_MaintenanceWorker`；不得再为第一人称直接挂独立 VFX AnimBP。
- `ShadowBodyMesh` / `LegsMesh` 是 `CharacterMesh0` 的 Leader Pose 跟随者；影子姿势以主身体最终 Pose 为准。
- RepairGun 的 `ABP_RepairGun_AnimLayer` 同时链接到 `CharacterMesh0` 和 `ArmsViewMesh`。基础链为 `DefaultSlot -> Layered Blend Per Bone.BasePose`，武器层只覆盖配置的上半身分支，再进入 WeaponAimOffset/Slots。
- 当前 WalkRun 状态直接播放 `AS_Rifle_A_Run`（0.5×）；这是因为已验证 BlendSpace Player 在该模板继承链运行时返回参考姿势。不要重新接回旧 BlendSpace，除非先在 PIE 同时证明身体、第一人称和影子均输出非参考 Pose。

## 2026-08-02 session152 — VFXPack 第一人称上半身例外

- 用户明确要求先忽略下半身速度、让玩家视角上半身与 VFXPack 一致；MaintenanceWorker 的 `ArmsViewMesh` 因此直接使用已整理的 `ABP_VFXPack_FirstPerson`，复用原版 Idle/Run 状态机和播放速度。
- `CharacterMesh0`、`ShadowBodyMesh` 与 `LegsMesh` 继续保留项目主 ABP/Leader Pose 链，下半身 locomotion 不在本次调整范围内。
- 这是 RepairGun/VFXPack 第一人称视图的明确特例；不要据此把其他角色或武器改成独立第一人称 AnimBP。

## 2026-08-02 session153 — 移动反馈与投影 Pose 同源

- `ABP_VFXPack_FirstPerson` 的 Idle/Run 继续负责主要武器姿态偏移；C++ 只复刻原 Walking/Running CameraShake 的频率、振幅和淡入状态，输出改写到 `ViewmodelRoot`，不启动 gameplay CameraShake。
- 已删除此前自创的移动方向位移/旋转和鼠标旋转滞后，避免与原动画重复叠加。
- 运行时逐骨审计确认旧 `ShadowBodyMesh` 虽正确 Leader=`CharacterMesh0`，但因此复制了与第一人称不同的身体 locomotion 上半身。现改为 Leader=`ArmsViewMesh`；`spine_03/hand_r/hand_l` 组件空间 Pose 与 VFXPack 第一人称逐项相同。`LegsMesh` 仍 Leader=`CharacterMesh0`，下半身速度体系本轮不改。

## 2026-08-02 session156 — VFXPack 同源 AnimBP 最终修正

- session153 的 `ShadowBodyMesh Leader=ArmsViewMesh` 已撤销。MaintenanceWorker 恢复本项目既有的同源方式：`CharacterMesh0` 与 `ArmsViewMesh` 使用同一原版 VFXPack AnimBP 类，各自拥有 AnimInstance；Shadow/Legs 均 Leader=`CharacterMesh0`。
- VFXPack `Walk_Run_1D` 只有 Speed 轴。方向姿态由原角色 Body_Sway 写入 AnimBP，不是 2D BlendSpace：原始目标为 `Clamp(MoveRight+MouseX,-1,1)` 和 `Clamp(-MoveForward-10×LookUp,-1,1)`，Walk 插值速度 2、Sprint 8；写入 Modify Bone 前原 EventGraph 还精确乘以 `Lean_Sides_Offset=8.0` / `Look_Up_Offset=2.0`。
- session224 当前边界：按用户决定恢复 `e1c24eb` / session220 的完整75° `RemappedLeanRoll/RemappedLookPitch` 骨骼映射。A/D 继续通过 `spine_03/hand_l` Modify Bone 链带动手臂与枪；不旋转装备 Actor，也不移动 `ArmsViewMesh` 做圆弧补偿。该版本倾斜观感获用户认可，但保留枪口随骨骼枢轴上下平移的已知问题。
- 普通移动 Body Sway 的进入/回弹速度由 `AFPSCharacterBase.ViewmodelBodySwayInterpSpeed` 控制，Class Defaults 分类为 `Viewmodel|Movement`，默认 `6.0`；冲刺保持 `8.0`。
- 静态第一人称构图仅在 `BeginPlay` 应用：`ViewmodelRoot.Location=0`，Arms 使用 `ViewmodelOffsetLocation` 与 `BaseArmsRotation`。Tick 不再重写 Arms 静态 Transform，只更新 `ViewmodelRoot` 的冲刺 Pitch 和 AnimBP Lean/Look。
- C++ 必须把 `Is_Moving`、`Is_InAir`、`Character_Speed`、`Lean_Sides_Amount`、`Look_Up_Amount` 同帧写给两个 AnimInstance，避免第一/第三人称及影子上半身再次分叉。
- session157 修正：前后倾斜的原版 AnimBP 变量实际名为 `Look_Up_Amount`，不是 `Look_Up_Down_Amount`。方向倾斜必须缓存 Enhanced Input 原始移动轴，A/D → `Lean_Sides_Amount`、W/S → `Look_Up_Amount`；不得只用 CharacterMovement 速度反推后就以变量数值代替最终姿势验收。正式 AnimGraph 为 `spine_03` Additive Roll/Pitch，加上 `hand_l` 的 0.5× Additive Roll。
- session165：取消项目补充的 5cm `ViewmodelRoot` Y 向横移；A/D 最终视觉只来自上述 AnimBP 骨骼链，构图根节点不再随侧移输入改变位置或旋转。
- session166：冲刺恢复原蓝图职责：按键意图驱动 0.2s 可逆时间线，同时插值 MaxWalkSpeed 550→750 并旋转 `BodyRotator` 等价节点 `ViewmodelRoot` Pitch 0→-12.5°；不再把压枪角写入 `spine_03` 的 `Look_Up_Amount`。

## 2026-08-04 玩家最终统一链

- MaintenanceWorker 四个 SkeletalMesh 均指定 `ABP_CharacterBase_Body` 最终骨架子类；Shadow/Legs Leader=`CharacterMesh0`。
- RepairGun 遵循统一动画架构：`ArmsViewMesh` 与 `CharacterMesh0` 使用同一 `ABP_CharacterBase_Body` 主 AnimBP，装备时同时链接同一个 `ABP_RepairGun_AnimLayer`；不再存在 `BodyEquipmentAnimLayerClass` 或 `ABP_RepairGun_BodyAnimLayer` 分流。维修枪 Idle/Run 使用外部批准资源项目 `FPSShooter1` 生成并迁入的方向修正最终资产 `AS_VFXPack_FP_Idle/Run`。Shadow/Legs 继续跟随 `CharacterMesh0`。
- MaintenanceWorker 身体资产的视觉正前方为 Mesh 局部 `+Y`；`CharacterMesh0`、`ShadowBodyMesh`、`LegsMesh` 必须使用蓝色 Z/Yaw `-90°`，把视觉 `+Y` 对齐 Character/Actor 的 `+X` 前向箭头。第一人称 `ArmsViewMesh` 的相机空间构图旋转保持独立，不得用它反推或覆盖身体朝向。Unreal Python 的 `Rotator` 位置参数为 roll/pitch/yaw，脚本写 Yaw 时必须使用第三个参数。
- 初始装备完成链接后，在首个渲染帧前对 CharacterMesh0 与 ArmsViewMesh 执行零时长动画评估，避免冷启动入口 Pose 闪帧。
- 无引用旧 `ABP_CharacterBase` 已删除；正式运行类仅保留模板 `TABP_BodyLocomotion` → 最终 `ABP_CharacterBase_Body` → RepairGun Linked Layer。
