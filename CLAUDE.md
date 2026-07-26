# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 启动必读（每次会话开始时强制执行）

**在做任何其他事情之前，必须按顺序读取并记住以下文件：**

1. **`.agents/harness/AGENTS.md`** — 工作规则、UE5 代码规范、完成标准、验证命令。
2. **`.agents/harness/feature_list.json`** — 当前所有功能及其状态，找到 `active_feature` 字段。
3. **`.agents/harness/progress.md`** — 当前进度与上一次会话的交接内容（文件底部）。
4. **`active_feature` 对应的 archive 文件** — 路径在 `feature_list.json` 每个功能条目的 `archive_file` 字段中。若当前有活跃功能，必须读取其 archive 文件，了解实现历史、未解决 Bug 和遗留决策。

若 `active_feature` 为 `null`（无活跃功能），则跳过第 4 步，但仍需完成前 3 步。

> 以上文件是本项目 Agent 工作状态的唯一真相来源，优先级高于任何聊天记忆。

## Project Overview

**TheManTest** is an Unreal Engine 5.7 single-player game project. It uses the Gameplay Ability System (GAS) and Enhanced Input plugins.

## Building & Compiling

- Open `TheManTest.uproject` with Unreal Engine 5.7 to launch the editor.
- Open `TheManTest.sln` in Visual Studio to edit and compile C++ code.
- Compile from inside UE Editor: click the **Compile** button (or `Ctrl+Alt+F7`), or use **Live Coding** (`Ctrl+Alt+F11`).
- Compile from Visual Studio: Build the **Development Editor** configuration targeting **Win64**.
- Hot-reloading C++ changes (adding new UPROPERTY/UFUNCTION) generally requires a full editor restart; Live Coding works for function body changes.

## Architecture

### Core Framework (`Source/TheManTest/Public/Core/`)

| Class | Role |
|---|---|
| `ATheManGameModeBase` | Minimal game mode shell |
| `ATheManPlayerController` | Owns Enhanced Input setup; routes WASD/look/jump/weapon-switch to the possessed character; drives character switching via `SwitchCharacter(FName)` |
| `ATheManPlayerState` | Owns the `UAbilitySystemComponent` and `UTheManAttributeSetBase` — GAS state persists here across character death/respawn |

**Character switching flow**: `ATheManPlayerController` reads rows from a DataTable (`DT_CharacterRoster`) typed as `FCharacterType` (defined in `TheManCharacterTypes.h`). Each row maps a `FName` ID to a `TSubclassOf<AFPSCharacterBase>` and display metadata. Pressing `TestSwitchCharacterAction` cycles the roster index and calls `SwitchCharacter`.

### Character System (`Source/TheManTest/Public/Characters/`)

> **⚠️ 旧系统（`ATheManCharacterBase` 及其子类、`UTheManAnimInstanceBase`）已于 FEAT-041 删除（备份在 scratchpad/deprecated-char-backup-session43）。唯一玩家基类为 `AFPSCharacterBase`。**

**唯一玩家基类：`AFPSCharacterBase`（第一人称 + 第三人称身体，FEAT-038）**
- 组件层级（FEAT-038 三件套）：
  - `CapsuleComponent`(root) → `ArmsMesh`（Leader+动画宿主，只渲染手臂材质段，跟 capsule 俯仰）→ `HeadCamera`（挂 ArmsMesh 的 head 骨骼，`bUsePawnControlRotation`）
  - `CapsuleComponent` → `BodyRoot`（SceneComponent，绝对旋转，Tick 每帧只取 Yaw → 直立）→ `ShadowBodyMesh`（Follower，全身，OwnerNoSee+bCastHiddenShadow，只投影）+ `LegsMesh`（Follower，只渲染腿材质段，OnlyOwnerSee，无影）
  - 三 mesh 同一 Skeleton，Shadow/Legs `SetLeaderPoseComponent(ArmsMesh)` 共享姿势；几何分离用材质段（`ShowMaterialSection`），禁用 HideBoneByName
  - 注：FEAT-038 C++ 已完成（session43），新骨架/身体 mesh 导入 + 角色 BP 配置在蓝图侧进行
- `GetMesh()` 继承自 ACharacter，隐藏不用
- 实现 `IAbilitySystemInterface`，ASC 从 `ATheManPlayerState` 获取
- 含 `UEquipmentManagerComponent`、`InitialEquipmentClasses`、`CharacterData`、`InitGEClass`、`ArmsHiddenSections`/`LegsHiddenSections`
- 含 `Move` / `Look` / `SwitchEquipment` 输入方法；`PossessedBy` 处理俯仰角限制及 GAS 初始化
- 具体子类：`AFPSInfiltrator` / `AFPSMaintenanceWorker` / `AFPSTheExecutive`（均为空壳，差异化在蓝图）

### GAS Integration

- ASC lives on `ATheManPlayerState`; character gets it via `PossessedBy` → `PlayerState->GetAbilitySystemComponent()`.
- `UTheManAttributeSetBase` exposes `Health` and `MaxHealth` using the `ATTRIBUTE_ACCESSORS` macro (generates Getter/Setter/Initter for each attribute).
- Initial attribute values come from `UTheManCharacterDataAssetBase` (a `UPrimaryDataAsset`) applied through a `GameplayEffect` class set on the character Blueprint.

### Equipment System (`Source/TheManTest/Public/Equipment/`)

Inheritance chain: `AEquipmentBase` → `AWeaponBase` → `AFirearm`

- `AEquipmentBase` provides both `UStaticMeshComponent` and `USkeletalMeshComponent`, socket names (`EquipSocketName`, `HolsterSocketName`), an `EquipMontage`, and an `EquipmentAnimLayerClass`. `Equip(AActor*)` / `Unequip()` are virtual overrideable lifecycle hooks.
- `UEquipmentManagerComponent` (on `AFPSCharacterBase`) manages the `Inventory` array of spawned `AEquipmentBase*` actors, and `CurrentEquipmentIndex`. `SwitchEquipment(int32 Direction)` cycles the active slot.

### Animation

动画实例采用三层继承结构（FEAT-021 重构）：

**基类 `UBaseLocomotionAnimInstance`** (`Source/TheManTest/Public/Characters/Animation/`)
- 骨骼无关，任何角色/敌人均可继承
- `NativeInitializeAnimation` 缓存 `APawn*` 和 `UCharacterMovementComponent*`
- 每帧输出：`Speed` / `Velocity_Z` / `bIsFalling` / `AimPitch` / `Direction`（均为 `BlueprintReadOnly`）

**玩家动画 `UFPSCharacterAnimInstance`** (`Source/.../FPSCharacterBase/Animation/`，FEAT-041 由 `UFPSArmsAnimInstance` 改名)
- 继承基类，当前为空子类（仅基类 Locomotion 变量）
- 挂在 ArmsMesh(Leader)，驱动手臂/影子/腿三件套共享的全身姿势；FEAT-039/040 在此扩展身体瞄准变量
- DefaultEngine.ini 有 CoreRedirect（旧 `FPSArmsAnimInstance`→新名）保旧 ABP 父类链接

**人形怪动画 `UHumanoidEnemyAnimInstance`** (`Source/.../Enemy/Humanoid/`)
- 继承基类，缓存 `AHumanoidEnemy*`，每帧轮询敌人状态
- 关键变量：`AIState` / `bIsTurning` / `TurnAnimIndex`（0-6）/ `bIsPatrolScanning` / `PatrolScanAnimIndex`
- **无 Stopping 状态**：到达路点时启动虚拟减速，将 `VirtualSpeed` 从 `LastWalkSpeed` 以 `StopDecelerationRate`（默认 300 cm/s²）线性降到 0 并覆盖 `Speed`，ABP 靠 Walk→Idle（Speed < 10）自然过渡

**已删除（FEAT-041）：`UTheManAnimInstanceBase`**
- 旧系统动画实例已删；对应旧 ABP（`ABP_MainCharacter` / `ABP_FirstPerson_MainCharacter`）待编辑器删除

## Module Dependencies

Declared in `TheManTest.Build.cs`:
```
Core, CoreUObject, Engine, InputCore, EnhancedInput,
GameplayAbilities, GameplayTags, GameplayTasks
```

## Content Structure

### 地图
- `Content/Maps/TestMap.umap` — 主测试关卡（原 Cyber01/Maps/NewMap，已迁移）
- `Content/NewMap`（NewMap 相关外部 Actor 数据）

### 角色资产
- `Content/Characters/CharacterBase/Blueprint/BP_TheManCharacterBase.uasset` — 角色基类蓝图
- `Content/Characters/CharacterBase/Animations/`
  - `ABP_MainCharacter.uasset` — 主角色动画蓝图（第三人称骨骼驱动）
  - `ABP_FirstPerson_MainCharacter.uasset` — 第一人称手臂动画蓝图
  - `BS_IdleWalkRun.uasset` — 移动混合空间
  - `CR_FPS_Arms.uasset` — 第一人称手臂 Control Rig
  - 子目录：`BlendSpaces/` / `Logic/` / `Montages/` / `Sequences/`
- `Content/Characters/CharacterBase/DataAsset/DA_BaseCharacterAttributes.uasset`
- `Content/Characters/CharacterBase/GAS/GE_CharacterBaseBase_Init.uasset`
- `Content/Characters/MaintenanceWorker/Blueprint/BP_MaintenanceWorker.uasset`
- `Content/Characters/MaintenanceWorker/DataAsset/DA_MaintenanceWorkerAttributes.uasset`
- `Content/Characters/Infiltrator/Blueprint/BP_Infiltrator.uasset`
- `Content/Characters/Infiltrator/DataAsset/DA_InfiltratorAttributes.uasset`
- `Content/Characters/TheExecutive/Blueprint/BP_TheExecutive.uasset`
- `Content/Characters/TheExecutive/DataAsset/DA_TheExecutiveAttributes.uasset`

### Mannequin 骨架与动画（UE5 标准人形）
- `Content/Characters/Mannequins/Meshes/` — SKM_Manny_Simple / SKM_Quinn_Simple / SK_Mannequin / IK_Mannequin
- `Content/Characters/Mannequins/Anims/Rifle/` — 步枪动画（Fire / Reload / Equip / ADS Aim Offset 等）
- `Content/Characters/Mannequins/Anims/Unarmed/` — 徒手动画（Jog / Walk / Jump / Fall/Land）
- `Content/Rifle_01/Character/Mesh/` — UE4 Mannequin（SK_Mannequin / IK_SK_Mannequin）及配套材质纹理

### 武器资产
- `Content/Weapons/RepairGun/Blueprint/BP_RepairGun.uasset` — 修理枪蓝图（当前唯一装备）
- `Content/Weapons/RepairGun/Mesh/` / `Material/` / `Textures/`
- `Content/Weapons/_Shared/Animations/Firearm/ABP_FirearmBase.uasset` — 武器动画层蓝图（WeaponAimOffset 层内含 AimIK）
- `Content/Weapons/_Shared/Animations/Interface/ALI_WeaponAnim.uasset` — 武器动画层接口（Linked Anim Layer Interface）

### 科幻武器资产包（SCI_FI_WEAPON_PACK）
包含 SCFP / SCFSR / SCF_Rifle / SCF_Rifle_02 / SciFiRifle / SCI_FI_Shotgun 六种武器的网格、材质、动画和演示地图，供后续武器扩展参考。

### 核心蓝图
- `Content/Core/` — BP_TheManGamemodeBase / BP_TheManPlayerController / BP_TheManPlayerState / DT_CharacterRoster

### 输入
- `Content/Inputs/Actions/` — IA_Jump / IA_Look / IA_Move / IA_SwitchEquipment / IA_Test
- `Content/Inputs/Contexts/IMC_Default.uasset`
