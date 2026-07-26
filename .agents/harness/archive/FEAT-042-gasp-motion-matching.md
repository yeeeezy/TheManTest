# [FEAT-042] GASP Motion Matching 集成（CMC 版，UEFN_Mannequin 全身骨架）

**创建日期：** 2026-06-27
**状态：** in_progress（session51 开工 / 阶段1 资产迁移）
**Archive 文件：** `archive/FEAT-042-gasp-motion-matching.md`

---

## 功能概述

把 Epic **Game Animation Sample（GASP）** 的 **Motion Matching 全套下半身 locomotion** 集成进 TheManTest，**取代 FEAT-039 手搭的下半身状态机 + 全部停步 C++**。用户拍板"真上 GASP 全套 MM"（不是只搬几个 clip），理由"正好学新技术"。

上半身**保留本项目自己的射击层**（武器 Linked Anim Layer 盖 spine+，BBBAimIK 改 Manny spine 骨骼名）——MM 只接管下半身移动。

---

## 设计决策 / 选型定案（来自 session50 对 GameAnimationSample 的勘查）

本地样例工程：`D:\Unreal Projects\GameAnimationSample`（UE5.7）。

1. **用 CMC 版，不用 Mover。**
   - GASP 并排提供 CMC 版和 Mover 版角色。用 **`SandboxCharacter_CMC` + `SandboxCharacter_CMC_ABP`**——建在标准 `ACharacter` + `UCharacterMovementComponent` 上，与本项目 GAS / 装备 / 相机 **零冲突**。
   - Mover / NetworkPrediction / Locomotor 插件**装着不用即可**（机甲 FEAT-043 才需要 Mover）。
2. **骨架：全身上 `SK_UEFN_Mannequin`（+ `IK_UEFN_Mannequin`）。**
   - MM 全库在 `Characters/UEFN_Mannequin/Animations/MotionMatchingData/`（Default / Dense / Sparse 密度档，Chooser `CHT_PoseSearchDatabases` 选库），含 走/跑/蹲 × Loops/Starts/Stops/Pivots/TurnInPlace + Jumps/Lands/Traversal。
   - **决策**：角色自己的 mesh 在 **Blender 重绑 Manny 骨架**，比 retarget 几百个动画 + 重建 Pose Search DB **省太多**，质量也最高。
   - `RetargetedCharacters/`（`BP_Echo` / `Twinblast` + `ABP_GenericRetarget`）= Epic 官方"换皮"范式，留作参考。
3. **上半身保留自己的射击层**：现有 `ALI_WeaponAnim` 接口 + `ABP_FirearmBase`（含 BBBAimIK）范式继续用，BBBAimIK 脊柱链骨骼名改成 Manny 的 `spine_01→spine_03`。

---

## 取代关系（重要）

- **取代**：FEAT-039 的「下半身 locomotion 状态机 + 2D blendspace + 全部停步 C++」——包括 session45 的 strafe 8 向、session48~50 反复打磨的 StopAnimIndex / 延迟停 / 保持原速滑行等所有停步逻辑。MM 自带 Stops/Pivots/Starts，不再需要手搭。
- **保留 / 沿用**：
  - FEAT-038 三件套 mesh（ArmsMesh Leader / ShadowBodyMesh / LegsMesh Follower）+ Leader/Follower 架构 —— GASP ABP 挂 Leader（ArmsMesh 或合并后的 GetMesh），Shadow/Legs 跟随。
  - 上半身武器 Linked Layer + BBBAimIK（原 FEAT-039 后半段 + FEAT-040）。
  - 装备 / 武器层链接 C++（`Equip()` → `LinkAnimClassLayers`）不动。
- **FEAT-039 状态调整**：下半身部分作废 → FEAT-039 retitle 为只剩上半身武器层（叠在 MM 之上），待 FEAT-042 阶段3 一并落地。

> ⚠️ session50 写的"保持原速滑行"停步 C++（`FPSCharacterBase` 的 `bHasMoveInput`/`bCoasting`、`FPSCharacterAnimInstance::IsStopCommitted()`）**已决定不验证**，将随 MM 集成清理。是否立即删见实现日志。

---

## 分阶段路线

- **阶段0**（用户）：先玩透 GASP 本体，理解 MM / Chooser / Pose Search Database / Trajectory 怎么运转。
- **阶段1**：把 CMC 版 GASP **Migrate 进 TheManTest**，用 Manny mesh 在独立测试关卡跑通 MM（先不接本项目角色）。
- **阶段2**：接进 `AFPSCharacterBase`——Leader mesh 跑 MM ABP，Shadow/Legs 跟随；相机 / 装备 / GAS 照常。
- **阶段3**：叠上半身武器层（武器 Linked Layer 盖 spine+，BBBAimIK 改 Manny 骨骼名）。
- **阶段4**：把角色自己的 mesh 在 Blender 重绑 Manny 骨架，换上。

---

## 范围

**插件：** 启 **Motion Trajectory + Pose Search + Animation Locomotion Library**（GASP MM 依赖；Chooser 插件）。Mover / NetworkPrediction 不需要（除非 FEAT-043）。

**资产迁移（UE Migrate，用户操作）：** 在 GameAnimationSample 工程里右键 `SandboxCharacter_CMC`（及 `SandboxCharacter_CMC_ABP` / MM 数据库 / Chooser / UEFN_Mannequin 骨架+mesh+IK），Migrate 进 TheManTest `Content/`。

**涉及 C++：**
- 预期需要 Trajectory / MM 驱动相关的少量 C++（GASP 角色类里有 locomotion 状态计算、trajectory 生成）。迁移后评估：是直接用 GASP 的 `SandboxCharacter_CMC` 逻辑搬进 `AFPSCharacterBase`，还是抽出 component。开工阶段2 时定。
- 清理 FEAT-039 停步 C++。

**涉及蓝图 / 编辑器（用户操作）：**
- 跑通 MM ABP；接 Trajectory 节点 + Pose Search DB；上半身 Layered blend per bone 叠武器层。

**依赖：**
- FEAT-038 三件套 mesh 架构（已 C++ 完成）。
- 本地 `D:\Unreal Projects\GameAnimationSample`。

**完成标准：**
- [ ] 启 MM 相关插件（Motion Trajectory / Pose Search / Animation Locomotion Library / Chooser），编辑器无报错
- [ ] Migrate CMC 版 GASP + UEFN_Mannequin 进 TheManTest，独立测试关卡 Manny 跑通 MM（走跑/起步/停步/转身/pivot 自然）
- [ ] 接进 `AFPSCharacterBase`：Leader 跑 MM，Shadow/Legs 跟随姿势一致；相机/装备/切角色正常
- [ ] 上半身武器层叠在 MM 之上（持枪 aim pose + BBBAimIK），切武器切层正常
- [ ] （阶段4）角色自有 mesh 重绑 Manny 换上
- [ ] 清理 FEAT-039 停步 C++ + TEMP 调试

---

## 实现日志

### 2026-06-27-session51 — 功能创建 + 阶段1 起步

- 用户 session50 拍板上 GASP 全套 MM，session51 选 B 路线（停步小改不验证，直接转 GASP 主线），登记本功能。
- 选型 / 取代关系 / 分阶段路线见上，均沿用 session50 勘查结论。
- 用户确认已玩过 GASP CMC 版、就是要的手感 → 进阶段1。
- **核对 GameAnimationSample 真实路径**（写进 progress 交接）：CMC 角色 `Content/Blueprints/SandboxCharacter_CMC`(+`_ABP`)；骨架 `Content/Characters/UEFN_Mannequin/Meshes/SK_UEFN_Mannequin`；MM 库+Chooser `Characters/UEFN_Mannequin/Animations/MotionMatchingData/`（`CHT_PoseSearchDatabases` + Dense/Sparse/Relaxed/ExtremeSparse 密度档）。样例 MM 插件：PoseSearch/AnimationLocomotionLibrary/Chooser/MotionWarping/AnimationWarping/CurveExpression。
- **阶段1 第1步完成**：TheManTest 启用 6 个 MM 插件（Pose Search / Animation Locomotion Library / Chooser / Motion Warping / Animation Warping / Curve Expression）。Mover/NetworkPrediction 不启（CMC 不需要，留给 FEAT-043）。
- **暂停点**：用户午休下机。下午从阶段1 第2步（Migrate `SandboxCharacter_CMC` 进 TheManTest）接续，步骤见 progress.md 交接。

### 2026-06-27-session52 — 阶段1 Migrate 完成 + 膨胀盘点

- **用户已 Migrate `SandboxCharacter_CMC`**。触发 UE「Missing Project Settings」警告（MetaHuman `Kellan_FaceMesh` 需要高骨骼数渲染设置）→ 让用户点 **Enable Missing**（写进 `DefaultEngine.ini [/Script/Engine.RendererSettings]`：`r.GPUSkin.Support16BitBoneIndex=True` / `r.GPUSkin.UnlimitedBoneInfluences=True` / `SkeletalMesh.UseExperimentalChunking=1`，纯增量、安全）。
- **盘点 Migrate 实际带进 `Content/` 的东西**（`Content/` 现共 6.9G）。**关键认知：演示膨胀不是孤立文件，而是 `SandboxCharacter_CMC` 蓝图硬引用**——`AC_VisualOverrideManager` 运行时在 Manny/Echo/Twinblast/Kellan 之间换皮、`AC_TraversalLogic` 引用 `LevelPrototyping`、SmartObjects 攀爬交互。**所以重新 Migrate 不会更干净**（依赖链一样全拉），想甩掉只能在阶段2 弃用整个 Sandbox 角色蓝图、只留 UEFN_Mannequin MM 数据自搭精简角色。
- **决策**：阶段1 不删不重迁，保留整坨先跑通 MM 验证手感；清理推迟到阶段2（换成自己的角色后，硬引用消失，整批删才安全无 broken ref）。

#### ⚠️ 阶段2 安全删除清单（换成自有精简角色后再删）

> 前提：阶段2 已弃用 `SandboxCharacter_CMC` 蓝图、自搭只引用 `UEFN_Mannequin` MM 数据的精简角色 ABP。届时下列资产不再被硬引用，可整批删。删前在 UE 里用 **Reference Viewer / Size Map** 复核无现役引用。

**纯演示膨胀（≈2.5G，删了最划算）：**
- `Content/Characters/Echo`（1.1G，换皮演示角色）
- `Content/Characters/Paragon`（705M，Twinblast 换皮）
- `Content/MetaHumans`（300M，Kellan 换皮 + Common）
- `Content/Characters/UE5_Mannequins`（350M，**待确认是否项目原有**——CLAUDE.md 原列 `Characters/Mannequins`，删前确认不是现役 SKM_Manny 来源）
- `Content/Levels/LevelPrototyping`（37M，演示关卡几何）

**GASP 演示蓝图 / Mover 版（小，但属噪音）：**
- `Content/Blueprints/SandboxCharacter_Mover` + `_Mover_ABP`（Mover 版，我们用 CMC）
- `Content/Blueprints/MovementModes`（872K，Mover 专用移动模式）
- `Content/Blueprints/SmartObjects`（444K，攀爬/坐凳交互）
- `Content/Blueprints/RetargetedCharacters`（484K，BP_Echo/Twinblast）
- `Content/Blueprints/GM_Sandbox` / `PC_Sandbox` / `Cameras`（演示 GameMode/Controller/相机，我们有自己的）
- `Content/Input`（GASP `IMC_Sandbox` + IA_*，我们有自己的 `Inputs/`，注意单复数别删错）
- `AC_VisualOverrideManager` / `AC_TraversalLogic`（换皮 + 攀爬演示组件，自有角色不挂）

**保留（MM 核心 + 阶段2/3 要用）：**
- `Content/Characters/UEFN_Mannequin`（1.7G，骨架 + mesh + **Pose Search 数据库**，MM 弹药库）
- `Content/Blueprints/SandboxCharacter_CMC` + `_ABP`（阶段1 验证用；阶段2 拆完逻辑后才删）
- `Content/Blueprints/Data`（Chooser / locomotion 数据）
- `Content/Blueprints/ControlRigs` / `AnimModifiers` / `AnimNotifies`（ABP 依赖）
- `Content/Misc/SandboxAnimCurveCompressionSettings`、`Content/_SystemSupport/CR_Mannequin_Body`（小，曲线/CR 依赖）
- `Content/Audio`（12M，脚步 Foley，可留可删，看是否接 foley notify）

#### ⚠️ 阶段2 同步禁用的插件（删演示资产后一并关）

清理时连同演示资产关掉这些 GASP 专用插件（关前确认无现役引用）：`Mover` / `NetworkPrediction` / `Locomotor`（Mover 版角色）、`SmartObjects` / `GameplayInteractions`（攀爬交互）、`RigLogic` / `LiveLink` / `LiveLinkControlRig` / `HairStrands`（Kellan MetaHuman）、`DrawDebugLibrary`（仅 BFL_HelpfulFunctions 用，若自有角色不引用则关）。**保留**：`PoseSearch` / `AnimationLocomotionLibrary` / `Chooser` / `MotionWarping` / `AnimationWarping` / `CurveExpression`（MM 核心）。

#### 插件踩坑（session52）

- Migrate 后 8 个蓝图编译失败（`BFL_HelpfulFunctions` / `SandboxCharacter_CMC_ABP` / `AC_SmartObjectAnimation` / `AC_TraversalLogic` / `BP_Kellan` / `Face_AnimBP` / `SandboxCharacter_Mover` / `_Mover_ABP`）——全因缺插件，`BFL_HelpfulFunctions`（缺 `DrawDebugLibrary`）失败连累 `SandboxCharacter_CMC_ABP`。
- 用户 UI 手动启插件点错俩相似名：`NetworkPredictionInsights`（应 `NetworkPrediction`）、`HairStrandsMutable`（应 `HairStrands`），还漏了 `Chooser` / `AnimationWarping`；`DrawDebugLibrary` / `RigLogic` / `LiveLinkControlRig` 是 **Experimental** 插件，Plugins UI 默认隐藏搜不到。
- **修法**：直接改 `TheManTest.uproject` Plugins 数组，对齐 GASP 的 `GameAnimationSample.uproject`（17 插件 + 本项目 GameplayAbilities = 18）。重启编辑器后全绿。

### 2026-06-27-session52 — ✅ 阶段1 完成（MM 跑通，手感通过）

- 修正插件后 8 个蓝图全部编译通过。
- **GASPTest 关卡 GameMode 配置**（仅此测试关卡）：`World Settings → GameMode Override = GM_Sandbox`、`Default Pawn Class = SandboxCharacter_CMC`、`Player Controller Class = PC_Sandbox`。根因：之前用项目默认 GameMode（`TheManPlayerController` 只认 `IMC_Default` + 路由给 `AFPSCharacterBase`），与 GASP 角色自带的 `IMC_Sandbox` 输入对不上 → 能进游戏但推不动。换 GASP 自己的 GM/PC 后输入通。
- Water Body Collision profile 警告（演示资产引用 Water 插件碰撞通道）：点 Add entry 写 DefaultEngine.ini，无害，与移动无关。
- **用户确认："手感完美"** → 阶段1（独立关卡 Manny 跑通 MM：走/跑/起步/停步/转身/pivot）**通过**。
- ⏭ **下一步：阶段2** —— 接进 `AFPSCharacterBase`（Leader 跑 MM ABP、Shadow/Legs 跟随；相机/装备/GAS 照常）。先勘查 `SandboxCharacter_CMC` 的 C++/蓝图 locomotion 逻辑（trajectory 生成 / MM 驱动），定"搬进 AFPSCharacterBase vs 抽 component"。

---

## 阶段2 勘察资料（GASP MM 架构还原，session52）

> GASP 是**纯蓝图工程**（无 C++ 源码，已确认无 `Source/`）。下方架构图由资产命名 + GASP 公开架构知识还原，`.uasset` 内部图表需用户在编辑器打开确认。

### MM 架构图（CMC 版）

```
SandboxCharacter_CMC（蓝图，建在 ACharacter+CMC 上）
├── 组件
│   ├── CharacterTrajectory 组件 ······ 生成轨迹（过去+预测路径）→ 喂 MM【核心】
│   ├── AC_PreCMCTick ················· CMC tick 前更新输入/轨迹（时序关键）
│   ├── AC_TraversalLogic ············· 攀爬（❌不需要）
│   └── AC_VisualOverrideManager ······ 换皮 Manny/Echo/Kellan（❌不需要）
├── 状态机（EventGraph 算出 → 经 BPI 喂 ABP）
│   ├── E_Gait（Walk/Run/Sprint）/ E_RotationMode（OrientToMovement/Strafe/Aim）
│   ├── E_Stance（Stand/Crouch）/ E_MovementState（Idle/Moving）/ E_MovementDirection
│   └── 每 Gait 对应不同 MaxWalkSpeed（CMC 参数）
└── 接口：BPI_SandboxCharacter_Pawn / BPI_SandboxCharacter_ABP（pawn↔ABP 双向）

SandboxCharacter_CMC_ABP（挂角色 mesh，Blueprints/SandboxCharacter_CMC_ABP）
├── ThreadSafeUpdate：经 BPI 从 pawn 拉状态 → 算 ABP 变量
└── AnimGraph：Pose History → Motion Matching 节点(◄Trajectory, ◄Chooser 选库)
        → Orientation/Rate Warping(AnimationWarping) → 脚 CR「CR_Biped_FootPlacement」

MM 数据（已迁入 Characters/UEFN_Mannequin/Animations/MotionMatchingData/）
├── CHT_PoseSearchDatabases（主 Chooser + Dense/Sparse/Relaxed/ExtremeSparse 密度档）
├── Schemas/PSS_*（Loops/Starts/Stops/Pivots/TurnInPlace/Idle/Jump…）
└── Databases/PSD_*（动画+schema 烘出的搜索库）
```

### 关键数据资产（Blueprints/Data/）
- 枚举：`E_Gait` / `E_RotationMode` / `E_Stance` / `E_MovementState` / `E_MovementMode` / `E_MovementDirection`(+Bias)
- 结构：`S_PlayerInputState` / `S_CharacterPropertiesForAnimation` / `S_ChooserOutputs` / `S_MovementDirectionThresholds`
- 公共库：`BFL_HelpfulFunctions`（需 DrawDebugLibrary）
- 脚贴地 CR：`Blueprints/ControlRigs/CR_Biped_FootPlacement`

### 阶段2 最小搬运集 vs 丢弃
- **必须**：CharacterTrajectory 组件 / Gait·RotationMode·Stance·MovementState 状态机 / MM ABP（MotionMatching+Chooser+Warping+脚CR）/ AC_PreCMCTick 时序 / MM 数据（已迁，复用）
- **丢弃**：Traversal / SmartObjects / VisualOverride 换皮 / Cameras（用 FP 相机）/ Mover·Slide / Crouch（可选先不要）

### 集成策略（建议 B，待勘察后定稿）
`AFPSCharacterBase` 已是标准 ACharacter+CMC，与 GASP CMC 同底，可移植：
- **C++**（Claude 写）：加 `CharacterTrajectory` 组件 + 移植 Gait/RotationMode/MovementState 状态机（读 CMC velocity/input）+ 复刻 AC_PreCMCTick 更新时序。
- **ABP**（用户搭，Claude 给图）：Leader mesh 精简 ABP——MotionMatching + Trajectory + 复用 `CHT_PoseSearchDatabases` + Warping + 脚 CR，砍 traversal/smartobject/camera。
- **Shadow/Legs**：FEAT-038 `SetLeaderPoseComponent` 已就绪，自动跟随。

### ⏭ 待用户在编辑器确认（Claude 读不了 .uasset）
1. **`SandboxCharacter_CMC`** Components 面板：确认有无 `CharacterTrajectory` 组件 + `AC_PreCMCTick`；各 Gait 的 MaxWalkSpeed（Walk/Run/Sprint 三档速度值）。
2. **`SandboxCharacter_CMC_ABP`** AnimGraph：MotionMatching 节点前后接什么、Trajectory 引脚来源、Database 是否连 `CHT_PoseSearchDatabases`；ThreadSafeUpdate 是否经 `BPI_SandboxCharacter_ABP` 从 pawn 取状态。
> 看完这两个定：移植进 AFPSCharacterBase vs 抽 component；C++ 具体写什么。

---

## 阶段2 勘察实测（session53，用户贴出 Components + Movement + ABP AnimGraph 全量导出）

### Components（实测，对照"最小搬运集"）
- **`AC_PreCMCTick` ✓ 存在** —— CMC tick 前更新输入/轨迹的时序组件，**必须复刻**。
- **`MotionWarping` ✓**（traversal 用，基础移动不强依赖）。
- **⚠️ 无独立 `CharacterTrajectory` 组件**！轨迹不是组件生成，而是 **ABP 自己的 `Trajectory`(TransformTrajectory 结构) 变量** → 喂给 Pose History 节点的 `TransformTrajectory` 引脚。即轨迹在 ABP EventGraph（或 AC_PreCMCTick）里算。**→ C++ 移植核心未知点：`Trajectory` 怎么算的(下面待确认)。**
- 不需要：`AC_TraversalLogic`/`AC_SmartObjectAnimation`/`VisualOverride`/`BP_VisualOverrideManager`（换皮+攀爬）、`GameplayCamera`/`SpringArm`/`Camera`（用本项目 FP 相机）、`AC_FoleyEvents`（脚步音效，可选）。

### Gait 速度（pawn 的 Movement 类，FVector = 前/侧/后 三分量）
- Walk `200/180/150`、Run `500/350/300`、Sprint `700/700/700`、Crouch `225/200/180`
- `Strafe Speed Map Curve = Curve_StrafeSpeedMap`（按方向插值速度）
- 机制：按当前 Gait+移动方向算 `MaxWalkSpeed` 写回 CMC（本项目 gait 速度照填这套或自调）。

### `SandboxCharacter_CMC_ABP` AnimGraph 链（输出 Root → 输入，实测节点顺序）
```
Root ← PoseHistory(PoseSearchHistoryCollector：收集 foot_r/foot_l/thigh_r/thigh_l/spine_05/pelvis + "Phase"曲线；TransformTrajectory◄变量Trajectory)
     ← ComponentToLocalSpace ← LegIK(ik_foot_l/r, foot_l/r, 膝铰链Z) ← FootPlacement(实验, ik_foot_root/pelvis/ball_l/r, contact_l/r曲线驱动)
     ← LocalToComponentSpace ← RemapCurves(contact_l/r=(1-x)*100，喂 FootPlacement 落脚速度)
     ← OffsetRootBone(实验, 根骨偏移让脚更稳) ← DefaultSlot(traversal 蒙太奇注入点) 
     ← AimOffset(ApplyMeshSpaceAdditive, Enable_AO 开关, BS_Neutral_AO_Stand, Dead Blending 防 pop)
     ← Lean(ApplyMeshSpaceAdditive, Get_LeanAmount, BS1D_Additive_Lean_Run)
     ← BlendListByInt(ActiveChildIndex◄变量 LocomotionSetup)：
         0 → **MotionMatching 节点**（内部 BlendStack 图：OrientationWarping[spine_01→05+neck_01/02+head] → Steering → OffsetRoot；OnUpdate=Update_MotionMatching 跑 Chooser 选 DB；Database=AlwaysDynamic）
         1 → State Machine+BlendStack 实验路（StateMachine 纯逻辑无 pose → Inertialization → TwoWayBlend ← BlendStack_3，bAlwaysUpdateChildren）
```
- **两条 locomotion 路由 `LocomotionSetup` 切**：0=纯 MM 节点（简单），1=状态机+Chooser+MM+BlendStack 实验组合。**接本项目先用 0 路即可。**
- BlendStack 内/Root 都用 `MDT_UEFN_Mannequin` 镜像表 + `SK_UEFN_Mannequin` 的 `FastFeet+Root_Weight`/`FastHead_Weight` BlendProfile。

### 关键认知：ABP 完全自包含
AnimGraph 全部读 **ABP 自己的变量/函数**，不直接 cast pawn 类做硬逻辑：
- 变量：`Trajectory` / `LastNonZeroVelocity` / `CurrentDatabaseTags` / `BlendStackInputs` / `LocomotionSetup` / `MovementMode` / `Stance`/`Stance_LastFrame` / `Gait`/`Gait_LastFrame` / `MovementDirection`(+LastFrame) / `TargetRotation`(+OnTransitionStart)
- 函数：`IsMoving` / `Get_DesiredFacing` / `Get_AOValue` / `Enable_AO` / `Get_LeanAmount` / `IsPivoting` / `ShouldTurnInPlace` / `IsAnimationAlmostComplete` …
- 这些在 EventGraph `ThreadSafeUpdate` 里经 `BPI_SandboxCharacter_ABP` 从 pawn 拉。
- **骨骼名全 Manny 标准**：`spine_01→05`/`neck_01/02`/`head`/`pelvis`/`thigh_l/r`/`foot_l/r`/`ball_l/r`/`ik_foot_root`/`ik_foot_l/r` → 阶段3 BBBAimIK + 上半身武器层直接套这些名。

### 集成结论（定稿：复用 ABP，让 AFPSCharacterBase 充当"喂状态"的 pawn）
ABP 巨大且自包含 → **不重建 ABP**。最省接法：
1. **C++（AFPSCharacterBase）**：移植 GASP 状态计算——Gait/RotationMode/Stance/MovementState/MovementDirection 枚举 + gait 速度向量驱动 CMC `MaxWalkSpeed` + 复刻 `AC_PreCMCTick` 时序（用 CMC `OnMovementUpdated`/自定义 PreCMC tick）+ trajectory 生成。状态做成 BlueprintReadOnly / 实现 `BPI_SandboxCharacter_Pawn` 接口。
2. **ABP**：复用迁来的 `SandboxCharacter_CMC_ABP`（复制改名 `ABP_Body_GASP` 之类），其 `ThreadSafeUpdate` 改从 `AFPSCharacterBase` 读状态。**AnimGraph 一字不改。**
3. **三件套**：ArmsMesh(Leader) 挂这个 ABP，Shadow/Legs `SetLeaderPoseComponent` 跟随。相机/装备/GAS 照常。

### ⏭ 仍需在编辑器确认（C++ 移植前必看，Claude 读不了 .uasset 逻辑图）
1. **`Trajectory` 变量怎么算** —— 打开 `SandboxCharacter_CMC_ABP` EventGraph + `AC_PreCMCTick` 蓝图，看轨迹生成（是 `UCharacterTrajectoryComponent` 还是 `AnimationLocomotionLibrary` 的 GenerateTrajectory 读 CMC velocity/accel）。这是 C++ 要复刻的核心。
2. **Gait→CMC 速度怎么写回** —— 看 `SandboxCharacter_CMC` EventGraph / `BPI_SandboxCharacter_Pawn` 接口里"按 Gait+方向设 MaxWalkSpeed"的逻辑。
3. **`BPI_SandboxCharacter_ABP` / `_Pawn`** 两个接口的函数签名（决定 AFPSCharacterBase 要实现/暴露哪些）。

---

## ABP EventGraph 实测（session53）+ 架构决策转向

### ABP EventGraph 调用链（实测）
- `BlueprintUpdateAnimation`(游戏线程) → set `HasOwningActor`(=IsValid(TryGetPawnOwner)) → 若有效:
  → `Update_CVarDrivenVariables`（缓存 CVar，线程安全限制）
  → **`Update_PropertiesFromCharacter`**（★从 pawn 拉所有状态：velocity/accel/gait/stance/rotationmode/trajectory…）
  → 若 `UseThreadSafeUpdateAnimation`==false → **`Update_Logic`**（★算派生状态：MovementDirection/AOValue/LeanAmount/LocomotionSetup 等）
- `BlueprintPostEvaluateAnimation` → 仅 debug（TransitionHistory 字符串历史）。

→ **重活全在两个 BP 函数 `Update_PropertiesFromCharacter` + `Update_Logic` 里**（以及 pawn 端 EventGraph/AC_PreCMCTick 的 trajectory+gait 计算）。这些是几百节点的纯蓝图。

### ⚠️ 架构决策转向（重要，推翻"C++ 全移植"）
原计划"C++ 移植 Gait/Stance/Trajectory 状态计算"——**实测后否决**：这套逻辑是 GASP 几百个蓝图节点（pawn EventGraph + AC_PreCMCTick + ABP 的 Update_PropertiesFromCharacter/Update_Logic），逐节点抄进 C++ **工作量巨大且极易抄错**，得不偿失。

**改走「蓝图组合」路（最省最稳）**：GASP 逻辑**留在蓝图**，C++ 只做已有的胶水（GAS/装备/相机/三件套）。具体两条候选，下次开机和用户定：
- **C1（推荐·重父类）**：把 `SandboxCharacter_CMC`（蓝图）**reparent 到 `AFPSCharacterBase`(C++)**。这样 GASP 整套 pawn 端 locomotion BP 逻辑（组件 AC_PreCMCTick/MotionWarping + EventGraph gait/trajectory）原样保留，AFPSCharacterBase 的 GAS/装备/相机/三件套 C++ 能力叠加进来。风险：reparent 兼容性 + 两边组件/输入冲突需理顺；FP 相机替换掉 GASP 的 GameplayCamera/SpringArm。
- **C2（抽组件）**：把 GASP pawn 端 locomotion 逻辑搬成一个**蓝图 ActorComponent**（gait 状态 + trajectory + AC_PreCMCTick），挂到现有 `BP_FPSxxx`；角色 BP 实现 `BPI_SandboxCharacter_Pawn/_ABP` 转发给该组件。改动分散但不动现役角色继承链。

两条都**复用 ABP 不改 AnimGraph**。C1 改动集中、最接近 GASP 原样（手感最有保障），C2 侵入小但要搬 BP 逻辑+接两个接口。

> 即：FEAT-042 阶段2 主要是**蓝图工程**（reparent / 搬组件 / 接接口 / 挂三件套 Leader-Follower），C++ 改动远小于原估。已无需再贴 BP 函数内部导出——`Update_PropertiesFromCharacter`/`Update_Logic`/pawn EventGraph 当黑盒整体复用即可。

### ⏭ 下次开机第一件事：和用户定 C1 vs C2
- C1 reparent：先备份 `SandboxCharacter_CMC`，试 reparent 到 `AFPSCharacterBase`，看编译/组件冲突。通了则最省。
- C2 抽组件：稳但搬运量中等。
- 定了再排具体步骤（相机替换、输入 IMC 合并、三件套 Leader 挂 ABP、Shadow/Legs SetLeaderPoseComponent）。

---

### 2026-06-27-session54 — 阶段2 开工：C1 reparent 跑通（reparent + FP 相机 + 组件共存验证通过）

按 session53 拍板的 **C1 路线**（reparent `SandboxCharacter_CMC` → `AFPSMaintenanceWorker`）逐步执行，**纯编辑器操作，无 C++ 改动**：

**1. 备份 + reparent（通过）**
- 复制 `SandboxCharacter_CMC` 留底。
- Reparent 父类 → `AFPSMaintenanceWorker`（经 `AFPSCharacterBase`）。**无任何报错**。

**2. 组件实测（reparent 后，对照归类）**
- C++ 组件全部正确出现、无命名冲突：`BodyRoot`/`ShadowBodyMesh`/`LegsMesh`/`HeadCamera`/`EquipmentManager`/`ScanEffect`。
- **关键利好确认**：GASP Manny mesh + `SandboxCharacter_CMC_ABP` 挂在**继承自 ACharacter 的 `Mesh`(CharacterMesh0)** 上 —— 与 `AFPSCharacterBase` 的 Leader（`GetMesh()`）是同一个组件，**reparent 后 MM 宿主自动落到 Leader 上**，无需重挂。
- 保留：`AC_PreCMCTick`（MM 时序核心）、`MotionWarping`、`Mesh`。
- 删除：`GameplayCamera`/`SpringArm`/`Camera`（GASP 三人称相机架，换 FP `HeadCamera`）。可后续删：`VisualOverride`/`BP_VisualOverrideManager`/`AC_TraversalLogic`/`AC_SmartObjectAnimation`/`AC_FoleyEvents`。

**3. 相机冲突修复（通过）**
- 删 GASP 三个相机组件后，蓝图编译报错：GASP 的 **`SetupCamera` 自定义函数**里 `ActivateCameraForPlayerController`（引用已删 `GameplayCamera`）+ `Activate`（引用已删 `Camera`）节点断裂 → `' Target ' must have a connection` Fatal。
- **修法**：把 `SetupCamera` 函数体清空成空函数（删除入口节点外所有节点）。`Event Possessed_Client` 仍调它但空操作。删三相机后 `HeadCamera` 成唯一 `UCameraComponent` → 自动成为视图目标。
- 同理 `SetupCamera`/相机 swap 逻辑（scroll 切相机风格）属 GASP 演示，不影响。

**4. PIE 验证（GASPTest 关卡，GM_Sandbox/PC_Sandbox，通过）**
- ✅ FP 相机生效（HeadCamera）。
- ✅ MM 走/跑/停手感正常（reparent 后 MM 管线完好）。
- ⚠️ 已知遗留（非本步问题）：FP 下相机轻微穿模 —— 因 Leader(`GetMesh()`) 现挂 **GASP 完整 Manny 全身 mesh** 且 `OnlyOwnerSee`，相机眼高卡进 Manny 头部。**手臂隔离（`ArmsHiddenSections`/物理拆 mesh）尚未做**，Shadow/Legs 也未赋 mesh。按计划留到**阶段4 换自有 mesh** 时一并解决，占位 Manny 上不做拆分。

**结论**：阶段2 核心（C1 reparent + FP 相机 + C++ 组件共存 + MM 存活）**验证通过**。下一步 **输入合并**。

**备查：相机为何不直接绑 head 骨骼（FEAT-038 决策复述）**
直接把相机绑 head 骨骼实现 FP 的问题：① head bob（骨骼每帧被 locomotion/idle 动画驱动，相机抖，MM 下尤甚）；② 俯仰支点偏离（head 在脖子前上方，加 pitch 时绕偏移支点画弧，低头视角被甩向前下方）；③ 动画自带 yaw/roll/pitch 与控制器旋转打架（视角歪斜、瞄准与准星错位）；④ 根骨骼不在原点会整体偏位（FEAT-007）。现方案：相机挂 capsule、固定眼高 `(0,0,77)`、`bUsePawnControlRotation` → 零 head bob、俯仰干净、不与动画冲突。

### 阶段2 输入合并方案（session54 定，待执行）

GASP 输入实测（pawn EventGraph 直接绑增强输入，IA 在 `/Game/Input/`，非本项目 `/Game/Inputs/`）：
- `IA_Move`(+`IA_Move_WorldSpace`) → `AddMovementInput`；`IA_Look`(+Gamepad) → `AddControllerYaw/PitchInput`；`IA_Jump` → Jump/Traversal。
- **gait 开关** `IA_Sprint`/`IA_Walk`/`IA_Strafe`/`IA_Crouch`/`IA_Aim` → set `CharacterInputState`(`S_PlayerInputState`：`WantsToSprint/Walk/Strafe/Aim/Crouch`) → 经 `UpdateInputState_Server` → 驱动 MM 的 Gait/RotationMode（**MM 手感来源**）。
- pawn 有 `SetupInput` 自定义函数（`Event Possessed_Client` 调）添加 `IMC_Sandbox`。

**冲突**：真集成走 `TheManPlayerController`（BeginPlay 加 `IMC_Default`）+ `AFPSCharacterBase::SetupPlayerInputComponent`（绑本项目 IA_* via PC getter）。若 GASP 的 `IMC_Sandbox` 也加，移动会双重输入。

**B1 已验证通过（session54）**：①删 Tick 滑行块（C++，Live Coding）②清空 pawn `SetupInput` 函数（GASP 不再加 IMC_Sandbox，其 EventGraph 输入节点永不触发）③GASPTest 用 `BP_TheManGameMode`（Default Pawn 改 `SandboxCharacter_CMC`，直接 PIE 走 `GetDefaultPawnClassForController` 的"SelectedCharacterID 空→回退 DefaultPawnClass"分支，不必建新 GM）+ `BP_TheManPlayerController`（加 IMC_Default）。PIE：WASD/视角/跳 由我方 C++ 驱动、MM 跑步 locomotion 正常、无滑步。

**选定方向 = Option B（本项目输入为主，喂 GASP gait 状态）**，理由：保持 FEAT-016 输入架构（Controller 注册表 + Character 自绑 + IMC_Default），契合 session53「AFPSCharacterBase 充当喂状态的 pawn、复用 ABP 不改 AnimGraph」定调；GASP 输入系统降级为被我们驱动的消费者。增量步骤：
- **B1**：清空 pawn `SetupInput` 函数（同 SetupCamera 手法）→ GASP 不再加 `IMC_Sandbox` → 其 EventGraph 输入节点 IA_* 永不触发（无害，**不必删**）。GASPTest 关卡 GameMode 改回项目默认 `TheManGameMode`/`TheManPlayerController`（加 `IMC_Default`）。验证我方 C++ Move/Look/Jump 驱动 MM（默认 gait=Run 即可）。
- **B2**：C++ 喂 gait —— `AFPSCharacterBase` 的 Sprint 等改为写 GASP `CharacterInputState`（经 `BPI_SandboxCharacter_Pawn::Set_CharacterInputState` 接口或等价），让 walk/run/sprint/strafe/aim 生效。注意：现有 `StartSprint/StopSprint` 直接改 `MaxWalkSpeed` 会与 GASP 的 gait→MaxWalkSpeed 逻辑打架，B2 改为喂 `WantsToSprint`。
- 我方射击/切武器/扫描输入（`PrimaryFire`/`SwitchEquipment`/`Interact`）仍走 C++ 绑定 + `IMC_Default`，不变。

### 2026-06-27-session54 — ⚠️ 重大根因发现 + 方向从 Option B 转 Option A

**症状**：reparent + FP 相机后，转视角时角色身体/影子**不跟随准星转**。依次尝试均无效：① BP Class Defaults `bUseControllerRotationYaw=true`；② 确认 CMC `Orient Rotation to Movement=false` / `Use Controller Desired Rotation=false`；③ 清空 GASP `UpdateRotation_PreCMC` 函数；④ C++ 每帧 `SetActorRotation` 强制 yaw=控制器 yaw。**全部不动。**

**根因（定性）**：GASP 的 `SandboxCharacter_CMC_ABP` AnimGraph 用 **OffsetRootBone + OrientationWarping + Steering** 节点，把 **mesh 视觉朝向与胶囊体(actor)旋转解耦**（这是 GASP 平滑转身手感的来源）。所以外部无论怎么转 actor（引擎 `bUseControllerRotationYaw` / CMC / C++ `SetActorRotation`），胶囊体转了但 mesh 被 OffsetRootBone 抵消、保持原朝向。**mesh 只在 GASP 自己的状态管线（输入→`CharacterInputState`→`Update_Logic`→ABP 的 Steering）给出目标朝向时才转。**

**连带教训**：我们为走 Option B（我方输入/旋转驱动）清空了 `SetupInput`（→不加 IMC_Sandbox→`CharacterInputState` 恒空）和 `UpdateRotation_PreCMC`（→无朝向目标），正好把这条管线切断 → mesh 彻底失去朝向驱动。stage 1「手感完美」恰恰因为这条管线完整。

**结论：C1「我方驱动、绕开 GASP 旋转」从架构上行不通。转 Option A** ——让 GASP 整条 locomotion/输入/旋转管线**原样跑**，**只替换第三人称相机为 FP HeadCamera**；我方开火/切武器/扫描输入之后叠加进 GASP 的 `IMC_Sandbox`，GAS/装备走 `AFPSCharacterBase` 继承能力、不碰 GASP locomotion。代价：放弃 FEAT-016「IMC_Default 统一输入」纯洁性，改用 GASP `IMC_Sandbox` 当主输入上下文；换来零对抗、locomotion 完整。

**Option A 恢复/验证步骤（session54 末）**：
1. 丢弃被拆坏的 `SandboxCharacter_CMC`（`SetupInput`/`UpdateRotation_PreCMC` 节点已删，不可逆），改用 reparent 前的备份 `SandboxCharacter_CMC_BACKUP` 复制改名回来（管线完整）。
2. reparent → `AFPSMaintenanceWorker`。
3. **只动相机**：删 `GameplayCamera`/`SpringArm`/`Camera` + 清空 `SetupCamera`；**`SetupInput`/`UpdateRotation_PreCMC` 不动**。
4. GASPTest World Settings 用回 `GM_Sandbox`/`PC_Sandbox`（IMC_Sandbox + 输入管线完整）。
5. PIE 预期：FP 视角（HeadCamera 唯一相机自动生效）+ 转视角身体/影子朝准星（strafe 正常）+ MM 手感同 stage 1。

**C++ 现状**：session54 删了 `FPSCharacterBase::Tick` 的 FEAT-039 滑行块（保留，正确清理）；临时加的强制 yaw 块已撤回。`bHasMoveInput` 等仍是死字段待收尾清理。

### 2026-06-27-session54 — 朝向认知彻底厘清 + FP 方案定案（推翻前面 actor 旋转的所有折腾）

**一连串误判的终点**：前面在「身体不跟相机转」上折腾了 bUseControllerRotationYaw / CMC 开关 / gut UpdateRotation_PreCMC / C++ 强制 yaw，全错。真相经实测厘清：

1. **GASP mesh 朝向归 ABP 的 OffsetRootBone/Steering 管**，外部转 actor 无效（前面已记）。
2. **GASP 默认不是"站定跟视线"**：默认 OrientToMovement 类行为——站定转相机身体不动（相机可绕角色转，因控制器旋转独立），一旦移动身体朝**视线/屏幕前方**对齐（实为 strafe-when-moving，非朝移动方向）。所以"站定 90° 下半身不动"是**正常**，不是 bug。
3. **第一人称的"手臂/身体跟相机"不靠 actor yaw，靠两层**：
   - **下半身(腿/影子)**：MM strafe locomotion（移动时朝视线；站定 idle）。
   - **上半身(手臂/枪/脊柱)**：武器层 + **脊柱 BBBAimIK**（阶段3）即时拧向准星——**这才是 FP 手臂跟相机的来源**，IK 即时、不滞后。
4. **GASP 自带 TurnInPlace 已实测可用**：按住 **Aim(鼠标右键)** → 进 Aim 模式 → 站定转视角下半身 turn-in-place 跟过来。默认模式不触发是因为没进 Aim/Strafe 态（没"身体该朝视线"的诉求 → 无差异要补）。
5. **快速甩鼠标下半身 turn-in-place 追不上 = 动画驱动的固有滞后**，每个 turn-in-place 游戏都有。对 FP **基本无害**：①FP 看不到自己腿 ②上半身/枪靠 AimIK 即时跟、不滞后 ③**开火走相机/准星射线(现有 GA_Shoot)，与 mesh 视觉朝向无关 → 滞后不影响命中**。唯一露馅处是地上影子甩枪慢半拍（可调转速）。

**FP 朝向方案定案（session54 用户拍板）**：
- **让"身体跟视线"模式常驻**（FPS 不用一直按右键）：常驻 Aim/Strafe 态（喂 `WantsToAim`/`WantsToStrafe` 默认 true）。
- **调快 body rotation interp 速度**，减小快速甩枪时影子/下半身滞后。
- 手臂跟准星 = **阶段3 上半身武器层 + BBBAimIK**（FP 手感主体；下半身滞后被其完全盖住）。
- 不再在 actor yaw / CMC 旋转上做任何事——全部交给 GASP rotation mode + turn-in-place + 阶段3 上半身 IK。

**执行结果（session54）**：已在 `SandboxCharacter_CMC` 的 `CharacterInputState` 默认值里把 strafe/aim 常驻打开（`WantsToStrafe`/`WantsToAim` 默认 true），PIE 确认默认即"身体跟视线 + 站定 turn-in-place"，用户确认「现在可以了」。**body rotation interp 速度调快留作后续打磨**（快速甩枪影子滞后，非阻塞）。⚠️ 注意：若日后发现常驻 Aim 带 ADS 降速/FOV 副作用，再改为常驻 Strafe + 单独配站定 turn-in-place。

**阶段2 朝向/相机/MM 基座到此打通**。剩余阶段2：①三件套 Shadow/Legs Follower（赋 mesh + `SetLeaderPoseComponent` C++ 已就绪，验证影子/腿跟随）②我方输入并进 `IMC_Sandbox`（开火/切武器/扫描 IA + 绑定）③GAS/装备接上。

### 2026-06-27-session55 — 阶段2 第1项：三件套 Shadow Follower 跑通（含 VisualOverride 换皮系统踩坑）

**目标**：给 `ShadowBodyMesh` 赋 Manny mesh，验证 FEAT-038 Leader/Follower 在 MM 下成立（地上完整人形影子跟 MM 动）。C++ 侧 `ShadowBodyMesh->SetLeaderPoseComponent(GetMesh())` + `OwnerNoSee`/`bCastHiddenShadow` 早已就绪，纯编辑器操作。

**编辑器操作**：`ShadowBodyMesh` → Skeletal Mesh = `SK_UEFN_Mannequin`（同 Leader）、Anim Class 留空（Follower）、相对 Rotation Yaw=-90 + Location Z=-88。

**⚠️ 踩坑：出现两个影子 + 改 Leader `Mesh` 任何设置都无效**
- 症状逐步排查：①关 Leader `Mesh`(CharacterMesh0) 的 Cast Shadow → 仍两个影子；②清空 `Mesh` 的 SkeletalMesh → 运行时仍有 mesh+影子；③关 `Mesh` 的 Visible → 仍有 mesh+影子。在 BP 里改 Leader `Mesh` 的一切（mesh/可见/投影）运行时全被还原。
- **根因 = GASP 换皮系统 `BP_VisualOverrideManager`（组件面板里的一个 ActorComponent）在 BeginPlay 运行时强制套用默认外观**，覆盖角色 `Mesh` 组件的 SkeletalMesh / Visible / Cast Shadow。配套的 `VisualOverride` 是个 **Child Actor Component**（Child Actor Class 默认空，由管理器运行时驱动）。第二个 mesh+影子就是这套系统强加的。
- **解法**：组件面板删掉 **`BP_VisualOverrideManager`** + **`VisualOverride`**（两个都是 session52 清单里"可后续删"的换皮演示件）→ 编译（用户实测**无 EventGraph 报错**，干净删除）。删后 Leader `Mesh`(CharacterMesh0) 终于按 BP 设置生效：Visible 开、Cast Shadow 关、Manny + `SandboxCharacter_CMC_ABP` 跑 MM。
- **教训给后续（克隆其他角色 / 重新 Migrate 时必踩）**：GASP 的 `SandboxCharacter_CMC` 的"渲染外观"和"动画宿主"是拆开的——`Mesh`(CharacterMesh0) 只是 MM 动画宿主，可见外观由 `BP_VisualOverrideManager` + `VisualOverride` Child Actor 运行时套。接本项目 FEAT-038（要 Leader `Mesh` 自己当可见手臂 + `ShadowBodyMesh` 投影）**必须先删这套换皮系统**，否则它运行时覆盖、且让 BP 调试看起来"改了没用"。

**结果**：投影只剩 `ShadowBodyMesh` 一个，完整人形影子随 MM locomotion 实时动。用户确认「一切顺利」。`LegsMesh` 暂不赋（赋了与 Leader 全身 mesh 重叠，腿/手臂材质段分离留阶段4）。

### 2026-06-27-session55 — 阶段2 第2项：输入合并（GASP IA 并进 IMC_Default，单一 context）

**方案定案（用户拍板，优于 session54 的双 IMC/IMC_Sandbox 设想）**：不用 GASP `IMC_Sandbox`，也不用双 IMC 叠加——**把 GASP 的移动 IA 直接塞进我方 `IMC_Default`，单一 context 统一输入**，最贴合 FEAT-016。

**关键认知**：增强输入里 IMC 只是"按键→IA"映射表。GASP 的移动/gait 逻辑挂在 pawn **EventGraph**，监听 GASP 的 IA 资产（`/Game/Input/` 的 `IA_Move`/`IA_Look`/`IA_Sprint`…）。换哪个 IMC 承载这些 IA 映射无所谓——只要 IMC_Default 把 WASD 映射到 GASP 的 `IA_Move`，那些 EventGraph 节点就触发、MM 就动。等价于用 IMC_Sandbox，但只需一个 context 且我方战斗 IA 共存。

**编辑器操作**：`IMC_Default` 加 GASP 的 `IA_Move`(WASD)/`IA_Look`(Mouse)/`IA_Jump`(Space)/`IA_Sprint`(Shift)；**移除我方原 `IA_Move`/`IA_Look` 映射**（避免双重移动/视角双倍灵敏）；保留我方 `IA_PrimaryFire`/`IA_SwitchEquipment`/`IA_Interact`。GASPTest 关卡用 `BP_TheManGameMode`/`BP_TheManPlayerController`（BeginPlay 加 IMC_Default）；GASP pawn `SetupInput` 保持空（不加 IMC_Sandbox）；**保留 pawn EventGraph 的 `EnhancedInputAction IA_*` 节点**（GASP 移动响应逻辑，靠它们复活）。我方 C++ `SetupPlayerInputComponent` 的 Move/Look 绑定留着无害（对应 IA 无键位映射→不触发）。

**验证**：PIE WASD/鼠标/Shift → GASP MM 移动/视角/跳/冲刺正常，身体跟视线、手感同 stage1。**开火/切武器/扫描未验**（角色当前无武器，待第3项 GAS/装备接上一并测）。

**⏭ 阶段2 剩余**：③GAS/装备接上——`AFPSMaintenanceWorker`(reparent 后继承)的 ASC(PlayerState 取)/EquipmentManager/CharacterData/InitGEClass 已是基类能力，需在 GASP 角色 BP 填 `InitialEquipmentClasses`(BP_TestGun/BP_RepairGun) + `CharacterData` + `InitGEClass`(GAS 血量初始化) + 武器挂载 socket 对齐 Manny 骨骼；装上武器后回头验开火/切武器/扫描。

### 2026-06-27-session55 — 自有 mesh 换 Manny 骨架探索 + retarget 查证 + 决策（选 A：Manny 占位先推功能）

用户想提前做阶段4（把自有角色 mesh `SKM_CyberpunkMetalhead` 换上去），因为占位 Manny 全身 FP 穿模、头挡视角看不清效果。围绕"怎么把自有 mesh 弄到 UEFN Manny 骨架"做了一轮探索，**结论：UE5 没有省事路，决定推迟到最后用 Blender 重绑**。

**骨架兼容性实测（自有 CyberpunkMetalhead 骨架 ↔ UEFN Manny）：**
- 两骨架同谱系：都有 `index_metacarpal_l` 掌骨（MetaHuman/UEFN 谱系特征）+ 五节脊柱 `spine_01→05`；ref pose 截图对比"差不多"。
- **Follower 路 OK**：`ShadowBodyMesh` 赋自有全身 mesh + `SetLeaderPoseComponent(Manny Leader)` → PIE 影子正常跟 MM（leader pose 按骨骼名复制，跨兼容骨架 UE 带 ref pose 补偿，影子上扭曲不明显）。
- **Leader 直接跑 ABP 路扭曲**：`Mesh`(Leader) 赋自有手臂 mesh + Manny `SandboxCharacter_CMC_ABP` → PIE 手臂**扭曲**。根因：Leader 直接被 ABP 驱动、无 follower 的 ref pose 补偿 → ref pose 哪怕小差异（A/T pose 手臂角度）直接成蒙皮变形。
- 编辑器 viewport 里 ShadowBodyMesh 手臂错位是**假象**（viewport 不跑 BeginPlay 的 SetLeaderPoseComponent，显示各自 ref pose 重叠交叉），判断只看 PIE。
- **配 Compatible Skeletons（自有骨架 Compatible 列表加 UEFN Manny）仍扭曲** → 确认是 ref pose 几何差异，Compatible 只解决"骨架认可/动画能播"，不解决蒙皮变形。

**retarget mesh 查证（关键结论，避免下个 agent 重走）：**
- 建了 IK Retargeter `RTG_SKM_CyberpunkMetalhead`（Source `IK_SKM_CyberpunkMetalhead_FullBody` → Target `IK_UEFN_Mannequin`），retarget pose 视口对齐 OK，末尾 `Success! ready to transfer`（root bone None 警告是 root motion 给动画用的，无害）。
- **但 UE5 的 IK Retargeter 只 retarget 动画/Pose，不生成"重新蒙皮到新骨架的 mesh 几何资产"**。右键 mesh 搜 `retarget` 无此项——功能本就不存在。查证：[Auto Retargeting 5.7 文档](https://dev.epicgames.com/documentation/unreal-engine/auto-retargeting-in-unreal-engine) + [Epic forum 同款问题帖](https://forums.unrealengine.com/t/how-can-i-retarget-a-skeletal-mesh-to-a-shared-skeleton/696100) 均确认无内置 mesh 几何 retarget。
- **反向"把 GASP 动画 retarget 到自有骨架"对 MM 不可行/最贵**：GASP 是 Motion Matching，靠 Pose Search Database（按 Manny 骨架烘）。retarget 动画后要重烘所有 PSD（Dense/Sparse/Relaxed 多档）+ 改整个 ABP（Motion Matching/Pose History 骨骼名、Chooser、OrientationWarping spine 链）= 几百动画 + 多 DB + 超复杂 ABP 重制。正是 FEAT-042 选型当初否决的方向（archive 原话："mesh 在 Blender 重绑 Manny 比 retarget 几百动画+重建 Pose Search DB 省太多"）。
- **唯一可靠路 = Blender 把自有 mesh 重蒙皮（weight transfer）到 UEFN Manny 骨架**（比拆分难，新手有门槛）。

**决策（用户拍板）= 选 A：Manny 占位先推功能**
- 换 mesh 是纯视觉层、不阻塞功能 → 自有 mesh 重绑 Manny 推迟到**阶段4 视觉收尾**（Blender 重蒙皮）。
- **用户下一步计划**：先把 **GASP 的 Manny mesh 物理拆成 手臂 / 影子(全身) / 下半身** 三件套占位（同 Manny 骨架，拆完直接和 ABP/MM 兼容、**无扭曲**）→ Leader=Manny 手臂 mesh、ShadowBodyMesh=Manny 全身、LegsMesh=Manny 腿 → FP 只看手臂(消穿模)、低头看腿、影子完整。拆法同 session46 物理拆流程（Blender 按骨骼框选 → P 分离 → 导回指定 Manny 骨架、Add Leaf Bones 去勾）。
- **外部资源**：用户有同学用过 Motion Matching + 不同 mesh，可能帮忙处理自有 mesh 重绑 Manny 那步（阶段4 时联系）。
- 那个 `RTG_SKM_CyberpunkMetalhead` IK Retargeter 不用删，阶段4 真要 retarget 普通动画时还能用。

### 2026-06-28-session56 — 阶段2 第3项（接装备）开工规划 + 阶段3 上半身架构理清（纯规划，无代码改动）

> 本会话纯讨论 + 定方案 + 给操作清单，零代码/编辑器改动（用户困了，下午回来执行）。

**1. 上半身武器层怎么叠在 MM 之上（阶段3 架构定调）：**
- 核心节点 = **Layered blend per bone**，加在 `SandboxCharacter_CMC_ABP` AnimGraph **最末端输出前**：
  - Base Pose = MM 全身输出（整套 PoseHistory→MotionMatching→Warping→脚CR 原样不动，下半身/骨盆保留 MM）
  - Blend Pose = 上半身武器层（`ALI_WeaponAnim` 的 `WeaponAimOffset` 持枪/瞄准 pose）
  - BlendWeight = 从 `spine_01` 起 = 1.0（脊柱及以上换持枪 pose）
  - 之后接 `UpperBodySlot`（开火/换弹蒙太奇）→ BBBAimIK（Manny `spine_01→spine_03`，AimSourceBoneName=hand_r）→ Output
- **关键认知（省事）**：C++ 装备链路（`Equip()`→`SetAnimInstanceClass`+`LinkAnimClassLayers`）**完全不动**——作用在 Leader mesh(`GetMesh()`)的 AnimInstance 上，reparent 后 MM ABP 正好挂 `GetMesh()`（session54 确认）。`ALI_WeaponAnim`+`ABP_FirearmBase`(BBBAimIK) 范式继续用。**唯一要改**：让 `SandboxCharacter_CMC_ABP` **实现 `ALI_WeaponAnim` 接口** + 末端加上面的 Layered blend per bone（MM 部分一字不改，只在后面追加叠加）。

**2. 武器动画决策（用户拍板）= 重做成 Manny 骨架版：**
- 现有武器动画（持枪 pose/开火蒙太奇/装备蒙太奇）全建在**旧手臂骨架**（SCF_Rifle_02/SCFP，FEAT-033 Compatible Skeletons 凑的）。身体换 UEFN Manny 后骨骼名/ref pose 全变，旧 pose 套 Manny 错位/不认 → 阶段3-b 须出 Manny 骨架版持枪 pose + 开火蒙太奇。
- **素材来源候选**：项目已有 `Content/Characters/Mannequins/Anims/Rifle/`（UE5 标准 Manny 步枪动画 Fire/Reload/Equip/ADS），骨骼层级与 UEFN Manny 基本一致，阶段3 试 Compatible Skeletons 或轻量 retarget 直接拿来当素材，省从零做。

**3. 阶段2 第3项（接装备）操作清单（下午用户执行，纯编辑器，接装备本身不依赖上半身层——枪挂手骨 socket，手臂暂 MM pose 无握姿属正常，3-b 才解决）：**
- 在 reparent 后的 `SandboxCharacter_CMC` BP（父类 `AFPSMaintenanceWorker`）Class Defaults 填三个继承字段：`InitialEquipmentClasses`(BP_TestGun[+BP_RepairGun]) / `CharacterData`(DA_MaintenanceWorkerAttributes) / `InitGEClass`(GE_CharacterBaseBase_Init 血量初始化)。
- **武器挂载 socket 对齐 Manny 手骨**：打开 `SK_UEFN_Mannequin` 骨架编辑器 → `hand_r` Add Socket，名字与武器 BP 的 `EquipSocketName`(去 BP_TestGun 看,如 Grip_Point) 一致（或改武器 BP 的 socket 名）。先大致对位,精确贴合等 3-b 握姿后微调。
- **⚠️ 最可能的坑**：GASP pawn EventGraph 若 override 了 `Event BeginPlay`/`Event Possessed`,必须调 **Parent**(Add Call to Parent Function),否则 `AFPSCharacterBase::BeginPlay`(spawn 装备)/`PossessedBy`(GAS 初始化)不跑 → 装备不出现/开火无反应先查这里。
- PIE 验证(GASPTest, BP_TheManGameMode/BP_TheManPlayerController)：开火扣血 / 滚轮切武器。扫描是 Infiltrator 专属,当前 MaintenanceWorker 不验。

### 2026-06-28-session57 — 阶段3 起步：上半身武器层叠 MM 的宿主 ABP 结构跑通（编辑器，无 C++ 改动）

按 session56 定的架构，在 `SandboxCharacter_CMC_ABP`（MM 主 ABP，挂 Leader `GetMesh()`）末端叠上半身武器层，**纯编辑器操作，C++ 装备链路一字未动**。用户全程 PIE 验证通过。

**1. 宿主 ABP 实现 `ALI_WeaponAnim` 接口**
- Class Settings → Interfaces → Add `ALI_WeaponAnim` → 编译 → My Blueprint 的 Anim Layers 出现 `WeaponAimOffset` / `WeaponUpperBody` 两个可重写层函数。

**2. AnimGraph 末端实测链路（输出 → 输入）**
```
Output Pose(Root)
  ← Layered blend per bone（Base=MM；Blend Poses 0=WeaponAimOffset 层；Branch Filter: Bone=spine_01, Blend Depth=0）
       ├─ Base Pose      ← Use cached pose 'CachedMMPose'
       └─ Blend Poses 0  ← WeaponAimOffset(Linked Anim Layer 节点)，其 WeaponAimInPose ← Use cached pose 'CachedMMPose'
  ← Save cached pose 'CachedMMPose' ← PoseHistory(PoseSearchHistoryCollector，MM 收尾，X=1280)
```
- **插入点 = `PoseHistory` 之后、`Output Pose` 之前**（session56 "最末端输出前"）。MM 整条管线（PoseHistory→…→MotionMatching）一字不动，只在尾部追加叠加。
- Branch Filter `spine_01` / Depth 0 → spine_01 及所有子骨骼（脊柱/手臂/头）走武器层持枪 pose，pelvis/腿保持 MM。

**3. 两个关键认知（避免下个 agent 重踩）**
- **Pose 输出是单消费**（不像 float 数据线能一对多）：要把 MM 姿势同时喂 Base + 武器层输入，必须 `Save cached pose` 缓存一次 + 多个 `Use cached pose` 取用。
- **必须叠在 `PoseHistory` 之后**：PoseHistory 要记录"纯下半身 locomotion 姿势"喂 MM 搜索，持枪 pose 叠在它前面会污染匹配。且 PoseHistory 输出已是 **Local Space**，正合 Layered blend + Output 所需，无需空间转换。

**4. 持枪 pose 来源（占位 + 兜底）**
- 用户已导入一套 **Manny 重定向 aim 动画**在 `Content/RTG/`：`RTG_W2_Stand_Aim_Idle`（站姿持枪 idle）+ `RTG_W2_Walk/Jog_Aim_*`（各向 aim loop，W2 = 步枪 aim offset 套）。
- 把 `RTG_W2_Stand_Aim_Idle` 放进**宿主 `WeaponAimOffset` 层的默认实现**（默认图 Output 接该 Sequence Player）当兜底——没装备武器/武器还没做 Manny 版时也能看到持枪 pose。装备武器后由武器 ABP 的实现覆盖（走 `Equip()`→`LinkAnimClassLayers`，C++ 不改）。
- **PIE 验证（GASPTest）**：上半身持枪瞄准、下半身 MM 走/跑/停/转，用户确认「效果正常」。

**5. 剩余（阶段3 续 + 阶段2）**
- UpperBodySlot（开火/换弹蒙太奇插槽）+ `WeaponUpperBody` 层。
- BBBAimIK（Manny `spine_01→spine_03`，AimSourceBoneName=hand_r，枪口跟准星俯仰）。
- 接装备（阶段2 第3项，见 session56 清单）+ 做 per-weapon Manny 武器层 ABP（用 `RTG_W2_*` 那套 aim 动画 + AimPitch 驱动 aim offset blendspace）→ 切武器换持枪 pose 真正生效。

### 2026-06-28-session57（续）— 阶段2 第3项：接装备跑通（C++ 两处小改 + 编辑器字段）

接上半身武器层后顺手把阶段2 最后一项「GAS/装备」接上，**阶段2 三项（影子 Follower / 输入合并 / 装备）至此全通**。

**C++ 改动（`EquipmentBase.cpp` 构造函数，只动 .cpp，全量重编通过）：**
1. `EquipSocketName` 默认值 `Hand_R_Socket` → **`Grip_Point`**（对齐武器 BP 实际用的 socket 名；BP 覆盖值本就优先，改默认只为"新武器不填也对"）。
2. 武器 mesh `CastShadow`/`bCastDynamicShadow` `false` → **`true`**（FEAT-042：地上影子手里有枪，配合 FEAT-038 ShadowBodyMesh）。

**编辑器字段（用户填）：**
- `SandboxCharacter_CMC`：`InitialEquipmentClasses`=BP_TestGun(+BP_RepairGun) / `CharacterData`=DA_MaintenanceWorkerAttributes / `InitGEClass`=GE_CharacterBaseBase_Init。
- `SK_UEFN_Mannequin` 的 `hand_r` 加 socket **`Grip_Point`**。
- 武器 BP（BP_TestGun/RepairGun）：`EquipmentAnimClass`=**None**、`EquipmentAnimLayerClass`=**None**（见下"关键坑"）。

**⚠️ 关键坑（已写进 arch/09）：武器动画两字段会搞死 MM**
- `EquipmentAnimClass`（`SetAnimInstanceClass` 整体替换）若在武器上设了 → 装备瞬间把 `SandboxCharacter_CMC_ABP`(MM) 顶掉 → **MM 废**。**MM 角色上必须 None。**
- `EquipmentAnimLayerClass`（`LinkAnimClassLayers`）要 Manny 同骨架 + 实现 `ALI_WeaponAnim`；旧 `ABP_FirearmBase`（旧骨架）链不上。接装备阶段留 None，持枪 pose 由宿主 `WeaponAimOffset` 默认层（RTG_W2_Stand_Aim_Idle）兜底；阶段 3-b 出 Manny 武器层 ABP 再填。

**PIE 验证（GASPTest）用户确认全正常**：枪在手(Grip_Point)、左键开火、滚轮切枪、上半身持枪 pose、**地上影子手里有枪**。旧蒙太奇骨架警告无害（3-b 重做）。
- 遗留（非阻塞）：武器投影打开后 FP 近距离自阴影若难看，正解 FEAT-040 单独 owner-no-see 第三人称武器 mesh 投影；先这样。

### 2026-06-28-session58 — 阶段4 起步：占位 Manny 物理拆三件套（消 FP 穿模，为调相机/上半身做准备）

> 本会话先做了一项**顺序调整决策**，再带新手用户在 Blender 把占位 Manny mesh 物理拆成三件套并导回装配。**纯编辑器 + Blender 操作，C++ 一字未改。**

**1. 顺序调整决策（用户拍板）：先拆三件套 + 调相机，再做 BBBAimIK。**
- 起因：当前 Leader（`GetMesh()`/CharacterMesh0）挂的是 **GASP 整只 Manny 全身 mesh** + `OnlyOwnerSee`，FP 相机（眼高 77 挂胶囊体）卡进 Manny 头/脖子 → **穿模**，看不清手臂、没法对着手臂定相机。
- 用户要"根据手臂实际位置调相机" → 必须先有干净手臂。故把交接原计划"先 BBBAimIK"改为**先拆三件套占位 + 调相机**。
- **关键澄清（对正确性零影响）**：相机挂**胶囊体根**、固定眼高 `(0,0,77)`、`bUsePawnControlRotation`（C++ `FPSCharacterBase.cpp` 第63-66行），**不读 mesh、不被动画驱动**——拆不拆 mesh 相机都在原地。拆 mesh 也**不动 hand_r/枪口位置**（同骨架同 pose），所以 BBBAimIK 结果拆前拆后一致，先后顺序不返工。穿模纯是"全身 mesh 挡视线"的视觉问题，与相机位置/瞄准逻辑无关。

**2. BBBAimIK 放置定案（厘清交接的分歧）：放 per-weapon Manny 武器层 ABP，不放宿主 MM ABP。**
- session56/57 交接写"BBBAimIK 放宿主 `SandboxCharacter_CMC_ABP`"，但宿主父类是 GASP 类，拿不到 `AimTargetComponentSpace` 等变量，得在蓝图重算。
- 核 C++ 确认（`FirearmAnimInstance.cpp` + `Firearm.cpp`）：`UFirearmAnimInstance::NativeUpdateAnimation` **完全通用**——从 `GetOwningActor()`→controller→玩家视点前向射线 10000 算准星点，转 component space 喂 `AimTargetComponentSpace`，`bIsAiming` 恒 true，**不 cast 具体角色、不挑骨架**。`AFirearm::Equip()` 在 `GetArmsMesh()`(=GetMesh()=MM 宿主) 上 `LinkAnimClassLayers(EquipmentAnimLayerClass)` + `GetLinkedAnimLayerInstanceByClass` 取实例 `Cast<UFirearmAnimInstance>` 成功就写 `MuzzleLocalTransform`→`AimSourceLocalTransform`。
- → **最省 = 沿用 arch/12 范式**：per-weapon Manny 武器层 ABP 父类 `UFirearmAnimInstance` + 实现 `ALI_WeaponAnim`，`WeaponAimOffset` 内放 BBBAimIK(spine_01→03, hand_r, 用继承的 4 个变量)，**零 C++ 改动**复用现成喂值 + 装备链路。武器 BP `EquipmentAnimLayerClass` 指它（`EquipmentAnimClass` 永远 None 否则顶掉 MM）。宿主 ABP 的 `WeaponAimOffset` linked-layer 槽（session57 已搭）正是它的插入点。

**3. 物理拆 mesh 实操（Blender 新手，照 session46 流程，拆对象=占位 Manny）：**
- UE 右键 Leader 当前 Manny 全身 SkeletalMesh → Export FBX → Blender 导入。
- Edit Mode + `Alt+Z` X-Ray + Numpad1 正视图 → `B` 框选两臂(肩→指尖)`P` Selection 分出 **Arms**；`Alt+A` → 框选两腿(胯→脚，用户多选了整个胯部，**反而更好**：低头看腿+胯无缺口；剩余躯干头残块丢弃，影子用原整块资产不受影响)`P` 分出 **Legs**。
- 各导出 FBX：选 mesh+Armature → **Limit to: Selected Objects ✅** + **Add Leaf Bones ❌**。
- 导回 UE：**Skeleton 指定现有 `SK_UEFN_Mannequin`**（不留空）。
- BP 装配：Leader `Mesh`=Arms(保持 `SandboxCharacter_CMC_ABP`，MM 照跑) / `ShadowBodyMesh`=原整块 Manny / `LegsMesh`=Legs；`ArmsHiddenSections`/`LegsHiddenSections` 留空(物理拆不用材质段)。
- 用户确认"看起来挺不错"。

**⏭ 本会话进行中（未完）：**
- 调 `HeadCamera` Location（BP 覆盖 C++ 默认 `(0,0,77)`）对手臂目测定眼高/前后。若手臂被 MM 全身动画驱动显偏低、调相机压不下 → 备选杠杆：给 Leader 手臂组件单独加相对偏移(只动手臂不动影子，独立组件)。定稿值若要进代码默认再改 `FPSCharacterBase.cpp`。
- 相机定稿后回 **BBBAimIK（3-b 武器层 ABP）**：建 `ABP_FirearmBase_Manny`(父类 `UFirearmAnimInstance` + 实现 `ALI_WeaponAnim`)，`WeaponAimOffset`=RTG_W2_Stand_Aim_Idle→LocalToComponent→BBBAimIK(spine_01/02/03=0.2/0.4/0.6, hand_r, AimAxis(1,0,0), 4 变量继承)→ComponentToLocal→Result；`WeaponUpperBody` 直通。BP_TestGun `EquipmentAnimLayerClass`=它 / `EquipmentAnimClass`=None / `MuzzleLocalTransform` 填枪口相对 hand_r。

### 2026-06-28-session58（续2）— FP 手臂方案大转向：独立手臂 mesh 转组件跟视角（C++ 改完，待编译+BP）

> 本会话后半程围绕"FP 手臂怎么跟视角"反复讨论，最终**推翻"单骨架共享 + 上半身在 MM 宿主上做"**，改为**独立 FP 手臂 mesh**。C++ 已改完，**待全量重编 + BP 装配**。

**设计演进（关键认知链，避免重走）：**
1. 先试"加 ArmsPivot 支点、Tick 里转 `GetMesh()` 组件让手臂跟俯仰"——**PIE 完全无效**。根因：`GetMesh()` 是 MM 宿主，GASP 的 **OffsetRootBone/Steering 每帧（在我 Tick 之后）重写它的朝向** → 组件级旋转被覆盖。**结论：MM 宿主那张 mesh，组件旋转这条路走不通。**
2. 讨论过"在姿势里转 spine"（路1，MM 安全）——会让**影子也跟着俯仰**。用户最终明确**要影子端枪保持平的**（不跟俯仰）。
3. 关键事实：**Copy Pose From Mesh 只搬骨骼姿势、不搬组件旋转**。所以"手臂转组件跟视角"与"影子同步手臂姿势"天然兼容——影子 Copy 过去只拿到持枪 pose（平的），拿不到那个组件俯仰 → **正好是用户要的**。
4. **定案：FP 手臂 = 独立 mesh（不归 MM），转它的组件（绕 ArmsPivot）跟视角生效；MM 只管身体/影子/腿。** 用户选 **B（用刚拆的 Manny 手臂）** + 手臂自己一套武器 ABP。

**C++ 改动（`FPSCharacterBase.h/.cpp`，已改完，动了头文件→必须全量重编）：**
- 新增 `USceneComponent* ArmsPivot`（挂 capsule，默认 `(0,0,77)`，BP 拖到肩/手臂中心）+ `USkeletalMeshComponent* ArmsViewMesh`（挂 ArmsPivot 下，OnlyOwnerSee/无碰撞/不投影/AlwaysTickPose）。
- `GetMesh()`：`SetOnlyOwnerSee`→**`SetOwnerNoSee`**（退居 MM 驱动，对自己隐藏，只喂影子/腿 Follower）。
- **`GetArmsMesh()` 改返回 `ArmsViewMesh`**（原返回 GetMesh()）→ 武器层链接（`Firearm.cpp`）/开火蒙太奇（`GA_Shoot.cpp`）/装备挂载（`EquipmentBase.cpp` + `AttachTargetMesh`）全自动跟到 FP 手臂。`BeginPlay` 里 `AttachTargetMesh=ArmsViewMesh`。
- Tick 重写：**不再碰 `GetMesh()` 旋转**（交给 MM）；`BaseArmsRotation`+摇摆作用到 `ArmsViewMesh`；俯仰平滑插值后转 **`ArmsPivot`** 相对旋转（ArmsViewMesh 是其子级 → 绕支点转天然成立，无需手算位置）。
- 新增可调参数（`ArmsAiming` 分类）：`bArmsPitchFollow`(true) / `ArmsPitchFollowAmount`(0.7，方向反取负) / `ArmsPitchInterpSpeed`(12)。
- 首帧显隐 + `RevealArmsAndWeapon` 扩展到 ArmsViewMesh。删了上一版没用上的 `ArmsBaseRelLocation`（手算支点方案作废）。

**⏭ 待用户做（回来直接接）：**
1. **全量重编**（动了头文件）。
2. **BP 装配 `SandboxCharacter_CMC`**：① `ArmsViewMesh` 赋 Manny 手臂 mesh + 一个简单 FP 手臂 ABP（骨架 SK_UEFN_Mannequin，AnimGraph `RTG_W2_Stand_Aim_Idle`→Output 即可持枪）+ 相对 Location 调（先 Z≈-165 让脚回地面，PIE 微调；旋转由 C++ 管不用调）。② `ArmsPivot` 拖到肩/手臂中心。③ 武器 BP `EquipSocketName=Grip_Point`、两 anim 字段先 None。
3. **PIE**：手臂+枪跟视角俯仰（平滑）、影子端枪保持平的。方向反了 `ArmsPitchFollowAmount` 取负。
4. **follow-up（不阻塞）**：影子持枪 mesh（枪现 OnlyOwnerSee 给 FP 手臂，影子无枪 mesh，手型还在）= FEAT-040 单独 TP 武器；Manny 版开火蒙太奇（FP 手臂 ABP 加 DefaultSlot 才播）。
5. 用户待定：是否保留 LegsMesh（"低头看腿"要就留、不要可删省一张；删不删都不影响手臂方案）。

### 2026-06-30-session59 — 临时禁用 FP 手臂俯仰滞后和 sway（C++ 编译通过）

- 用户确认 FP 手臂效果已好，要求先不要加滞后。
- `FPSCharacterBase.cpp`：`ArmsPivot` 俯仰跟随逻辑保留 `TargetPitch = NormalizeAxis(ControlRotation.Pitch) * ArmsPitchFollowAmount`，但注释掉 `FMath::FInterpTo(CurrentArmsPitch, TargetPitch, DeltaTime, ArmsPitchInterpSpeed)`，改为 `CurrentArmsPitch = TargetPitch`。
- 同会话补改：用户指出左右看仍有 sway 滞后。把 `SwayTarget` + `CurrentSway` 三条 `FInterpTo` 保留注释，当前每帧 `CurrentSway = FRotator::ZeroRotator`。
- 结果：`ArmsViewMesh`/枪即时跟随视角俯仰，且无左右 sway 滞后；`ArmsPitchInterpSpeed` / `SwayInterpSpeedX/Y` 参数暂时不生效，注释保留，后续要恢复只需切回对应行。
- C++ 验证：`Build.bat TheManTestEditor Win64 Development ... -WaitMutex -FromMsBuild` 通过两次（关俯仰滞后后一次、关 sway 后一次）。
- 收尾排查：用户截图怀疑 `AC_*` 组件位置导致 mesh 异常。确认 `AC_FoleyEvents` / `AC_SmartObjectAnimation` / `AC_PreCMCTick` / `AC_TraversalLogic` 都是 ActorComponent，无 Transform，不会因组件面板排序影响 mesh 挂载。真正原因是 `Mesh(CharacterMesh0)` 的 Skeletal Mesh / Anim Class 被清空；用户恢复它作为 MM 宿主（Anim Class=`SandboxCharacter_CMC_ABP`）后，其他 mesh 恢复正常。

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| MM 插件启用无报错 | 2026-06-27 | ✅ | 6 个插件启用，需重启生效 |
| Migrate SandboxCharacter_CMC 进项目 | 2026-06-27 | ✅ | 带进整条演示依赖链（≈2.5G 膨胀），阶段2 清理 |
| 独立关卡 Manny 跑通 MM | 2026-06-27 | ✅ | GASPTest 关卡 GM_Sandbox/PC_Sandbox，用户确认"手感完美" |
| 接进 AFPSCharacterBase（C1 reparent + FP 相机 + 组件共存 + MM 存活） | 2026-06-27 | ✅ | session54 GASPTest PIE 通过；穿模/手臂隔离留阶段4 |
| 三件套 Shadow Follower（影子跟 MM） | 2026-06-27 | ✅ | session55：ShadowBodyMesh 赋 Manny + SetLeaderPoseComponent，删 BP_VisualOverrideManager 换皮系统后单个完整人形影子；Legs 留阶段4 |
| 接进 AFPSCharacterBase（输入合并：移动/视角/跳/冲刺） | 2026-06-27 | ✅ | session55：GASP IA 并进 IMC_Default 单一 context，MM 移动正常；战斗输入待武器 |
| GAS/装备接上（武器装配 + 开火/切武器） | 2026-06-28 | ✅ | session57：EquipSocketName 默认改 Grip_Point + 武器投影开（C++）；三字段填 + Manny Grip_Point socket；两 anim 字段留 None（EquipmentAnimClass 会顶掉 MM）；PIE 枪在手/开火/切枪/影子持枪正常。扫描是 Infiltrator 专属未验 |
| 上半身武器层叠加（宿主 ABP 结构） | 2026-06-28 | ✅ | session57：ALI_WeaponAnim 接口 + Layered blend per bone(spine_01) 叠 MM，WeaponAimOffset 默认层放 RTG_W2_Stand_Aim_Idle 兜底；上半身持枪+下半身 MM，PIE 通过。剩 UpperBodySlot/BBBAimIK/per-weapon 武器 ABP |
| 占位 Manny 物理拆三件套（消 FP 穿模） | 2026-06-28 | 🔄 | session58：Blender 拆 Manny 全身→Arms/Legs，导回套 SK_UEFN_Mannequin，BP 三件套装配（Leader=Arms/Shadow=原整块/Legs=Legs），用户"看起来挺不错"。调相机 + BBBAimIK 进行中 |
| FP 手臂俯仰滞后和 sway 禁用 | 2026-06-30 | ✅ | session59：`ArmsPivot` 俯仰直接跟随 `TargetPitch`；`ArmsViewMesh` sway 每帧清零；相关 `FInterpTo` 保留注释；Development Editor / Win64 编译通过 |
| CharacterMesh0 MM 宿主配置恢复 | 2026-06-30 | ✅ | session59：确认 `AC_*` 非挂载问题；`Mesh(CharacterMesh0)` 需保留 Skeletal Mesh + `SandboxCharacter_CMC_ABP`，用户恢复后其他 mesh 正常 |
| FP viewmodel 改为相机子级 | 2026-07-02 | ✅ | session62：`HeadCamera -> ViewmodelRoot -> ArmsViewMesh`；旧 `ArmsPivot` 手动 pitch 旋转替换；Development Editor / Win64 编译通过，PIE 待验证 |

### 2026-07-01-session60 - Camera attached to FP arms head socket test
- C++ change: in Source/TheManTest/Private/Characters/FPSCharacterBase/FPSCharacterBase.cpp, HeadCamera no longer attaches to RootComponent; it now attaches to ArmsViewMesh socket/bone head.
- Camera relative location and rotation are zeroed after attachment. bUsePawnControlRotation remains true so view rotation still follows controller input.
- Verification: attempted Build.bat TheManTestEditor Win64 Development ... -WaitMutex -FromMsBuild. First run hit AppData log write sandboxing; escalated run reached UBT but stopped because UE Live Coding is active. Need close editor/game or press Ctrl+Alt+F11, then rebuild and PIE test.

### 2026-07-02-session61 - Revert camera head-socket test and record next FPS viewmodel direction
- Decision: do not keep the gameplay camera mounted under the animated FP arms `head` socket. Professional FPS structure keeps the camera on a stable root/camera component, and lets viewmodel arms adapt to the camera.
- C++ change already made: `HeadCamera` attaches to `RootComponent` again, relative location `(0,0,77)`, zero relative rotation, `bUsePawnControlRotation=true`. `ArmsViewMesh` remains under `ArmsPivot` and still owns first-person arms/weapon rendering and equipment attachment.
- Verification: `Build.bat TheManTestEditor Win64 Development "D:\Unreal Projects\TheManTest\TheManTest.uproject" -WaitMutex -FromMsBuild` succeeded after rerunning with permission to write UBT AppData logs.
- Current issue to solve after work: rotating `ArmsPivot` around shoulder/arms center causes screen-space drift at different pitch angles. Do not rotate the Character capsule for pitch; that would break CharacterMovement/collision/GASP/body/shadow assumptions.
- Recommended next implementation: put the FP viewmodel pivot at the camera origin, not the shoulder. Either keep current components and set `ArmsPivot` relative location equal to `HeadCamera` `(0,0,77)`, then position `ArmsViewMesh` with relative offset; or add a stable `FPSViewRoot`/`ViewmodelRoot` under `RootComponent` at camera origin and attach both camera-relative arms and optional camera effects there. The camera remains authoritative; arms follow it.

### 2026-07-02-session62 - Move FP viewmodel under camera with ViewmodelRoot
- User discussion resolved the architecture: use a plain `SceneComponent` viewmodel layer under the camera, not a SpringArm. Future movement lag / sway / ADS / bob should be procedural offsets on `ViewmodelRoot` or `ArmsViewMesh`; SpringArm camera lag/collision is the wrong tool for first-person viewmodels.
- C++ change: `AFPSCharacterBase` now creates `ViewmodelRoot` attached to `HeadCamera`, and attaches `ArmsViewMesh` to `ViewmodelRoot`. `HeadCamera` remains stable under `RootComponent` at `(0,0,77)` with `bUsePawnControlRotation=true`. Added `EnsureViewmodelAttachment()` in construction/BeginPlay to keep inherited BP instances attached correctly if old native attachment state is cached.
- Removed/replaced old `ArmsPivot` component and the per-frame manual pitch rotation around shoulder/arms center. The first-person hands now inherit camera rotation directly, so the viewmodel pivot is the camera origin and should avoid pitch-dependent screen-space drift.
- Preserved existing separation: `GetArmsMesh()` still returns `ArmsViewMesh`, so equipment attachment, FP weapon rendering, and fire montage routing still target the independent FP hands. `GetMesh()` remains the GASP/MM host for body/shadow/legs.
- Verification: Development Editor / Win64 build succeeded. First run hit UBT AppData log permissions; escalated run completed UHT, compile, link, and metadata successfully. After the attachment safeguard, a second Development Editor / Win64 build also succeeded. User confirmed the latest screenshot/component tree now shows the intended hierarchy.
- Next editor step: because the C++ component changed from `ArmsPivot` to `ViewmodelRoot`, open the active character BP and check the component tree plus `ArmsViewMesh` relative transform under `ViewmodelRoot`. Then PIE verify pitch up/down screen stability, weapon position, firing, weapon switching, body/shadow/legs.

### 2026-07-03-session63 - Direction change: abandon Motion Matching and delete GASP assets/plugins
- User decision: stop using Motion Matching / GASP for the player character. New direction is project-native `AFPSCharacterBase` + ordinary AnimBP locomotion state machine + independent FP viewmodel.
- Kept useful C++ architecture from this feature: `HeadCamera -> ViewmodelRoot -> ArmsViewMesh`, with `GetArmsMesh()` still returning `ArmsViewMesh` so equipment attachment, FP weapon rendering, and fire montage routing remain on the independent first-person hands.
- `.uproject` plugin cleanup: removed PoseSearch, AnimationLocomotionLibrary, MotionWarping, AnimationWarping, Chooser, CurveExpression, SmartObjects, GameplayInteractions, Mover, NetworkPrediction, Locomotor, DrawDebugLibrary, RigLogic, LiveLink, LiveLinkControlRig, and HairStrands. Kept ModelingToolsEditorMode and GameplayAbilities.
- Deleted 21 GASP/MM asset targets after resolving and validating paths inside the workspace:
  - `Content/Blueprints/BPI_SandboxCharacter_ABP.uasset`
  - `Content/Blueprints/BPI_SandboxCharacter_Pawn.uasset`
  - `Content/Blueprints/SandboxCharacter_CMC_ABP.uasset`
  - `Content/Blueprints/SandboxCharacter_CMC_BACKUP.uasset`
  - `Content/Blueprints/SandboxCharacter_CMC_BACKUP1.uasset`
  - `Content/Blueprints/SandboxCharacter_Mover.uasset`
  - `Content/Blueprints/SandboxCharacter_Mover_ABP.uasset`
  - `Content/Blueprints/MovementModes`
  - `Content/Blueprints/SmartObjects`
  - `Content/Blueprints/RetargetedCharacters`
  - `Content/Blueprints/Cameras`
  - `Content/Blueprints/Data`
  - `Content/Blueprints/AnimModifiers`
  - `Content/Blueprints/AnimNotifies`
  - `Content/Blueprints/ControlRigs`
  - `Content/Characters/UEFN_Mannequin`
  - `Content/Characters/Echo`
  - `Content/Characters/Paragon`
  - `Content/MetaHumans`
  - `Content/Input`
  - `Content/_SystemSupport`
- File-level verification: deleted targets no longer exist; `rg --files Content | rg "(SandboxCharacter|UEFN_Mannequin|MotionMatchingData|MetaHumans|Characters/Echo|Characters/Paragon|RetargetedCharacters|MovementModes|SmartObjects)"` returned no matches.
- Build verification: `Build.bat TheManTestEditor Win64 Development "D:\Unreal Projects\TheManTest\TheManTest.uproject" -WaitMutex -FromMsBuild` succeeded after rerunning with permission for UBT AppData logs.
- Editor follow-up required: open UE, Fix Up Redirectors, then check GameMode / `DT_CharacterRoster` / active character BP / AnimClass references. Replace any old `SandboxCharacter_CMC` / `SandboxCharacter_CMC_ABP` references with project-owned player BP and ordinary locomotion ABP before PIE.

### 2026-07-03-session63 addendum - Remove stop-animation C++ and keep FPS anim instance thin
- User decided not to implement stop animations. Player locomotion should follow the UE template style: `Idle <-> Walk/Run BlendSpace` driven by `Speed` / `Direction`, with jump driven by `bIsFalling` / `Velocity_Z`.
- Clarified class roles:
  - `UBaseLocomotionAnimInstance` is the shared base for player and enemy animation variables (`Speed`, `Direction`, `Velocity_Z`, `bIsFalling`, `AimPitch`).
  - `UFPSCharacterAnimInstance` remains as a thin player-specific subclass for future player-only variables. It currently adds only `AccelDirection` and `bHasAcceleration`.
- C++ cleanup completed:
  - Removed stop-related fields and logic from `UFPSCharacterAnimInstance`: `bShouldMove`, `bShouldStop`, `bIsJogging`, `bLeftFootUp`, `LockedStopDirection`, `StopDirectionIndex`, `bStopFromRun`, `StopAnimIndex`, stop config fields, foot-phase helpers, direction-to-stop indexing, and TEMP `GEngine` debug output.
  - Simplified `FPSCharacterAnimInstance.cpp` to only call the base update and compute acceleration direction/presence.
  - Removed `AFPSCharacterBase::OnMoveReleased`, Move Completed/Canceled bindings, `bHasMoveInput`, and cached braking/friction fields.
  - Removed the temporary `PythonScriptPlugin` entry from `.uproject`.
- Verification done before handoff:
  - Source search for stop-related symbols in `Source` and `.uproject` found no active symbols, only a comment noting that `StopAnimIndex` is no longer used.
  - `.uproject` JSON parses successfully.
  - Command-line build could not complete because UE Live Coding is active: UBT message was "Unable to build while Live Coding is active." User will compile manually.
- Editor follow-up required:
  - `ABP_BodyLocomotion` is project-owned and should not be deleted just because Motion Matching was abandoned.
  - Open `ABP_BodyLocomotion` and remove any leftover `PoseSearchHistoryCollector` / Motion Matching nodes and all Stop state / deleted variable references.
  - Rewire to ordinary `Idle <-> WalkRun` locomotion before compiling blueprints.

### 2026-07-03-session64 - Add non-root-motion 45-degree turn-in-place trigger variables
- User wants fast player body turn animations when idle rotation exceeds a threshold, starting with left/right 45-degree turns and ignoring broader turn system polish for now.
- C++ change: `UFPSCharacterAnimInstance` now exposes `bIsTurningInPlace`, `TurnInPlaceAngle`, and `TurnInPlaceIndex` (`0=left45`, `1=right45`), plus tunables `TurnInPlaceThreshold` (45), `TurnInPlaceMinSpeed` (3), and `TurnInPlaceCooldown` (0.25).
- Runtime logic: while speed is <= threshold and the character is not falling, the anim instance accumulates actor yaw delta. When accumulated yaw exceeds the threshold, it emits a one-frame turn request and resets the accumulator. Moving or falling clears the accumulator.
- Design note: first version uses non-root-motion turn animations because `AFPSCharacterBase` still uses `bUseControllerRotationYaw=true`; root-motion turn assets are reserved for a later body-orientation system if needed.
- Verification: `Build.bat TheManTestEditor Win64 Development ... -WaitMutex -FromMsBuild` succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session64 addendum - Decouple lower-body visual yaw from Pawn yaw
- User identified the real issue: the lower body should not follow Pawn rotation immediately. `AFPSCharacterBase` still keeps Pawn/camera yaw live for controls, but lower body/shadow now use an independent visual yaw on `BodyRoot`.
- C++ change: added `BodyVisualYaw`, `bBodyTurningInPlace`, `BodyTurnInPlaceAngle`, and `BodyTurnInPlaceIndex` to `AFPSCharacterBase`, with tuning properties `BodyTurnInPlaceThreshold` (5), `BodyTurnInPlaceInterpSpeed` (720), and `BodyTurnInPlaceMoveSpeed` (3).
- Runtime behavior: moving or falling snaps `BodyVisualYaw` to PawnYaw. While idle, `BodyVisualYaw` stays behind until the yaw delta exceeds the threshold, then it fast-turns toward PawnYaw and exposes left/right turn state for the ABP.
- `UFPSCharacterAnimInstance` no longer accumulates Pawn yaw itself. It reads the body turn state from `AFPSCharacterBase`, so `bIsTurningInPlace` now describes lower-body visual turning rather than controller yaw delta.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session64 addendum 2 - Lock turn target so turn animation cannot interrupt itself
- User refined the requirement: once a turn starts, it must not be interrupted or redirected by continuing camera rotation. The next decision should compare the post-turn lower-body forward vector against camera/Pawn forward, not accumulate camera rotation by itself.
- C++ change: added `BodyTurnInPlaceStepAngle` (default 45) and `BodyTurnTargetYaw`. On trigger, the body locks target yaw to `BodyVisualYaw +/- StepAngle` based on left/right direction. While turning, `BodyVisualYaw` uses `FixedTurn` toward this locked target instead of chasing live PawnYaw.
- Result: camera/Pawn can keep rotating freely for aiming, but the current lower-body turn finishes its locked 45-degree visual step first. After that, the new leg forward direction is compared against current PawnYaw for any next turn.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session64 addendum 3 - Add turn request window and animation lock
- User observed that fast continuous camera rotation still looked like the turn animation was interrupting itself.
- C++ change: split body turn state into visual progression and a short ABP request. `IsBodyTurningInPlace()` now returns `bBodyTurnRequestActive` instead of the whole visual turn duration.
- Added tuning properties:
  - `BodyTurnInPlaceRequestDuration` default `0.15`: short window for ABP to enter Turn_L45/Turn_R45.
  - `BodyTurnInPlaceAnimLockTime` default `0.45`: prevents retriggering while the turn animation should still be playing.
- `BodyVisualYaw` still turns toward locked `BodyTurnTargetYaw`; ABP only gets a short trigger pulse. This should reduce re-entry loops when the Turn state returns to Idle.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.
- Pending next session: user will inspect CS:GO-style lower body yaw behavior and continue tuning. Check ABP first: Turn state sequence players must be non-looping; Turn -> Idle should be based on `Relevant Anim Time Remaining < 0.1`, not a looping automatic rule. Then tune `BodyTurnInPlaceAnimLockTime` to match actual animation length.

### 2026-07-03-session65 - Sync lower-body visual yaw speed to turn animation time
- User observed a new issue: when the turn animation starts, the lower body visually snaps/teleports first and then plays the turn animation in place.
- Root cause in C++: `BodyTurnInPlaceInterpSpeed` was `720 deg/s`, so a 45-degree `BodyVisualYaw` turn completed in about `0.06s`, while `BodyTurnInPlaceAnimLockTime` was `0.45s`. The component yaw reached the target long before the ABP turn state had visually played.
- C++ change: added `BodyTurnVisualTimeRemaining`. On turn trigger, it is initialized from `BodyTurnInPlaceAnimLockTime`; while turning, `BodyVisualYaw` now uses `RemainingYawDelta / BodyTurnVisualTimeRemaining` to reach `BodyTurnTargetYaw` over the same lock/animation window instead of using the fixed 720 deg/s speed.
- Expected result: the lower-body component yaw progresses with the turn animation instead of snapping to the target first. If the visual still pops, the next check is ABP transition blending: Turn states should be non-looping, entered from Idle with a short blend/inertialization, and returned by `Relevant Anim Time Remaining < 0.1`.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session65 addendum - Revert play-rate idea and use repeated fixed 45-degree turns
- User clarified the desired behavior again after testing: do not use dynamic play rate. When the player flicks the camera far behind the lower body, the body should play multiple fixed turn animations in sequence instead of speeding one animation and then visually sliding.
- C++ change: removed the temporary `BodyTurnInPlacePlayRate` / `TurnInPlacePlayRate` output and related min/max play-rate tunables.
- Turn behavior is now fixed-step again: each trigger locks one `BodyTurnInPlaceStepAngle` step (default 45 degrees), takes `BodyTurnInPlaceAnimLockTime` seconds, and exposes a short `bIsTurningInPlace` pulse to ABP. When the step finishes, if the remaining yaw gap is still above `BodyTurnInPlaceThreshold`, the next Tick can trigger another 45-degree turn animation.
- Tuning note: if the fixed step feels too slow, reduce `BodyTurnInPlaceAnimLockTime` to match the real 45-degree animation length or the desired speed. Do not wire any Play Rate in the ABP for this version.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session65 addendum 2 - Delay visual yaw until Turn state has entered
- User observed that the gap between repeated turn animations still rotates the lower body.
- Cause: C++ starts the next `BodyVisualYaw` step as soon as it sends the ABP turn request, but the ABP may still be blending out/in between Turn states.
- C++ change: added `BodyTurnInPlaceVisualStartDelay` (default `0.06`) and internal `BodyTurnVisualStartDelayRemaining`. Each turn step now sends the short ABP request immediately, waits this small delay before rotating `BodyRoot`, then completes the 45-degree visual yaw over the remaining part of `BodyTurnInPlaceAnimLockTime`.
- Tuning note: if the body still rotates during the transition gap, increase `BodyTurnInPlaceVisualStartDelay` slightly (for example `0.08` or `0.1`). If the visual turn starts too late, reduce it toward `0.03`.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session65 addendum 3 - Fixed blueprint turn play rate synchronized with C++ yaw
- User wants a blueprint-adjustable animation asset play rate that stays unified with the C++ visual yaw speed.
- C++ change: added fixed `BodyTurnInPlacePlayRate` on `AFPSCharacterBase` (default `1.0`, clamp `0.1..3.0`) and exposed it to `UFPSCharacterAnimInstance` as `TurnInPlacePlayRate`.
- Runtime timing now treats `BodyTurnInPlaceAnimLockTime` as the 1x duration. Actual per-step timing is `BodyTurnInPlaceAnimLockTime / BodyTurnInPlacePlayRate`; `BodyTurnInPlaceVisualStartDelay` is also divided by the same play rate. This keeps BodyRoot yaw and the animation asset speed aligned.
- Editor follow-up: in `ABP_BodyLocomotion`, expose/connect the `Play Rate` pin on the `Turn_L45` and `Turn_R45` Sequence Players to `TurnInPlacePlayRate`. Then tune only `BodyTurnInPlacePlayRate` in the character BP for overall turn speed.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-03-session66 - Drive lower-body turn yaw from baked `TurnRootYaw` animation curve
- User migrated the two non-IP 45-degree turn assets into this project under `Content/RTG` and confirmed the left asset's `TurnRootYaw` curve runs `0 -> -45`.
- C++ change: `AFPSCharacterBase` now exposes `IsBodyTurnVisualInProgress()` and `ApplyBodyTurnRootYawDelta(float)`. `UFPSCharacterAnimInstance` adds `TurnRootYawCurveName` (default `TurnRootYaw`) plus internal previous-value tracking. During a turn step, it reads `GetCurveValue(TurnRootYawCurveName)`, applies the per-frame curve delta to `BodyVisualYaw`, and filters by turn direction so left turns only consume negative curve deltas and right turns only consume positive deltas.
- Removed the previous manual visual yaw progression concept from runtime: no `BodyTurnInPlaceVisualStartDelay`, no C++ fixed-speed/remaining-time visual turn estimate. The animation asset's baked curve now defines the visual yaw timing.
- `BodyTurnInPlacePlayRate` remains useful: it still scales the lock/retrigger timing as `BodyTurnInPlaceAnimLockTime / BodyTurnInPlacePlayRate`, and `TurnInPlacePlayRate` should still be connected to the `Turn_L45` / `Turn_R45` Sequence Player Play Rate pins so the curve playback and C++ lock agree.
- Editor follow-up: ensure `ABP_BodyLocomotion` Turn states use `Content/RTG/RTG_W2_Stand_Aim_L_45` and `Content/RTG/RTG_W2_Stand_Aim_R_45`, both carrying the `TurnRootYaw` curve.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session67 - Expose C++ pose-fix rotator for Turn state root-yaw cancellation
- User wanted the pose-fix curve math moved out of Blueprint. C++ now exposes `TurnRootYawPoseFixYaw` and `TurnRootYawPoseFixRotation` on `UFPSCharacterAnimInstance`.
- Runtime behavior: `TurnRootYawPoseFixYaw = -GetCurveValue(TurnRootYawCurveName)` and `TurnRootYawPoseFixRotation = FRotator(0, TurnRootYawPoseFixYaw, 0)`. These values reset to zero when there is no valid player character.
- Initial editor follow-up was to wire `Transform Modify Bone(root)` in the Turn states, but user testing showed that state-local placement is not sufficient during Turn->Idle blends. Use the handoff note below instead: place the root-yaw cancellation once outside `LocomotionSM`.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session67 handoff note - Move pose-fix outside LocomotionSM
- User tested the Turn-state-local pose fix and still saw a visible recovery toward Idle before the C++/BodyRoot yaw pulled the pose back.
- Diagnosis: putting `Transform Modify Bone(root)` inside Turn_L45 / Turn_R45 only affects each Turn state's local output. During the Turn->Idle transition, the state machine blends Turn weight down and Idle weight up, so the root-yaw cancellation is also blended out.
- Next editor fix: move the root-yaw cancellation node outside `LocomotionSM` and apply it once to the state machine's final blended pose:
  `LocomotionSM -> Local To Component -> Transform Modify Bone(root, Rotation=TurnRootYawPoseFixRotation) -> Component To Local -> downstream Slot/Layer/Output`.
- Keep the Turn states themselves simple: each state should only play its left/right Sequence Player, with Sequence Player Play Rate connected to `TurnInPlacePlayRate`.
- If a small recovery remains after moving the node, reduce Turn->Idle transition blend time; try `0.05` first, then `0.0` if needed.
- User also requested next session cleanup: remove now-unused C++ variables/logic from previous turn-in-place iterations, but keep the play-rate path (`BodyTurnInPlacePlayRate`, `TurnInPlacePlayRate`, and lock timing scaled by play rate).

### 2026-07-04-session68 - Revert root-bone pose fix and keep curve-driven BodyRoot turn
- User pointed out the earlier mesh/component-rotation schemes did not show the Turn->Idle recovery because the rotation lived outside the AnimGraph state blend. Decision: do not use AnimGraph root-bone correction for this problem.
- C++ cleanup: removed `UFPSCharacterAnimInstance::TurnRootYawPoseFixYaw` and `TurnRootYawPoseFixRotation`. Removed unused `AFPSCharacterBase::BodyTurnInPlaceInterpSpeed`.
- Current intended flow: Turn states only play `RTG_W2_Stand_Aim_L_45` / `RTG_W2_Stand_Aim_R_45` Sequence Players; `TurnInPlacePlayRate` still drives their Play Rate. `UFPSCharacterAnimInstance` reads the baked `TurnRootYaw` curve delta and calls `AFPSCharacterBase::ApplyBodyTurnRootYawDelta()`, which rotates outer `BodyVisualYaw` / `BodyRoot`. No `Transform Modify Bone(root)` node is needed for turn correction.
- Editor follow-up: remove any state-local or global `Transform Modify Bone(root)` nodes that were added only for `TurnRootYawPoseFixRotation`. Keep the Turn states simple and verify PIE idle turning again.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session68 addendum - Keep BodyRoot turn, globally cancel asset root yaw
- User tested/observed the failure mode more precisely: during a left turn, the pose returns toward the old/right Idle before snapping to the left final position. Diagnosis: the turn asset still carries root-bone yaw. `BodyRoot` is turning from the curve, but the animation root bone blends back to Idle on Turn->Idle, so both layers fight during the transition.
- C++ change: restored `UFPSCharacterAnimInstance::TurnRootYawPoseFixRotation`, but only for global use outside `LocomotionSM`. It is `FRotator(0, -CurrentTurnRootYaw, 0)`.
- Intended AnimGraph now: Turn states remain simple Sequence Players with Play Rate = `TurnInPlacePlayRate`; after `LocomotionSM`, add one global `Transform Modify Bone(root)` using `TurnRootYawPoseFixRotation`, then continue to downstream output. This preserves the original outer BodyRoot rotation scheme while neutralizing the asset's internal root yaw across Turn->Idle blends.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session68 addendum 2 - Hold completed turn pose until leaving idle
- User identified the more important visual issue: the original quick prototype looked acceptable because the rotating feet and Idle feet had matching in-place contact. With the curve-driven turn, the body really rotates to the new visual yaw while the state machine immediately blends back to the normal Idle pose, so the feet/pose snap back before the next state catches up.
- C++ change: `AFPSCharacterBase` now tracks `bBodyTurnPoseHoldActive`. It becomes true when a 45-degree turn step finishes while still idle, clears immediately when moving/falling or when a new turn step triggers, and is exposed through `UFPSCharacterAnimInstance::bHoldTurnInPlacePose`.
- Intended ABP change: Turn_L45 / Turn_R45 should not return to Idle while `bHoldTurnInPlacePose` is true. Use `Relevant Anim Time Remaining < 0.1 && !bHoldTurnInPlacePose` for Turn->Idle. Movement/falling transitions should still bypass this hold and leave the Turn state immediately.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session68 addendum 3 - Add turn serial for repeated held turns
- User observed that only the first turn stayed locked. Likely cause: after the first turn, ABP remains held inside the Turn state, so the next `bIsTurningInPlace` pulse does not re-enter the same state or restart its Sequence Player.
- C++ change: added `BodyTurnInPlaceSerial` on `AFPSCharacterBase`, exposed to ABP as `TurnInPlaceSerial`. It increments every time C++ triggers a new fixed 45-degree turn step.
- Intended ABP use: when held at the end of a Turn state, use `TurnInPlaceSerial` changing as the signal to re-enter/restart the matching Turn_L45 / Turn_R45 sequence. Idle entry alone is not enough because the state machine may never return to Idle while pose hold is active.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session68 addendum 4 - Drop pose hold; scale curve so final landing matches PawnYaw
- User rejected the held-pose direction and reframed the desired behavior: intermediate continuous rotation state is not important; the final landing point should match the Pawn/Idle facing. That means Turn can return to Idle normally as long as `BodyVisualYaw` ends at the locked Pawn yaw.
- C++ change: removed the hold/serial path from the public AnimInstance outputs. On each turn trigger, C++ now locks `BodyTurnTargetYaw = PawnYaw` and computes `BodyTurnRootYawScale = abs(YawDelta) / abs(BodyTurnInPlaceStepAngle)`. The baked `TurnRootYaw` curve still provides timing/velocity, but each curve delta is scaled before applying to `BodyVisualYaw`. When the turn lock expires, `BodyVisualYaw` snaps exactly to `BodyTurnTargetYaw`.
- Intended ABP: Turn_L45 / Turn_R45 can return to Idle normally by `Relevant Anim Time Remaining < 0.1`. No `bHoldTurnInPlacePose` / `TurnInPlaceSerial` handling is needed.
- Verification: Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-04-session69 - Park lower-body turn-in-place and clean C++ state
- Lower-body turn-in-place is parked on the project-native AFPSCharacterBase + ABP_BodyLocomotion route.
- Final runtime shape: AFPSCharacterBase drives BodyVisualYaw on BodyRoot while Pawn/camera yaw remains controller-driven. Turn steps lock a per-step target in C++ and use the TurnRootYaw curve only as progress/timing.
- Current C++ defaults: BodyTurnInPlaceThreshold=30, BodyTurnInPlaceMoveSpeed=3, BodyTurnInPlaceStepAngle=45, BodyTurnInPlaceAnimLockTime=0.45, BodyTurnInPlacePlayRate=1.3.
- Large yaw gaps are handled by repeated fixed 45-degree steps; BodyTurnSequenceId increments per step so UFPSCharacterAnimInstance resets curve progress for each loop.
- Removed obsolete cleanup fields and paths: BodyTurnRootYawScale, GetBodyTurnRootYawScale(), ApplyBodyTurnRootYawDelta(), BodyTurnInPlaceRequestDuration, BodyTurnRequestTimeRemaining, BodyTurnVisualCompleteTimeScale, and UFPSCharacterAnimInstance::TurnRootYawPoseFixRotation.
- ABP_BodyLocomotion should now keep Turn_L45_IP / Turn_R45_IP looped, connect Sequence Player Play Rate to TurnInPlacePlayRate, enter by bIsTurningInPlace + TurnInPlaceIndex, and leave by !bIsTurningInPlace. Remove old Transform Modify Bone(root) compensation nodes that referenced TurnRootYawPoseFixRotation.
- Verification: text search found no remaining Source references to removed fields. Build.bat reached UHT, then stopped because UE Live Coding is active; close editor/game or press Ctrl+Alt+F11 and rerun Development Editor / Win64 build.
- Next planned work per user: upper-body weapon animation.
- Post-handoff code hygiene: FPSCharacterBase.h had several real UPROPERTY macros hidden by old mojibake comments. Restored actual UPROPERTY lines for SprintSpeed, BaseArmsRotation, bArmsPitchFollow, ViewmodelRoot, ArmsViewMesh, BodyRoot, ShadowBodyMesh, LegsMesh, ArmsHiddenSections, and the BodyTurn tuning fields. UHT processed successfully after this repair; full C++ compile is still blocked only by active Live Coding.

### 2026-07-04-session70 - Upper-body weapon animation C++ glue
- User confirmed editor-side cleanup was done: blueprint references/Fix Up Redirectors/PIE checks and ABP_BodyLocomotion Turn_L45_IP / Turn_R45_IP setup were handled before this session.
- Clarified architecture: no new AnimInstance class is needed. Existing `EquipmentAnimLayerClass` remains the original FP viewmodel linked-layer field used by `AEquipmentBase::Equip()` on `AFPSCharacterBase::GetArmsMesh()` (`ArmsViewMesh`).
- User clarified both skeletons are identical and wants the body/shadow to sync only the FP arms animation pose, not the FP mesh component rotation or viewmodel offset.
- Reverted the temporary optional body firearm fields (`BodyEquipmentAnimLayerClass`, `BodyFireMontage`) before finalizing this session.
- Cleaned `AFirearm::Equip()` so the FP linked layer is not re-linked manually. It now only writes `MuzzleLocalTransform` into the already-linked FP `UFirearmAnimInstance`.
- `UFPSCharacterAnimInstance` now exposes `ViewmodelPoseSourceMesh` (the character `ArmsViewMesh`) and `bHasViewmodelPoseSource` for `ABP_BodyLocomotion`.
- Intended ABP_BodyLocomotion setup: `Copy Pose From Mesh` using `ViewmodelPoseSourceMesh`, then `Layered Blend per Bone` to mix only the upper body over normal body locomotion. FP `FireMontage` remains the single source of fire animation; the body receives it via Copy Pose.
- Verification: sandboxed build failed on UBT AppData log permission, escalated build reached UHT successfully, then stopped because UE Live Coding is active. Close editor/game or press Ctrl+Alt+F11, then rerun Development Editor / Win64 build.

### 2026-07-04-session70 addendum - Equipment animation base class
- User requested an extra inheritance layer so non-firearm equipment can share basic upper-body movement animation while firearms keep BBBAimIK-specific data.
- Added `UEquipmentAnimInstance` (`Equipment/Animation/EquipmentAnimInstance.h/.cpp`) as the common equipment animation parent:
  - `Speed`
  - `Direction`
  - `Velocity_Z`
  - `bIsFalling`
- Changed `UFirearmAnimInstance` to inherit from `UEquipmentAnimInstance` instead of `UAnimInstance`; existing firearm AimIK variables and update logic remain in `UFirearmAnimInstance`.
- Added `AnimGraphRuntime` to `TheManTest.Build.cs` and used `UKismetAnimationLibrary::CalculateDirection` to avoid the deprecated `UAnimInstance::CalculateDirection` warning.
- Verification: `Build.bat TheManTestEditor Win64 Development ... -WaitMutex -FromMsBuild` succeeded with no new warnings.

### 2026-07-04-session70 addendum 2 - Dual-mesh weapon layer link
- User clarified the final desired architecture: one main ABP can run on both `GetMesh()` and `ArmsViewMesh`; weapon switching should still use `EquipmentAnimLayerClass`.
- Reverted the Copy Pose plan and removed `UFPSCharacterAnimInstance::ViewmodelPoseSourceMesh` / `bHasViewmodelPoseSource`.
- Changed `AEquipmentBase::Equip()` so FPS characters link `EquipmentAnimLayerClass` to both:
  - `AFPSCharacterBase::GetArmsMesh()` (`ArmsViewMesh`, first-person viewmodel)
  - `AFPSCharacterBase::GetMesh()` (body/leader mesh used by shadow/legs)
- `AEquipmentBase::Unequip()` unlinks from both meshes. `PlayEquipMontage()` still plays on the first-person montage mesh (`ArmsViewMesh`) to preserve existing behavior.
- `AFirearm::Equip()` now writes `MuzzleLocalTransform` into the linked `UFirearmAnimInstance` on both `ArmsViewMesh` and `GetMesh()`, so BBBAimIK has a valid AimSource on both independent AnimInstances.
- Verification: `Build.bat TheManTestEditor Win64 Development ... -WaitMutex -FromMsBuild` succeeded with no new warnings.

### 2026-07-04-session70 handoff - Rifle upper-body layer editor state
- User configured both `GetMesh`/CharacterMesh0 and `ArmsViewMesh` to use `ABP_BodyLocomotion`, and weapon BP `EquipmentAnimLayerClass` to `ABP_Rifle_UpperBody`.
- `ABP_Rifle_UpperBody` inherits from `TABP_Firearm_UpperBodyBase`; the template implements `ALI_WeaponAnim`.
- `WeaponAimOffset` is pass-through for now.
- `WeaponUpperBody` direct output is confirmed working: `BlendSpacePlayer(BS_Rifle_UpperBody_IdleWalkRun) -> Output Pose` animates the arms.
- Remaining editor issue: `Layered Blend per Bone` inside `WeaponUpperBody` did not blend the BlendSpace over `UpperBodyInPose`.
- Next steps:
  - Rebuild `Layered Blend per Bone` from the working direct state.
  - `UpperBodyInPose -> Base Pose`; `BlendSpacePlayer -> Blend Poses 0`; `Blend Weight 0 = 1.0`.
  - Branch filter under `Layer Setup[0]`: `spine_01`, depth `2`; enable Mesh Space Rotation Blend. User verified depth `999` does not work in this setup.
  - If still not visible, test branch filters `clavicle_l`/`clavicle_r` or `upperarm_l`/`upperarm_r` and verify the filter is attached to Blend Pose 0.

### 2026-07-04-session71 note - WeaponUpperBody blend depth verified
- User verified the `Layered Blend per Bone` setup inside `WeaponUpperBody`.
- Working branch filter: Bone Name `spine_01`, Blend Depth `2`.
- Previous guidance to use Blend Depth `999` is wrong for this current setup; it failed to blend correctly. Keep the value at `2` unless the skeleton/layer graph changes.
- User decided upper-body weapon locomotion should only distinguish Walk vs Run speed, not movement direction. Build `ABP_Rifle_UpperBody` around `Speed` only, using a 1D BlendSpace or threshold blend. Do not spend time wiring a 2D Direction x Speed upper-body BlendSpace for the rifle layer.
