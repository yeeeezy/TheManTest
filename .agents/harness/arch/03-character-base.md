# 角色基类（Character Base）

**何时读取：** 修改角色通用行为、组件布局、GAS 初始化流程、增加所有角色共享的属性或函数时。

> ⚠️ **唯一玩家基类为 `AFPSCharacterBase`。旧 `ATheManCharacterBase` 双骨骼系统已于 FEAT-041 删除（不再有"保留弃用"代码）。**

**唯一玩家基类（FPS）：**

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h` | HeadCamera / **ViewmodelRoot / ArmsViewMesh**（FEAT-042 独立 FP viewmodel）/ **BodyRoot / ShadowBodyMesh / LegsMesh**（FEAT-038 三件套，含 Getter）/ EquipmentManager；**`GetArmsMesh()` 返回 `ArmsViewMesh`**，武器挂载/开火蒙太奇/装备渲染走相机子级 FP 手臂；`GetMesh()` 是 GASP/MM 宿主，驱动身体/影子/腿；`ArmsHiddenSections` / `LegsHiddenSections`（EditDefaultsOnly 材质段隐藏）；CharacterData / InitGEClass；通用 `DefaultAbilityClasses`；`PrimaryFire()` / `SecondaryFire()` / `IsSprinting()`（冲刺键状态，停步走/跑档用）|
| `Source/TheManTest/Private/Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.cpp` | 组件挂载：**HeadCamera←RootComponent(capsule)**，**ViewmodelRoot←HeadCamera，ArmsViewMesh←ViewmodelRoot**。蓝图序列化的组件 Transform 是运行时唯一值：C++ 不得在 OnConstruction、BeginPlay 或 Tick 写 HeadCamera/BodyRoot/ViewmodelRoot/ArmsViewMesh/CharacterMesh0/LegsMesh 的位置或旋转；Tick 仅更新玩法与动画参数。C++ 同步驱动 ArmsViewMesh 与 GetMesh() 的原版 VFXPack AnimBP。`ShadowBodyMesh` / `ShadowUpperBodyMesh` 为弃用空组件；唯一完整影子来自 `GetMesh()` 的 CastHiddenShadow。|

**扫描组件：** `UScanEffectComponent` 由所有玩家角色基类创建。组件构造函数提供项目默认 `MPC_ScanEffect` 与世界空间适配材质 `M_InfiltratorScanTerrainAdaptive`，角色 BP 仍可覆盖。`TriggerScan` 会创建归属角色的运行时 `UDecalComponent`，先通过 `AddInstanceComponent` 纳入 Actor 生命周期，再注册到当前 World；独立 MID 接收 `ScanOriginWS` / `ScanRadius` / `ScanOpacity`，不复用红色后处理的相机相对参数。红色 MPC 为空时只跳过红色后处理，不再阻断地形扫描。`RetractScan` 隐藏 Decal 并清零透明度。

**FEAT-038 三件套组件布局（详见 archive/FEAT-038）：**

```
Capsule(root) [bUseControllerRotationYaw=true, bUseControllerRotationPitch=FALSE]
├─ HeadCamera = 挂 capsule 固定眼高(相对 Z≈+77)，bUsePawnControlRotation，稳定 gameplay 相机
│   └─ ViewmodelRoot = 原版 BodyRotator 等价节点，位于相机原点，负责整体冲刺旋转
│       └─ ArmsViewMesh = 持有原版 SK_ArmMesh 相对位置/旋转的独立 FP 手臂+武器挂载目标
├─ GetMesh()  = GASP/MM Leader+动画宿主+根运动源，OwnerNoSee，CastShadow=false
└─ BodyRoot (SceneComponent, 绝对旋转, Tick 每帧只取 Yaw → 直立；相对 Location=(0,0,0))
    ├─ ShadowBodyMesh = 已弃用空组件；完整影子直接由 GetMesh() CastHiddenShadow
    └─ LegsMesh       = Follower，只渲染腿材质段，OnlyOwnerSee，无影
```
- session89 起暂停原地转身方案：`bUseControllerRotationYaw=true`，Pawn 直接跟随 Controller yaw；`BodyRoot` 每帧直接使用 Actor yaw 并保持 Pitch/Roll 为 0。旧 `BodyVisualYaw` 滞后、固定 45 度转步和动画曲线驱动代码均已删除，后续转体作为独立方案重新设计。
- 三 mesh 须引用**同一 Skeleton**；几何分离只用材质段（`ShowMaterialSection`）/OpacityMask，**禁用 HideBoneByName**（会改共享姿势）。本项目实际用「物理拆 mesh」（Blender 拆 Arms/Legs，原整块当 Shadow），`ArmsHiddenSections/LegsHiddenSections` 留空。
- 蓝图侧：Legs 组件相对变换需与 GetMesh() 完全相同（当前 Location=(0,0,-90)、Yaw=-90），保证下半身根点与脚底影子起点重合。ShadowBody/ShadowUpperBody 不再持有 Mesh，避免编辑器显示旧副本而与 PIE 的权威 CharacterMesh0 不一致。
- 蓝图侧 **Mesh（GetMesh()）组件**：指定全身骨架 mesh + locomotion AnimClass + 相对 Transform；**Cast Shadow 取消勾选**（BP 勾选会覆盖 C++ 的 false，造成手臂形状影子与 ShadowBodyMesh 穿帮）。`OnlyOwnerSee` 已由 C++ 设——故编辑器视口（无 owner）看不见手臂，PIE 里可见，属正常。

**根运动历史决策（FEAT-039，已被 FEAT-042/session62 的独立 Viewmodel 方案取代）：**
- FEAT-039 曾短暂把手臂宿主合并进 `GetMesh()`，以便由 `UCharacterMovementComponent` 提取根运动；这不是当前组件结构。当前 `GetArmsMesh()` 返回独立 `ArmsViewMesh`，玩家普通移动交给 CharacterMovement，武器层按 session70 同时链接 `ArmsViewMesh` 与 `GetMesh()`。
- 前提条件成立才安全：session47 相机已从 head 骨骼挪到 capsule → 手臂无人依赖，可合并。
- **根运动方案（详见 arch/12 + archive/FEAT-039）**：ABP 根运动模式与「输入驱动 locomotion」的取舍是本项目踩过的大坑，务必看 12。一句话：**`Root Motion from Everything` 会让动画接管移动、压制 WASD 输入**——铁律「原地 clip（idle/走跑循环/跳跃）Enable Root Motion 必须关，只有位移 clip（停步等）才开」；漏关一个原地 clip（尤其 idle）→ 移动卡死+抖动。
- **session62 FP viewmodel 方案修正（重要）**：相机仍挂 capsule 固定眼高、只用控制器旋转转向；FP 手臂不再绕肩部 `ArmsPivot` 手动俯仰，而是 `HeadCamera -> ViewmodelRoot -> ArmsViewMesh`。这样旋转支点就是相机原点，手臂屏幕位置不随 pitch 漂移。不要把相机挂到动画 head 骨骼下，也不要旋转 Character capsule pitch。后续相机/移动惯性优先叠到 `ViewmodelRoot` 或 `ArmsViewMesh`，不用 SpringArm。
- 第一人称视点横向/纵向微调不需要 SpringArm，继续保持 `Capsule -> HeadCamera -> ViewmodelRoot -> ArmsViewMesh`。`HeadCamera.RelativeLocation` 只定义真实观察点；`ViewmodelRoot` 必须留在相机原点，以保持原版 BodyRotator 的旋转半径；最终手臂/武器构图偏移由 `ArmsViewMesh.RelativeTransform` 持有。
- **FEAT-065 最终构图（session126）：** `ViewmodelRoot.RelativeLocation=(0,0,-7)`、`RelativeRotation=(0,0,0)`，HeadCamera 保持 110° FOV 与真实观察点不变。构造、OnConstruction、BeginPlay 均通过 `ApplyViewmodelFraming()` 应用，避免 BP 旧默认值覆盖；装备继续挂 `ArmsViewMesh` Socket。`TheManTest.Player.Viewmodel.FramingCapture` 会以稳定 Idle 帧执行 1920×1080 截图及层级/FOV/Transform/Socket 断言。
- C++ 基类速度默认（session47 调）：`WalkSpeed`=250 / `SprintSpeed`=550；`PitchMin`=-75 / `PitchMax`=40。FEAT-070 对 MaintenanceWorker 的实际 Walk/Jog 脚速逐帧标定后，`BP_MaintenanceWorker` 专属覆盖为 Walk=100 / Sprint=300 cm/s，与其 BlendSpace 的原速样本网格点精确一致；其他角色不得无依据照搬该动画专属值。
- FEAT-038 C++ 已完成（session43 编译通过，session47 改相机/俯仰/BodyRoot/速度）；身体 mesh 已物理拆好导入，角色 BP 三件套装配在蓝图侧进行。

> 旧 `ATheManCharacterBase` / `Infiltrator` / `MaintenanceWorker` / `TheExecutive` 及 `UTheManAnimInstanceBase` 已于 FEAT-041 删除，备份在 scratchpad/deprecated-char-backup-session43。

> **MCP 审计遗留项（2026-07-26）：** 活动资产 `BP_Infiltrator` 仍硬引用 `/Game/Characters/Infiltrator/Blueprint/BP_Infiltrator_Old`。当前仅确认依赖存在，尚未定位具体属性或图节点；在用户手动确认前不得自动删除或重写该引用。
