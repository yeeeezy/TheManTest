# 进度日志

## 2026-09-01 session251 交接：R键 Gameplay Tag 换弹

- 新增 `IA_Reload` 并在 `IMC_Default` 映射 R；Controller 持有 Action，Character 绑定后发送 `Input.Weapon.Reload` Gameplay Event。
- 新增共享 `UGA_Reload/BGA_Reload`；`BP_RepairGun` 通过独立 `ReloadAbilityClass` 配置，装备授予、卸下回收，不重构武器技能集。
- 当前即时换弹；满弹拒绝，空弹补满并消耗一个备用弹夹，现有 `OnAmmoChanged` 自动刷新 HUD。
- Development Editor 冷构建、相关蓝图编译保存、`TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success。

## 2026-09-01 session250 交接：中心准星缩小20%

- 空心准星最终半径46.08px、线宽2px、80分段；HUD 其余布局不变。
- Development Editor 构建和 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；最新截图已覆盖并打开，待用户确认。

## 2026-09-01 session249 交接：HUD 改为视觉中心对齐

- 撤销仅按字体基线判断对齐的方案；四个 HUD 文本元素现按同一条视觉中心线排列，小号 `+` 与备用弹夹数居中于大号数字。
- Development Editor 构建和 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；最新截图已覆盖，待用户确认。

## 2026-09-01 session248 交接：HUD 字体基线对齐

- 血量图标、血量值、当前子弹和备用弹夹改为同一条 Slate 字体基线，修复小字号数字下沉造成的横向不齐。
- Development Editor 构建和 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；最新截图已覆盖并打开，待用户确认。

## 2026-09-01 session247 交接：按参考图加入血量并重排弹药

- HUD 底部新增当前血量；只显示当前值，不显示最大值。
- 弹药改为大号当前子弹数与右侧小号备用弹夹数，不显示弹夹容量；中心准心保持不变。
- PlayerController 绑定 PlayerState ASC 血量委托，切角色时解绑重绑；无 UI Tick。
- Development Editor 构建和 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；最新截图 `Saved/Screenshots/WindowsEditor/TMT_CombatHUD.png`，待用户确认观感。

## 2026-09-01 session246 交接：Combat HUD 准星再放大1.8倍

- 中心空心圆半径由32px改为57.6px，线宽2.5px、96分段；弹药布局与逻辑不变。
- Development Editor 构建与专项自动化成功；新截图已覆盖并打开，待用户确认观感。

## 2026-09-01 session245 交接：Combat HUD 准星放大四倍

- 中心空心圆半径由8px改为32px，线宽2px、64分段；弹药布局与逻辑不变。
- Development Editor 构建与弹药生命周期专项自动化均成功；新截图已覆盖 `TMT_CombatHUD.png`，待用户确认观感。

## 2026-09-01 session244 交接：Combat HUD 与玩家弹药第一阶段

- 当前 active feature 切换为 FEAT-078；FEAT-077 保持 `needs_improvement` 等待前台动画主观复核。
- `ATheManPlayerController` 本地创建原生 Combat HUD：屏幕中心8px空心圆；右下角两行显示 `当前弹药 / 容量` 与 `弹夹 数量`，Hit Test Invisible、无 Tick。
- `AFirearm` 默认30发容量、当前30发、备用弹夹3；开火前真实扣弹，空弹不产生任何开火反馈；提供换弹接口。EquipmentManager/Firearm 委托驱动 HUD，切枪和切角色会解绑重绑。
- Development Editor 构建成功；`TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；截图 `Saved/Screenshots/WindowsEditor/TMT_CombatHUD.png`。待用户前台确认布局观感；换弹输入/动画尚未接。

## 2026-09-01 session243 交接：按最新截图更新 ArmsViewMesh 默认值

- `AFPSCharacterBase` 的 `ArmsViewMesh` 默认 Transform 更新为 Location `(-14.766994,-4.322017,-134.599387)`、Rotation `(Roll=5.752239°, Pitch=0.077753°, Yaw=-114.908449°)`、Scale `(1,1,1)`。
- Development Editor 构建成功；冷启动 Native/BP CDO 均与截图一致。用户最新 `BP_MaintenanceWorker.uasset` 手调仍保留在工作区，未纳入上一安全检查点。
- 静态构图仍不在 BeginPlay/Tick 强制覆盖；待用户前台确认最终画面。

## 2026-09-01 session242 交接：ArmsViewMesh 当前构图设为父类默认值

- 根据用户截图，将 `AFPSCharacterBase` 构造默认 `ArmsViewMesh` Transform 设为 Location `(-6.330288,-6.449130,-141.685038)`、Rotation `(Roll=0.581459°, Pitch=0.297436°, Yaw=-117.090375°)`。
- 不新增 BeginPlay/Tick 静态覆盖；BP 仍可改构图。当前 `BP_MaintenanceWorker` 原值与新默认一致，无需重写蓝图资产即可保持相同画面。
- Development Editor 构建成功；冷启动 Native/BP CDO 回读均与目标值一致。待用户打开蓝图确认默认值显示及前台构图。

## 2026-09-01 session241 交接：维修工手臂 Mesh 与正式动画 Skeleton 统一

- `SKM_MaintenanceWorker_FirstPersonArms` 已从旧 `SKEL_MaintenanceWorker_FirstPersonArms` 改为正式 `SK_Mannequin_Arms_Skeleton`；不恢复短名称动画、不配置兼容 Skeleton、不重新迁移动画。
- 修改前逐骨验证两侧均为68骨，名称和父子层级完全一致。冷启动确认 Mesh、完整名称六条 Sequence、BlendSpace 与第一人称 AnimBP 全部使用同一个 Skeleton。
- 临时 Editor-only 赋值/验证代码已清除，最终 `TheManTestEditor Win64 Development` 构建成功。旧 Skeleton 资产暂未删除；用户原有 `BP_RepairGun` 与 TestMap ExternalActor 改动未触碰。

## 2026-09-01 session240 交接：删除未使用的短名称第一人称动画副本

- 已确认短名称 `AS_MW_FP_*` 使用 `SKEL_MaintenanceWorker_FirstPersonArms`，完整名称正式资产使用 `SK_Mannequin_Arms_Skeleton`；因此骨骼编辑器的兼容资产筛选结果不同。
- Asset Registry 预检确认短名称六条 Sequence 与 `BS_MW_FP_WalkRun` 外部引用均为0；七个资产已通过 Unreal 删除，冷启动回读剩余目标为0。
- 正式 `AS_MaintenanceWorker_FP_*` 六条 Sequence、`BS_MaintenanceWorker_FP_WalkRun` 和 `ABP_MaintenanceWorker_FirstPerson` 冷启动加载全部成功。用户原有 `BP_RepairGun` 与 TestMap ExternalActor 改动未纳入本次操作。

## 2026-08-23 session239 交接：RepairGun 可见枪体切换为 SK_SCFRIFLE

- `BP_RepairGun.SkeletalMesh` 已指定 `/Game/Weapons/RepairGun/Meshes/SK_SCFRIFLE`；装备 BeginPlay 在 Skeletal Mesh 有资产时显示它并隐藏旧 `StaticMesh` 及其 Overlay 子组件。旧 Static Mesh 资产仍保留，继续作为现有世界空间枪械影子的复制源。
- 用户将自行在 `BP_RepairGun.SkeletalMesh` 上调整握持 Relative Transform；当前未替用户校准 GripPoint。枪口仍优先读取 Skeletal Mesh 的 `MuzzleSocketName`，不存在时回退现有 `MuzzleLocalTransform`。
- 已确认并记录两个暂不处理的 Bug：RepairGun 描边无可见效果；装备消融无可见效果。
- Development Editor 构建成功；运行时断言确认 Skeletal Mesh 已加载且可见、旧 Static Mesh 已隐藏，截图 `Saved/Screenshots/PlayerFramingCurrent.png` 显示新枪体进入画面。FramingCapture 仅继续被用户当前 FOV 与旧 77° 断言不一致阻断。

## 2026-08-23 session237 交接：蓝图预览与 PIE 手臂朝向差异已实证定位

- 临时逐骨探针确认 `ArmsViewMesh` 蓝图 CDO 与 PIE Relative Transform 完全一致：Rotation `(-0.087114,-82.335250,0.647280)`、Location `(-1.615754,0.000001,-141.738776)`；不存在 PIE 强制覆盖组件构图。
- CDO 冷回读 `Update Animation in Editor=false`。因此角色蓝图组件视口不持续求值 `ABP_MaintenanceWorker_FirstPerson`，截图显示参考姿势；PIE 才执行 BeginPlay 装备/动画层初始化与逐帧 AnimBP 求值。
- `AS_MaintenanceWorker_FP_Idle` 对参考姿势的组件空间角差：`upperarm_r=36.248°`、`lowerarm_r=109.692°`、`hand_r=72.553°`；PIE 对同一 Idle 的对应误差仅 `0.585°/0.457°/0.610°`。巨大朝向差由“参考姿势 vs 实际 Idle”确定性解释，不是 Lean/Look、ViewmodelRoot 或 RepairGun 额外旋转。
- 临时 C++ 探针已移除。FramingCapture 运行到探针并成功产出数据，但因用户当前蓝图 FOV 已不再是旧测试硬编码的 77° 而失败；该旧断言需另行按用户当前构图语义处理。

## 2026-08-23 session236 交接：修正第一人称手臂近距离裁切方向

- 用户截图确认 session235 的公式方向相反，远处手臂保留但贴近相机的手肘仍会遮挡。现改为 `(Distance(CameraPositionWS, AbsoluteWorldPosition) - Arm Near Clip Distance) / Arm Near Clip Fade Width`，近处裁掉、远处保留。
- `MI_MaintenanceWorker_FirstPersonArms` 最终值为 Near Clip Distance 40cm / Fade Width 8cm；只影响第一人称 Arms，身体、影子和武器材质未改。
- 手臂独立证据图 `Saved/Screenshots/WindowsEditor/TMT_FPArmNearClip_ArmsOnly_40cm.png` 显示近镜头肘部主体被裁掉、双手与前臂保留；正常持枪图 `Saved/Screenshots/PlayerFramingCurrent.png` 无整块手肘遮挡。
- 正常 DX12 冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success。一次额外 `-NullRHI` 截图尝试因测试需要渲染视口而崩溃，不属于游戏运行路径；正常渲染复跑成功。
- FEAT-077 继续 `in_progress`，待用户前台在实际动作姿势中审核 40cm/8cm 过渡。

## 2026-08-23 session234 交接：最终使用 Arms Transform 调全部静态构图

- 按用户确认删除冗余 `SprintPivot`，最终层级为 `HeadCamera -> ViewmodelRoot -> ArmsViewMesh`。
- 用户只通过蓝图 `ArmsViewMesh.Transform` 调整所有静态构图；C++ 不读写 Arms Transform。`ViewmodelRoot` 不用于静态构图，Tick 只写实际速度驱动的冲刺 Pitch。
- 写入前检查点 `8331987`；Development Editor 完整构建成功，冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success。
- 用户的 `BP_MaintenanceWorker.uasset` 与 TestMap ExternalActor 改动保持原样。FEAT-077 继续 `in_progress`，待用户前台用 Arms Transform 确认最终构图。

## 2026-08-23 session233 交接：蓝图组件 Transform 恢复为静态构图权威

- 已删除 `Viewmodel Offset Location/Rotation`，C++ 不再覆盖 `ViewmodelRoot` 或 `ArmsViewMesh` 的静态 Transform；蓝图组件预览值即运行时值。
- 新层级为 `HeadCamera -> ViewmodelRoot -> SprintPivot -> ArmsViewMesh`；C++ Tick 只写 `SprintPivot.Pitch`，保留实际速度驱动的冲刺压枪。自动化层级断言已同步。
- 写入前检查点 `2bdee43`。关闭编辑器后 Development Editor 完整构建成功；冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 在删除与可编辑 Transform 冲突的旧横轴断言后复跑 1/1 Success。
- 用户的 `BP_MaintenanceWorker.uasset` 与 TestMap ExternalActor 改动保持原样。FEAT-077 继续 `in_progress`。

## 2026-08-23 session232 交接：唯一构图旋转改为相机中心支点

- 已删除 `Viewmodel Arms Rotation` / `BaseArmsRotation`；`ArmsViewMesh.RelativeRotation` 固定为零。
- 只保留实例可调的 `Viewmodel Offset Location` 和 `Viewmodel Offset Rotation`。Location 每帧写 Arms；Rotation 默认 `(-3,-90,-1)`，每帧写相机中心的 `ViewmodelRoot`，冲刺 Pitch 在其上叠加。
- 写入前检查点 `c7f1532`；`TheManTestEditor Win64 Development` UHT/编译/链接成功。用户的 `BP_MaintenanceWorker.uasset` 与 TestMap ExternalActor 改动保持原样。
- FEAT-077 继续 `in_progress`；待前台 PIE 调整两个 Offset 参数并确认最终构图。

## 2026-08-23 session231 交接：PIE 实例构图改为每帧直接覆盖

- 已删除 session229 的 `PostEditChangeProperty` 判断路径；Tick 每帧无条件写入 `Viewmodel Offset Location` 与 `Viewmodel Arms Rotation`，PIE 运行实例修改后下一帧生效。
- 冲刺动态 Pitch 仍仅作用于父级 `ViewmodelRoot`，不覆盖 Arms 自身旋转。`TheManTestEditor Win64 Development` 完整构建成功。
- 写入前检查点 `8fb7f78`。用户未提交的 `BP_MaintenanceWorker.uasset` 与 TestMap ExternalActor 删除保持原样，未纳入本次修改。
- FEAT-077 继续 `in_progress`；待用户前台 PIE 手调构图并确认最终数值。

## 2026-08-22 session230 交接：构图问题未解决，待用户手动测试

- 当前第一人称枪械位置/朝向构图问题仍未解决，不得将 FEAT-077 标记完成；用户将于明天在 PIE 运行实例中手动调整并验证。
- 待测入口：`Viewmodel|Framing` 下的 `Viewmodel Offset Location` 与 `Viewmodel Arms Rotation`。测试前先将蓝图旧的 `Viewmodel Offset Rotation` 重置为 `(0,0,0)`，避免父级支点旋转造成绕相机偏移。
- PIE 属性修改采用编辑器 `PostEditChangeProperty` 按修改即时应用，不存在 Tick 静态 Transform 检测或逐帧覆盖。当前用户手调的 `BP_MaintenanceWorker.uasset` 保持未提交、未覆盖。
- 本次仅记录停机交接，未新增实现或验证；最近一次 Development Editor 冷构建成功，但 FramingCapture 仍需在旧蓝图偏移清零并完成手动构图后重跑。

## 2026-08-22 session229 交接：Viewmodel 构图支持 PIE 实例即时编辑

- `Viewmodel Offset Location` 与 `Viewmodel Arms Rotation` 改为 `EditAnywhere`，可在 PIE 运行实例 Details 中调整。
- 新增编辑器专用 `PostEditChangeProperty`：仅在这两个属性真正被编辑时立即更新 `ArmsViewMesh` Location/Rotation；普通 Tick 不检测、不比较、不覆盖静态构图，打包版本无该编辑器回调。
- 写入前 WIP 检查点：`0b54ba2`。Development Editor 冷构建成功。用户未提交的 `BP_MaintenanceWorker.uasset` 手调继续保留且未被覆盖；旧 ViewmodelRoot Rotation 仍需在蓝图中重置为0后再用 Arms Rotation 调朝向。

## 2026-08-22 session228 交接：暴露 Arms 自身构图旋转

- 将原 `BaseArmsRotation` 从 `Mesh` 分类整理到 Class Defaults 的 `Viewmodel|Framing`，显示名为 `Viewmodel Arms Rotation`，默认仍为 `(-3,-90,-1)`；BeginPlay 直接应用于 `ArmsViewMesh`，调整其 Yaw 不会像父级 ViewmodelRoot 那样绕相机原点画圆。
- 写入前 WIP 检查点：`3af3217`。Development Editor 冷构建成功；Shadow 与 EquipDissolve 回归 Success。FramingCapture 正确失败并揭示用户刚才的 BP 手调已保存旧 `Viewmodel Offset Rotation`/Location 覆盖，需用户在 BP 中将旧父枢轴 Rotation 重置为0，再使用新 Arms Rotation，之后补跑构图回归。
- `BP_MaintenanceWorker.uasset` 的现有未提交改动属于用户手调，本轮未覆盖或提交。

## 2026-08-22 session227 交接：冲刺压枪改为实际速度驱动

- 将冲刺速度切换与视觉压枪解耦：`SprintTransitionAlpha` 继续按 Shift 意图在0.2秒内切换 `MaxWalkSpeed` 550→750；新增局部 `SprintVisualAlpha`，按实际水平速度在 `WalkSpeed..SprintSpeed` 映射0..1。
- `ViewmodelRoot.Pitch` 仅使用 `SprintVisualAlpha`：原地按 Shift、受阻或实际速度不超过550时不下压，超过550后随速度连续压至最大角度；静态 Arms Transform 仍只在 BeginPlay 应用一次。
- 新增运行时断言“原地持有 Sprint 不压枪”。写入前 WIP 检查点：`102ffab`。Development Editor 冷构建成功；3项 `TheManTest.Player` 回归全部 Success。

## 2026-08-22 session226 交接：静态 Viewmodel 构图改为初始化时应用

- `ViewmodelRoot.Location`、`ArmsViewMesh.ViewmodelOffsetLocation` 与 `BaseArmsRotation` 改为在 `BeginPlay`（蓝图默认值加载后）应用一次；Tick 不再重复覆盖这些静态 Transform。
- Tick 仅保留 `ViewmodelRoot` 的动态冲刺 Pitch，以及 AnimBP Lean/Look 参数更新；双 AnimBP、模板继承、Linked Layer 与 Body Copy Pose 架构未改。
- 写入前 WIP 检查点：`2ba4834`。首次冷链接因运行中的 Unreal Editor 锁定 DLL 报 LNK1104；用户关闭编辑器后 Development Editor 冷链接成功，3项 `TheManTest.Player` 回归全部 Success。

## 2026-08-22 session225 交接：暴露并加快 Viewmodel Body Sway 插值

- 普通移动 Body Sway 插值速度从硬编码 `2.0` 改为蓝图可调 `ViewmodelBodySwayInterpSpeed`，位于角色 Class Defaults 的 `Viewmodel|Movement`，默认 `6.0`；A/D/W/S 进入倾斜与松键回弹共用该速度。
- 冲刺继续保持原版硬编码 `8.0`，未改变 session224 恢复的75° Lean/Look骨骼映射及其表现边界。
- 写入前 WIP 检查点：`2e3b5ad`。Development Editor 冷构建成功；3项 `TheManTest.Player` 回归全部 Success。

## 2026-08-22 session224 交接：恢复 session220 骨骼倾斜版本

- 用户决定保留最早“倾斜正确但存在上下平移”的版本。玩家角色源码与相关测试已精确恢复到 `e1c24eb` / session220：完整75° `RemappedLeanRoll/RemappedLookPitch` 骨骼映射恢复。
- session221 的装备 Actor 独立 Roll、session222 的 `ArmsViewMesh` 圆弧位置补偿、session223 的符号修正及其专项测试均已移除；三份相关源码与 `e1c24eb` 定向比较无差异。
- Development Editor 冷构建成功；恢复后的3项 `TheManTest.Player` 回归全部 Success。当前已知边界是 A/D 倾斜观感按用户认可版本恢复，同时保留其原有枪口上下平移。

## 2026-08-22 session223 交接：纠正 VFXPack A/D 倾斜方向

- 用户指出 session222 虽通过骨骼角度与枪口高度测试，但实际旋转方向完全相反；此前自动化只验证“发生旋转”，未验证输入方向，属于验收缺口。
- 对照历史原版运行记录（A→正 Lean、D→负 Lean）与 VFXPack 右移证据图 `TMT_VFXPack_StrafeRight.png`，确认 Enhanced Input 为 A=`X=-1`、D=`X=+1`，因此 Body Sway 必须使用 `SideInput=-MoveInput.X`。现已纠正符号。
- 专项测试新增硬断言 D 输入必须得到小于 `-5°` 的 `Lean_Sides_Amount`，并保存真实 D 输入构图供目视检查；新图与原版右移图均为枪身左上→右下倾斜。Development Editor 构建成功；全部4项 `TheManTest.Player` 回归 Success。

## 2026-08-22 session222 交接：恢复 VFXPack 骨骼倾斜并锁定枪口高度

- 用户前台确认 session221 只旋转装备根节点导致“手不动、只有枪口动”。重新以 `UE389_MuzzleSource/.../VFXPack` 原始 `FirstPersonCharacter` 与 `FirstPerson_AnimBP` 为准审计：A/D 应驱动 AnimBP 的 `spine_03` Component Space Additive Roll，并由后代 `hand_r/GripPoint` 带枪，`hand_l` 叠半倍率 Roll；原版不旋转武器 Actor。
- 删除装备根节点独立 Roll，恢复平滑 `Lean_Sides_Amount × 8` 骨骼驱动；移除错误的 75° Roll/Pitch 交叉映射，纯 A/D 的 `Look_Up_Amount=0`。
- 自动化量化发现，仅恢复骨骼链会因本项目较长的相机空间手臂构图使枪口高度漂移约 `-18.6cm`。现以 `spine_03→muzzle` 中性向量计算原版 Roll 的圆弧，并对 `ArmsViewMesh` 施加等量反向构图补偿；手臂和枪仍由同一骨骼 Pose 倾斜，武器 Actor 相对 Transform 不变。
- 新增 `TheManTest.Player.Viewmodel.VFXPackLateralSway`：实测 `spine_03=6.92°`、`hand_r=7.82°`，枪口相对 ViewmodelRoot 高度变化约 `-0.006cm`，无 A/D→Pitch 串扰。Development Editor 构建成功；全部4项 `TheManTest.Player` 回归 Success。待用户前台 PIE 主观验收。

## 2026-08-22 session221 交接：枪械 Roll 与骨骼位置滞后解耦

- 根因确认：旧 A/D 输入经 75° 基差同时写入 AnimBP 的 `Lean_Sides_Amount` / `Look_Up_Amount`，上游 Modify Bone 枢轴使手部 `GripPoint` 沿弧线移动，因此视觉上出现上下平移；组件 Location 本身并未变化。
- 暂时关闭 AnimBP 的 Lean/Look 骨骼滞后输入。A/D 仅在当前装备自身根节点施加即时本地 X/Roll `-8°..+8°`；装备根节点相对 Location 每帧锁定为零，因此枪体绕自身前轴倾斜但挂点保持原地。
- `FramingCapture` 新增左右输入运行时断言：MoveLeft/MoveRight 下装备根节点 Location 均为零，Roll 分别为 `-8°/+8°`。Development Editor 构建成功；`FramingCapture`、`Shadow.UpperBodyEvidence`、`EquipDissolveEvidence` 均 1/1 Success；截图人工审查未见构图或影子回归。待用户晚间前台主观复核。

## 2026-08-22 session220 交接：撤回移动位置方案并恢复原枪体绕轴旋转

- 用户澄清需求不是上下/左右位置滞后，而是枪体保持原地时沿枪管前向轴的旋转。session218/session219 对 ViewmodelRoot XY 位移及 Roll/Pitch 分解的正式改动均撤回。
- `FPSCharacterBase` 与相关玩家测试源码已精确恢复到远程基线 `e1c24eb`：`ViewmodelRoot.Location` 每帧为零，完整 75° `RemappedLeanRoll/RemappedLookPitch` 映射恢复，原枪体绕轴倾斜重新生效。
- 临时最终旋转探针曾确认 A/D 时武器 Actor 相对相机角从约 Roll `-13.86°` 变化到 `+4.74°`；探针与错误位置专项测试均已删除。
- Development Editor 构建成功；`Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session219 交接：恢复侧移枪械 Roll 幅度

- 用户前台确认 session218 去除侧移 Pitch 串扰后枪械倾斜不明显。根因是仍保留旧 75°投影的 `cos(75°)`，把原始 `±8°` 侧移 Roll 压缩为约 `±2.07°`。
- A/D 的纯绕枪轴 Roll 已恢复为原始 `SourceLeanRoll=±8°`；前后输入对 Lean 的既有基差补偿保留，侧移到 Look/Pitch 的交叉项仍为0，独立 ViewmodelRoot XY 位置滞后不变。
- `MovementLagDirections` 强化为平滑后 Roll 绝对值必须大于5°且 Pitch 小于0.05°；专项测试、Development Editor 构建及原三项玩家动画回归均 Success。

## 2026-08-22 session218 交接：第一人称移动惯性分层修正

- 临时诊断开关分别隔离 WalkRun、Lean、Look 后量化 `hand_r` 相对相机高度：WalkRun 约 `0.23cm`；Lean 左右总差约 `2.29cm`；Look 左右总差约 `4.63cm`；原组合总差约 `6.81cm`。`ViewmodelRoot` 与 `ArmsViewMesh` 原 Location 全程固定，确认问题主因是侧移经 75°换轴串入 Look/Pitch，而非组件平移。
- 临时诊断代码已撤回。正式实现保留现有侧移 `Lean/Roll`（枪械绕轴偏转），只移除侧移到 `Look/Pitch` 的交叉项；前后输入仍独立驱动 Look/Pitch。
- 新增相机局部 `ViewmodelRoot` XY 位置滞后：A→+Y、D→-Y、W→-X、S→+X，Z 强制为0；默认侧向3cm、前后2cm、插值速度6，参数可在角色默认值调整。
- 新增 `TheManTest.Player.Viewmodel.MovementLagDirections`，验证四向反向滞后、无Z位移、A/D保留Roll且不串入Pitch。该测试、Development Editor 构建及原三项玩家动画回归均 Success。

## 2026-08-22 session217 交接：枪械 WalkRun 恢复 Blend Space 占位

- 纠正 session216 的错误方向：`TABP_FirstPersonFirearmBase.WeaponUpperBody/WalkRun` 不应使用 Run Sequence 冒充移动混合，现已替换为无具体资产的 `Blend Space Player`，并将 `Speed` 接入 X。
- `ABP_RepairGun_FirstPerson.ParentAssetOverrides` 已移除原 WalkRun Sequence Override，改为覆盖 `/Game/Weapons/RepairGun/Animations/FirstPerson/Locomotion/BS_WalkRun_RepairGun`；Idle 仍使用空 Sequence Player + Idle Sequence Override。
- 冷导出确认 WalkRun 图只包含一个连接到 State Result 的 Blend Space Player，不再存在 WalkRun Sequence Player；模板本身不绑定具体 BlendSpace。
- `TheManTestEditor Win64 Development` 构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session216 交接：枪械模板断线播放器清理

- 冷导出确认 `TABP_FirstPersonFirearmBase.WeaponUpperBody/WalkRun` 的实际输出早已连接空 `Sequence Player`；用户看到的非 Sequence 节点是旧方案遗留、完全断线的 `Blend Space Player`。
- 删除断线 `Blend Space Player` 及仅向它供值的孤立 Speed Getter。模板现在 Idle、WalkRun 各只有一个连接到 State Result 的空 `Sequence Player`。
- `ABP_RepairGun_FirstPerson` 继续通过 Parent Asset Overrides 提供 `AS_MaintenanceWorker_FP_Idle` 与 `AS_MaintenanceWorker_FP_Run`；枪械模板对 `/Game/Weapons/RepairGun/` 的具体资产依赖为 0。
- `TheManTestEditor Win64 Development` 构建成功；带渲染冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture` 及冷启动 `Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。`-nullrhi` 不适用于前两项截图型测试，会在截图辅助代码中访问无效渲染资源。

## 2026-08-22 session215 交接：角色动画模板去具体资产化

- `TABP_CharacterBase_FirstPerson` 的状态机由默认名 `New State Machine` 改为 `FirstPersonLocomotionSM`。
- 两个 CharacterBase 模板均只保留结构节点：动画序列使用无具体资产的 `Sequence Player`，移动混合使用无具体资产的 `Blend Space Player`；冷启动依赖审计确认两者对 `/Game/Characters/MaintenanceWorker/` 的具体资产依赖均为 0。
- `TABP_CharacterBase_BodyLocomotion` 原本已经通过子类 Asset Override 绑定具体动画，本轮无需迁移；`TABP_CharacterBase_FirstPerson` 的 7 个播放器绑定已迁入 `ABP_MaintenanceWorker_FirstPerson` 的 Parent Asset Overrides（Idle、Still×2、JumpStart、JumpLoop、JumpEnd、WalkRun BlendSpace）。
- 第一人称模板中 3 条依赖具体 Sequence 的剩余时间 Getter 已改为状态机原生自动剩余时间过渡，保证播放器清空资产后模板仍可独立编译。
- `TheManTestEditor Win64 Development` 构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session214 交接：第一人称宿主模板化

- 新增无骨架 `/Game/Characters/CharacterBase/Animations/FirstPerson/Logic/TABP_CharacterBase_FirstPerson`，完整继承原第一人称基础 locomotion/Jump、`ALI_WeaponAnim` 路由和最终 Lean/Look 图。
- `ABP_MaintenanceWorker_FirstPerson` 保留原路径与具体 `SK_Mannequin_Arms_Skeleton`，改为新模板子类并移除本地 AnimGraph；冷审计确认 ParentClass 指向模板、模板 `TargetSkeleton=None`、子类本地 `AnimGraph=None`、模板 AnimGraph 存在。
- C++ 新增编辑器辅助 `CreateFirstPersonHostTemplate`，用于从已验证宿主安全生成无骨架模板并把具体宿主转换为纯继承子类。
- 写入前 WIP 检查点：`20be43d`。`TheManTestEditor Win64 Development` 冷构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session213 交接：身体 Locomotion 模板提升至 CharacterBase

- 无骨架模板 `TABP_MaintenanceWorker_BodyLocomotion` 已通过 Unreal AssetTools 改名并迁移为 `/Game/Characters/CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion`。
- 具体实现 `ABP_MaintenanceWorker_Body` 继续留在 MaintenanceWorker 角色目录，并已重新编译保存；它是新模板当前唯一引用方。
- 此次只提升通用状态机与上半身 Pose 合成框架，没有移动 MaintenanceWorker 的 Skeleton、动画序列或 BlendSpace。
- 写入前 WIP 检查点：`a3da38b`。新旧路径、唯一引用、两项资产加载验证及 AnimBP 编译保存均已通过；旧路径 Asset Registry 为0且未留下 Redirector。
- 关闭编辑器后 `TheManTestEditor Win64 Development` 冷构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session212 交接：全项目资产目录规范化启动

- 当前活动功能切换为 FEAT-078；FEAT-077 已保存至检查点 `d923f88`，自动化通过但仍待用户前台主观验收，因此状态改为 `needs_improvement`。
- 新增 `arch/00-asset-directory-standard.md`：统一采用所有者优先、资源类型次之的结构，并规定 `_Shared` 必须有真实复用证据。
- 用户确认 MaintenanceWorker 专属表现资产全部迁回具体角色；Infiltrator 与 TheExecutive 尚无具体 Mesh，不创建空目录或复制占位美术资产。
- AssetTools 最终采用单次批量事务迁移248个资产，并补迁 InteractableBase 内部4个资产；顺序迁移实验发现依赖保存风险后已从检查点完整恢复，没有保留失败结果。
- CharacterBase 最终只剩 BP、Data、基础 Ability/Effect 四个资产；Actors、Weapons、Enemy 与具体角色均统一为规范分类名。
- 两个 TestMap External Actor 已重存并完成二次 Redirector Fixup；Characters、Actors、Weapons、Enemy 四个根 Redirector 均为0，旧非规范目录资产为0，清理72个空目录。
- 两条零引用且 Skeleton 已损坏的旧 Rifle Sequence 删除；10个 AssetTools 遗留的零引用旧 Skeletal Mesh 完整副本删除，均可从检查点 `d923f88` 恢复。
- MaintenanceWorker 33条 AnimSequence 冷加载 Skeleton 全有效；Development Editor构建和三项玩家动画回归全部Success。

## 2026-08-21 session211 交接：第一人称 Linked Anim Layer 模板化

- `ArmsViewMesh` 常驻宿主整理为 `/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/ABP_MaintenanceWorker_FirstPerson`，父类为 `UCharacterBaseAnimInstance`；地面持枪 Pose 改由 `ALI_WeaponAnim.WeaponUpperBody` Linked Layer 提供，腾空时切回宿主原 Jump 状态机以保持改前表现，原 `spine_03/hand_l` 通用晃动仍在最终输出端。
- 枪械模板正式命名为 `/Game/Weapons/_Shared/Animations/Templates/TABP_FirstPersonFirearmBase`，父类 `UFirearmAnimInstance`；维修枪子类为 `/Game/Weapons/RepairGun/Animations/FirstPerson/Logic/ABP_RepairGun_FirstPerson`。切枪只链接层，不更换 Arms 主 AnimInstance。
- `UCharacterBaseAnimInstance` 继承 `UFPSCharacterAnimInstance`，统一由 C++ 提供 Speed/Direction/VelocityZ/bIsFalling、`Character_Speed/Is_Moving/Is_InAir` 与 Lean/Look；第一人称宿主的素材包 EventGraph 驱动已清空。
- 身体具体实现归入 MaintenanceWorker：`ABP_MaintenanceWorker_Body`；其无骨架通用模板随后在 session213 提升为 `CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion`。无引用 `TABP_CharacterBase` 已删除。
- Development Editor 编译成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均 Success，验证影子上半身同步、构图和装备显隐未相对改前回归。
- 待办：用户前台实际输入复核 Idle/WASD/Run/Jump/切枪/开火主观观感。

## 2026-08-21 session210 交接：双 AnimBP 上半身 Pose 合成

- 当前活动功能切换为 FEAT-077；FlyingBug2 自动化已完成但待主观验收，FEAT-076 改为 `needs_improvement`。
- `ArmsViewMesh` 独立运行 `ABP_CharacterBase_FirstPerson_C`；`CharacterMesh0` 运行 `ABP_CharacterBase_Body_C`，在 Body 主图末端读取 `FirstPersonPoseSource`，只从 `spine_01` 以上混入 Arms 的局部骨骼 Pose。
- Arms 先 Tick、Body 后合成；Body root/pelvis/腿部 locomotion 不进入第一人称手臂。装备层与 Montage 只链接 Arms，CharacterMesh0 不再重复切换武器层。
- Development Editor 构建成功；Shadow UpperBody、FramingCapture、EquipDissolve 三项 PIE 自动化均 Success。
- 正式命名整理完成：第一人称类为 `FirstPerson/Logic/ABP_CharacterBase_FirstPerson`；身体类为 `Body/Logic/ABP_CharacterBase_Body`。旧素材来源名和 `Animations/Skeleton` 目录已清理。
- 待办：用户前台实际按键复核切枪、Idle/移动/跳跃/开火/冲刺；重点观察切枪首帧、影子上半身同步和下半身连续性。

## 2026-08-21 session209 暂停交接：FPSShooter1 第一人称架构调查

- 对 `D:\Unreal Projects\FPSShooter1` 做了只读导出调查，没有保存或修改外部工程；临时调查脚本与导出文件已清理。
- 原工程全身 `CharacterMesh0` 使用 `ABP_Unarmed` 运行完整 locomotion；第一人称 `ABP_FP_Copy` 不运行 locomotion 状态机，而是 `Copy Pose From Mesh（Use Attached Parent） -> CtrlRig_FPWarp -> Output`。
- 原工程武器 `ABP_FP_Weapon` 同样从最终第一人称 Mesh Copy Pose，再接第一人称 Control Rig/Aim 与 Montage Slot。
- 当前 TheManTest 的问题根因已经收敛：让 `ArmsViewMesh` 直接运行完整 `ABP_CharacterBase_Body`，会把 root/pelvis/下半身 locomotion 位移直接带进可见手臂；末端的 `spine_03/hand_l` Modify Bone 只能补旋转，无法消除该平移。
- 下一步建议（尚未实施）：`CharacterMesh0` 保留完整 Body AnimBP 和现有动画接口；为 `ArmsViewMesh` 建轻量 FP Copy AnimBP，复制 `CharacterMesh0` 的最终 Pose 后增加专用第一人称 Warp/校正，再叠加现有 Lean/Look；RepairGun 跟随最终 Arms Pose 或继续通过现有接口层连接。
- 暂停点：本次只记录调查结论，没有继续修改动画资产或架构。恢复工作时先基于上述 Copy Pose + FP Warp 方案设计最小改动并做 PIE A/D/W/Shift 对照验证。

## 当前状态

**最后更新：** 2026-08-22-session212

**当前功能：** FEAT-077 — 第一人称手臂与完整身体双 AnimBP 合成

**状态：** in_progress（FEAT-078 已完成归档）

## 当前待办

- 目录规范化实现与自动化验证已经完成；等待用户后续明确说“更新 Git”时再提交最终结果。
- FEAT-077 仍需用户前台主观复核切枪、Idle/移动/跳跃/开火/冲刺观感。

## FEAT-076 本轮进展

- session207：用户截图用原资产 root=0° 对照证明 session205 恢复 `-89.999977°` 的结论错误；异常确在迁入序列的 root 轨道。遵守 destination-only 边界，在 `FPSShooter1` 审计其现有最终资产，确认7条序列首尾 root Yaw 全为0°，备份到 `Saved/Codex/Backups/VFXPackFirstPerson_BeforeRootZero_20260821_114911` 后导出最终 FBX；TheManTest 只导入最终动画。Idle/Run/JumpStart/JumpLoop/JumpEnd/Fire 正常覆盖，Still 因0.066秒非帧边界首次被拒，启用 Snap to Closest Frame Boundary 后导入为3采样键。最终7/7首尾 root Yaw=0°；相关ABP/RepairGun Layer/BP_MaintenanceWorker编译保存，正式目录与三个使用方资产加载验证通过。FramingCapture 冷启动环境未注册该测试，不能声称PIE视觉通过，待用户前台复核。
- session206：按用户确认整理 MaintenanceWorker 当前使用/参考的 VFXPack 第一人称动画资产，不修改动画关键帧、root Transform、Skeleton 或重定向配置。通过 Unreal AssetTools 将旧 `/Game/Characters/CharacterBase/Animations/Legacy/VFXPackFirstPerson` 的 10 个资产迁至正式 `FirstPerson/Locomotion`、`FirstPerson/Actions`、`FirstPerson/Logic`；相关 AnimBP、RepairGun Layer 与 BP_MaintenanceWorker 编译保存成功，目标目录10个资产及三个运行使用方加载验证通过。首次只验证 Asset Registry 为0、漏删磁盘空目录；用户指出后已删除空 `VFXPackFirstPerson` 及随之变空的 `Legacy` 父目录，并强化迁移规则为 Asset Registry 与磁盘双重清场。用户下一步将在新正式目录手动修复动画。
- session205：撤销 session204 对 VFXPack 第一人称 7 条 AnimSequence 的错误 root `+90°` 烘焙。用户实际 PIE 证明该修改会令手臂/枪整体转错；从 `Saved/Backups/VFXPackFirstPerson_20260806_195138` 恢复修改前资产，7/7 文件恢复核对完成，冷审计首尾 root Yaw 均回到原始约 `-89.999977°`。PIE 截图 `TMT_RestoredVFXPack_ReferenceCheck.png` 与桌面参考图构图复核：枪位于右下、枪管朝前、准星无遮挡。PIE 与 BP CDO 的 HeadCamera、ViewmodelRoot、ArmsViewMesh、CharacterMesh0、LegsMesh 相对 Transform 逐项完全一致；没有运行时覆盖。Development Editor 构建成功，编辑器已关闭。错误 +90° 版本保存在 `Saved/Backups/VFXPackFirstPerson_BadRootPlus90_20260806_2010`。
- session204：修复 `/Game/Characters/CharacterBase/Animations/Legacy/VFXPackFirstPerson` 的基础方向。先在批准的外部资源项目 `FPSShooter1` 审计，确认 7 条 AnimSequence 的 root 首尾 Yaw 均约为 `-89.999977°`；在外部项目对 root 全采样关键帧烘焙逆时针 `+90°`，导出最终 FBX，再仅将最终动画迁入 TheManTest。目标项目全量冷审计 7/7 首尾 root Yaw=`0.0°`；Montage/BlendSpace 继续引用这些修正序列。TheManTestEditor Win64 Development 冷构建成功。外部备份：`FPSShooter1/Saved/Codex/Backups/VFXPackFirstPerson_20260806_194749`；目标备份：`Saved/Backups/VFXPackFirstPerson_20260806_195138`。
- session203：按用户动画架构撤销维修枪 FP/Body 双动画层分流。外部批准项目 `FPSShooter1` 已存在最终方向修正资产 `AS_VFXPack_FP_Idle/Run`，目标项目旧 `AS_Rifle_A_Idle/Run` 的全部引用已合并到该最终资产。删除零引用 `ABP_RepairGun_BodyAnimLayer`；两条 `RTG_W2_*` 仍被 MaintenanceWorker 全身 BlendSpace 引用，保留。C++ 删除 `BodyEquipmentAnimLayerClass`，`EquipmentAnimLayerClass` 与 Equip Montage 统一应用到 ArmsViewMesh 和 CharacterMesh0。PIE 确认两者主 AnimBP 均为 `ABP_CharacterBase_Body`、Linked Layer 均为 `ABP_RepairGun_AnimLayer`，root/pelvis/spine/双手组件空间旋转点积 0.99993~1.0；截图 `TMT_UnifiedVFXPackRepairLayer_PIE.png`。
- session202：撤销 session200/session201 的强行同点实验值，`BP_MaintenanceWorker` 恢复早上基线：HeadCamera `(0,-18.852108,77)`，ArmsViewMesh `(-18.107912,18.852108,-150.00795)` / Rotation `(-3,-15,-1)`，相反 Y 平移使手臂组件原点在角色局部横轴为 0，同时保留独立 viewmodel 的前后/高度构图。PIE 截图 `TMT_Final_MorningBaseline_NoTickTransform.png` 与桌面参考构图一致。运行时报告确认 CharacterMesh0、LegsMesh、ShadowBodyMesh、ShadowUpperBodyMesh 的世界位置/旋转完全相同。删除 OnConstruction/BeginPlay/Tick 中全部玩家组件 Transform 写入；Development Editor 冷构建成功，Tick 代码扫描无 SetRelative/SetWorld/SetActor/AddLocal/AddWorld。蓝图前/侧/顶截图为 `TMT_Final_BP_{Front,Side,Top}.png`，保存后关闭编辑器。
- session199：撤销 session198 的玩家蓝图三视图通过结论。最新用户截图确认上半身侧向；PIE 也证明旧 C++ 每帧覆盖蓝图 Transform。现删除 `OnConstruction/BeginPlay` 静态构图重写和 Tick 中 BodyRoot/ArmsViewMesh 基础 Transform 重写，运行时只在 BeginPlay 捕获的蓝图 Transform 上叠加冲刺/移动惯性；Development Editor 冷构建成功。尝试将第一人称 Arms 几何强行旋到身体 Yaw 后会导致第一人称消失或严重错位，实验值已撤销并恢复原构图。当前三视图明确未通过，等待 viewmodel/完整身体架构决策。
- session200：用户决定保留独立 Arms viewmodel，直接用蓝图 Transform 校正。`ArmsViewMesh` 当前蓝图值为 Location `(200,18.852108,-165)`、Yaw `-90°`，与身体/腿旋转基准一致；蓝图编译保存成功，PIE 截图 `TMT_IndependentArms_YawAligned_Raised_PIE.png` 已生成。C++ 仍只读取蓝图基线，不锁定该值；待用户前台确认位置与画面观感。
- session201：按用户要求继续自行审验三视图与 PIE。通过编辑器临时实例实际计算 Mesh Bounds，而非只看组件原点：最终蓝图 `HeadCamera=(-201.886,-121.126,77)`、`ArmsViewMesh=(200,120,-206.36)` / Yaw `-90°`，相反位移保留独立第一人称构图，同时让 Arms 与 CharacterMesh0 Bounds 中心误差收敛到 `X=-0.0001cm / Y=0.0003cm / Z=-0.0203cm`。PIE 报告确认 Arms 与身体世界 X/Y 同点、Yaw 一致，截图 `TMT_Player_BPAndPIEAligned_Final.png`；蓝图编译保存成功。仍等待用户前台最终观感确认，不提前关闭功能。
- session198：按用户要求统一 BP 编辑器与 PIE 的玩家手臂/影子轴线。根因是蓝图仍序列化旧 ShadowBody/ShadowUpperBody，而 BeginPlay 运行时清空，造成编辑器与游戏显示不同；现将两个弃用影子组件资产清空。保留 ArmsViewMesh 的前后/高度构图偏移，把其横向 `+18.852108` 用 HeadCamera 横向 `-18.852108` 精确抵消，世界横向误差冷读回=`0.000000cm`；BodyRoot 归零，LegsMesh 与 CharacterMesh0 均为 `(0,0,-90)` / Yaw `-90°`，脚底/下半身与唯一完整影子同点。Development Editor 冷构建与 Player Framing/UpperBody 自动化通过；蓝图组件三视图证据保存于 `Saved/Screenshots/WindowsEditor/TMT_BP_ThreeView_{Front,Side,Top}.png`。
- session197：按用户反馈重新做可见验收。人物在真实关卡编辑器视口中仅显示权威身体，并绘制红色 Actor `+X` 与绿色 Mesh 视觉 `+Y` 两支调试箭头；截图 `TMT_Player_Model_WithAlignedArrows.png` 中两箭头同向平行，编辑器计算点积=`1.0`，临时验证 Actor 随后删除。FlyingBug 旧三组左右腿对会让每排两腿同步抬起，形成机械摇摆；现改为标准交叉三足：左前+右中+左后 Phase0，右前+左中+右后 Phase0.5。18秒平地四张连续时相人工复核为两组三角支撑交替，`LocomotorCrawlEvidence` 与 `LocomotorSlopeEvidence` Success，人物 `UpperBodyEvidence` Success。
- session196：用户通过编辑器箭头发现 session195 玩家身体归零仍错误；该轮验收撤销。身体资产视觉前方实际为局部 `+Y`，正确组件修正是蓝色 Z/Yaw `-90°`。同时发现 Python `unreal.Rotator(0,-90,0)` 写到了绿色 Pitch，现改为 `unreal.Rotator(0,0,-90)` 并在真实 Editor 中冷读回 Mesh/Shadow/Legs 蓝色 Yaw 均为 `-90°`。自动化也从错误的 Mesh `ForwardVector` 改为核对 Mesh `RightVector`（局部 `+Y`）与 Actor 前向箭头，第一次在错误 Pitch 下真实 Fail，修正后 `UpperBodyEvidence` Success；`FramingCapture` Success。
- session195：撤销 session194 对“朝向已通过”的结论。骨骼参考姿势审计确认 FlyingBug2 的视觉前方为局部 `+Y`，将地表朝向从 `MakeFromXZ` 改为 `MakeFromYZ`，并新增“Mesh 局部 +Y 与实际位移方向点积 > 0.9”的运行时断言。玩家侧不再把第一人称 RepairGun Layer 同时强加到完整身体：`ArmsViewMesh` 保留第一人称层，`CharacterMesh0` 改用 `ABP_RepairGun_BodyAnimLayer` 的第三人称 Idle/Run；身体、影子、腿组件统一为零相对旋转。实际 Unreal Editor 内打开 `BP_MaintenanceWorker` 冷读回三个身体组件 Rotation=0；运行时身体正向、影子及 FlyingBug 爬行截图人工复核。Development Editor 冷构建、Crawl、Slope、UpperBodyEvidence、FramingCapture 均 Success。
- session194：修正旧验收只看足端数值、未保证整条前腿可见摆动的缺口。前腿 FBIK PositionAlpha 由0.2改为1.0；四张连续时相图人工确认两条前腿整链明显切换支撑/摆动姿态。平地与坡地自动化均 Success，Development Editor 冷构建 Success。同轮清除 shadow-only 枪体的第一人称相机空间旋转，改由 CharacterMesh0 GripPoint 决定投影方向；UpperBodyEvidence Success，最终截图已复核。

- 参考用户教程截图，确认正确结构为四个 Foot Set → 八个 FeetTransform → 八个 FullBodyIK Effector。
- 已将误配的头部 `tent_low*` 清除，改为六条真实接地腿的两组三足组，PhaseOffset=`0/0.5`；Control Rig 重编译保存成功。
- 自动化已升级为八条腿逐项检查 component/Rig 位移与抬落范围，并输出四张连续相位截图。
- session188 的八足结论作废：`tent_low*` 实为头部触须，旧自动化只证明头部骨骼移动。
- 已按参考姿势全局高度确认六条真实接地腿，建立两组三足交替 Foot Set，并将 FullBodyIK 收敛为六个对应 Effector；冷启动回读无断链，平地逐腿与坡地自动化均 Success。日志为 `SixLegGaitColdRound1.log`、`SixLegSlopeColdRound1.log`。
- 已按用户最新截图创建 `ABP_NightmareFlyingBug2_WalkLocomotor`：`Anim_Nightmare_bug2_walk1 -> Control Rig -> Output Pose`。移除外置 ControlRigComponent 整骨架覆盖；平地验证六足继续抬落且头部/触须累计运动 208.8cm，坡地验证 Success。日志 `WalkSourcePoseColdRound2.log`、`WalkSourcePoseSlopeColdRound1.log`。
- DebugGame 编辑器关闭后，六个主弯曲关节的 FBIK AngularStiffness 已成功写盘；六个 Effector 的 RotationAlpha 改为 0，仅由 Locomotor 修正位置并保留原 Walk 的尖足朝向。平地与坡地冷启动均 Success，日志 `WalkBlendPositionOnlyCold1.log`、`WalkBlendPositionOnlySlopeCold1.log`。
- session191：撤销“旧自动化成功即姿态正确”的结论。严格审查确认旧链最低仅 2/6 低位、六足高差 162cm。专用 AnimBP 改为原 Walk 头身循环 + 六腿接地帧分层，再进入 Locomotor；前支撑对 FBIK PositionAlpha=0.2，后四腿=1.0，步高4、空中占比0.22、骨盆 BobOffset=-35。最终两轮平地 Success（最低3/6低位、最大高差约56cm、头触须运动约204cm），坡地 Success；亮场截图已人工复核，待用户前台确认。
- session192：按用户确认的六条可接地腿，将两组三足改为三组左右腿对：前/中/后 Phase=`0/0.333/0.667`；保留六个 At 与六个 FBIK Effector 一一对应。Stepping 按教程截图恢复 `PercentOfStrideInAir=0.35`、`StepHeight=6`、`MaxCollisionHeight=1`。冷写盘与独立回读成功；平地两轮均为6/6低位、最大高差25.4/25.5cm，坡地 Success。Control Rig 初始预览六足 Z=-3.73~5.31cm，三对左右展开，无错误悬空基础姿势。
- session193：依据引擎 Locomotor 公式反复校准位移与步频。原设置在120cm/s为3.43 cycles/s；0.633与1.129两版分别因约190/106cm步幅导致中后腿拖地而否决。最终采用 `PhaseSpeedMin=0.8`、`PhaseSpeedMax=2.1`、`MinimumStepLength=12`，运行时约1.852 cycles/s、65cm步幅。平地连续两轮与坡地均 Success；六足独立抬落，中后足垂直范围3.2~3.6cm、前足29.7~31.3cm，最低6/6低位支撑，最大高差25.8cm，原Walk头颈混合保持运动。

## 本轮完成

- session186：纠正用户截图中的两项真实回归。FlyingBug2 现由运行时 ControlRigComponent 执行 Locomotor + FullBodyIK，自动化逐帧证明最终八足骨骼运动；三轮起伏路线均 Success。玩家影子收敛为唯一完整动画宿主 CharacterMesh0，清空重复 ShadowBody/ShadowUpperBody；VFXPack `Amount (S)` 曲线新增运行时逐帧读回断言。Development Editor 冷构建成功，Player 影子/VFX/Framing 最终两轮各 3/3 Success。FEAT-075 已归档。

- session185：纠正 session184 错误视觉验收。FlyingBug2 `CharacterMesh0` 从错误 `Pitch=-90°` 恢复为 `0°`，冷回读成功；新增 Mesh Up/Actor Up 方向断言，Development Editor 编译及平地、18°单坡测试 Success。用户要求继续建立明显起伏验证区、修复玩家影子动画并恢复与 VFXPack 一致的枪械出现效果，多轮实际画面通过前不得关闭功能。
- session185：`TestMap` 已建立七段连续起伏验证区；修复倾斜碰撞胶囊卡坡后，FlyingBug2 三轮冷启动连续爬行均 Success（约 2102~2105cm/18s）。影子改为完整身体单一来源并禁用错误 ShadowUpperBody；Viewmodel 枪体停投影，新增身体 GripPoint shadow-only 枪体。VFXPack 出现效果改为先写 `Amount (S)=1` 再显现，保留 0.5s 原曲线。Shadow/Viewmodel 各三轮 Success；待用户前台实际观感复核，当前仍为 in_progress。

- session182：玩家四个 Mesh 统一为 `ABP_CharacterBase_Body_C`，装备层向全部 Mesh 链接；Shadow/Legs Leader 正确。初始装备后首帧预评估消除枪下压。装备显现按 VFXPack 精确恢复 `Amount (S)` 0.5 秒 cubic Hermite 1→0（切线 -5.434987）。删除无引用旧 `ABP_CharacterBase`。
- session184：修正 FEAT-075：Nightmare 改为 `MOVE_Walking` + Locomotor/FullBodyIK 八足贴地爬行，平地与 18° 坡面自动化通过；Enemy 目录统一且旧目录/Redirector 清零；玩家 ShadowUpperBody Leader Pose 同步 Arms 最终 Pose，运行时骨骼一致性与截图验证通过。
- session182：Development Editor 冷构建成功；五个目标蓝图编译保存；四个目标目录资产验证全部有效；`TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success；冷启动日志未发现 Blueprint/Linker/Ensure/Accessed None/Invalid material index 错误。

- session181：修复 session179/180 未通过动画验证造成的回归。完整原版 VFXPack AnimBP 和 12 个依赖资产已从检查点恢复并用 AssetTools 迁入 CharacterBase；最终 ABP 保留完整原父类/AnimGraph，不再继承空模板。`ArmsViewMesh` 已重新绑定正式 Mesh 与 `ABP_CharacterBase_C`。普通 PIE 与冷重启 PIE 均用真实 A 键验证 `Is_Moving=True`、Speed 550/750 且双手为动态非参考 Pose；游戏视口截图确认枪和持枪手恢复。`TABP_CharacterBase` 目前仅预留，不在运行链。
- session180：FramingCapture 仅验证了构图，未能发现 AnimGraph 被剥离，原“模板链正确”结论已由 session181 撤销；装备溶解跳过空 helper mesh 的 C++ 修复及完整构建结果仍有效。
- session179：将 MaintenanceWorker 下暂时公用的 Body、第一人称 Mesh/材质/纹理、Body/第一人称动画和旧 VFXPack 参考动画统一迁到 `/Game/Characters/CharacterBase`；新增 `UCharacterBaseAnimInstance` 强类型承载原 VFXPack 五个变量，创建无骨架 `TABP_CharacterBase` 与最终骨架子类 `ABP_CharacterBase`，MaintenanceWorker 已引用最终子类。旧 `FPSArmsAnimInstance` 源码原先已不存在，本轮删除最后的 CoreRedirect；仍被 `TABP_BodyLocomotion` 使用的 `UFPSCharacterAnimInstance` 保留。Development Editor / Win64 构建成功。
- session178：开局/切枪不再播放手臂 Equip Montage，装备入口改为 C++ 固定的 VFXPack 枪体材质溶解（`Amount (S)`、0.45 秒、1→-1），不暴露蓝图参数；冲刺压枪默认改为 -6°。Development Editor / Win64 构建成功。
- session177：冲刺压枪终点新增蓝图参数 `SprintViewmodelPitchDegrees`（`Viewmodel|Sprint`，默认 -12.5°）；确认开局下压来自现有 Rifle Equip Montage 的自动播放，本轮未改装备动画。
- session176：按用户最新截图将 Viewmodel Movement Lag 正式默认值同步为左右 2.4cm、前后 1.4cm、跟随 8、回弹 10；保留用户当前 BP_MaintenanceWorker 资产设置。
- session175：新增轻量且可调的 WASD 枪械位置滞后，只作用于 ArmsViewMesh，默认左右 1.2cm、前后 0.8cm、跟随 8、回弹 16；两轴独立回弹，不改变原版冲刺旋转枢轴、角度或相机。Development Editor / Win64 构建成功。
- session174：按用户要求进行实际 PIE 截图对比，并通过分别隐藏 ArmsViewMesh/LegsMesh 确认冲刺中央遮挡来自第一人称前臂。根因是当前把原 SK_ArmMesh 的位置偏移放到了旋转节点 ViewmodelRoot，旋转枢轴与原版 `FPS_Camera -> BodyRotator(原点) -> SK_ArmMesh(偏移)` 不一致。现已让 ViewmodelRoot 回到相机原点、ArmsViewMesh 持有原位置/轴向偏移；Idle 构图不变，Shift 仍用原版 0.2s / -12.5°，修复后截图中央视野无前臂遮挡。Development Editor / Win64 构建成功。
- session173：纠正对用户“手臂挡枪/挡视野实现要与原版一致”的误解。撤销 session172 自定义 -25° 终点及 `SprintViewmodelPitchDegrees`，恢复原版 `BodyRotator` 精确 0.2s / Pitch -12.5°。后续只核对原版手臂 Pose、GripPoint 武器挂点与正常深度遮挡关系，不用加大角度伪修复。
- session172：按用户前台反馈加大 Shift 冲刺收枪角度，使前臂和枪退出中央视野。新增蓝图可调 `SprintViewmodelPitchDegrees`，默认由 -12.5° 加大为 -25°；原 0.2s 可逆过渡、550→750 速度与无横移设计不变。Development Editor / Win64 完整编译链接成功。
- session171：按原 VFXPack 实机 1280×720 截图重新对比 Idle/按 Shift+W 冲刺。相机恢复原版 HeadCamera `FieldOfView=77°`，保留原 `BodyRotator` 0.2s / Pitch -12.5°。另定位 RepairGun 漏复制原武器父节点 `RootOffset.Y=+11.660166`：原版网格相对 GripPoint 最终为 `(-0.000656,-5.097503,3.554176)`，而当前误为 `(0,-16.757669,3.554176)`。已将 `BP_RepairGun.StaticMesh` 恢复到原版最终变换并冷回读；Development Editor 编译成功。
- session170：找到原版枪械倾斜明显、当前项目几乎无反应的真正根因：迁移 AnimBP 时删除了对示例 `FirstPersonCharacter` 的硬 Cast EventGraph，C++ 只复制了 `PlayerLeanAmount/PlayerLookUpAmount`，遗漏原 EventGraph 随后使用的 `Lean_Sides_Offset=8.0` 与 `Look_Up_Offset=2.0`。现按原顺序在写入 AnimBP 前精确恢复 `Side×8` / `LookUp×2`，不添加任何组件位移或自创旋转。运行时探针实测 RepairGun A/D 前后旋转差由约 2.05° 恢复到约 10.24°，`hand_r` 由约 2.15° 恢复到约 6.61°。Development Editor 编译成功。
- session169：按用户要求撤销 session167–168 所有 A/D 可见性补偿。删除 `VFXMovementWeaponRollDegrees` 及任何 `ViewmodelRoot` / `ArmsViewMesh` 方向 Roll；普通移动严格只走原 Body Sway 变量和原 AnimBP `spine_03/hand_l` Modify Bone 链。`ViewmodelRoot` 只复刻原 `BodyRotator` 冲刺 0.2s / Pitch -12.5°，无位置变化。Development Editor 完整编译链接成功。
- session168：用户指出 session167 的 `ViewmodelRoot` Roll 仍然表现为横向平移。确认原因是该枢轴位于相机子级根部，旋转半径过大。现已将 A/D 可见性补偿的 ±6° Roll 移到 `ArmsViewMesh` 的骨架/胸口附近枢轴；`ViewmodelRoot` 只保留原版冲刺 0→-12.5° Pitch，位置始终固定。Development Editor 完整编译链接成功。
- session167：用户前台确认原 AnimBP 骨骼倾斜仍无可见旋转。原因是实际定向修正只有 `spine_03=±1°` 和 `hand_l=±0.5°`，在 RepairGun/110° FOV 构图下不可辨识。保留原 AnimBP 链的同时，现将同一 `CurrentVFXLeanSides` 以最大 ±6° 纯 Roll 应用到 `ViewmodelRoot`，与冲刺 0→-12.5° Pitch 合并；仍无任何横向位移。Development Editor / Win64 完整编译链接成功。
- session166：按用户确认重新深查 VFXPack 原角色蓝图。恢复完整 Body Sway 目标：`Clamp(MoveRight+MouseX,-1,1)` 与 `Clamp(-MoveForward-10×LookUp,-1,1)`，仍由 AnimBP 的 `spine_03/hand_l` Additive Modify Bone 负责普通移动与视角摆动。冲刺改回按键意图驱动的 0.2s 可逆过渡，同步插值速度 550→750，并在 `BodyRotator` 等价枢轴 `ViewmodelRoot` 上整体压枪 Pitch 0→-12.5°；不再依赖实际速度阈值，不再错写入脊柱单骨，且保持无 5cm 横向平移。同时将 `BP_MaintenanceWorker` 序列化的 HeadCamera FOV 从 100°改为 110°。Development Editor / Win64 编译成功，FramingCapture 1/1 Success，待前台 PIE 观感复核。
- session165：按用户最新决定取消额外的 5cm `ViewmodelRoot` Y 向横移；第一人称构图根节点固定回到 authored Transform，A/D 只保留原版 AnimBP 的 `spine_03/hand_l` Additive Roll。相机基础 FOV 改回 110°，删除 BeginPlay 硬覆盖，后续直接使用蓝图 Camera 组件原生 Field Of View。原输入 Clamp、2/8 插值与回正逻辑不变；Development Editor / Win64 完整编译链接成功，当前编辑器需重启后进行前台观感复核。

- session164：按用户要求将原版旋转职责归还 AnimBP。C++ 不再直接叠加 A/D Roll 或冲刺 Pitch；A/D 继续只写 `Lean_Sides_Amount`，冲刺按实际速度计算后并入 `Look_Up_Amount`（最多 -12.5°），由原 AnimBP 的 `spine_03 Modify Bone` 执行。此前用户认可的附加效果改为 C++ 最后叠加最多 5cm 的纯 Y 向左右位置偏移，不再通过远轴心 Roll 制造假平移。

- session163：修正 session162 只切换 Body Sway/CameraShake、未恢复真正冲刺压枪的问题。原版 `BodyRotator Timeline_2` 已确认使用 `RLerp(A=Identity, B=Pitch -12.5°)`；现将 `-12.5° * SprintVisualAlpha` 叠到 `ViewmodelRoot` Pitch，按实际水平速度在 WalkSpeed 550 到 SprintSpeed 750 间连续压低/抬回。左右移动原骨骼链及额外 2° Roll 保留。

- session162：再次核对原版侧倾节点：`spine_03` 与 `hand_l` 均为 Additive Roll，`RotationSpace` 未覆盖、采用 Modify Bone 默认 Component Space；枪随 `hand_r` 后代链绕脊柱轴心旋转。保留该原链及当前 2° `ViewmodelRoot` 补偿。冲刺视觉从 Shift 意图状态改为实际水平速度驱动：`WalkSpeed..SprintSpeed` 连续映射 0..1，Body Sway 插值速度由 2 连续过渡到 8，Running CameraShake 在速度比例达到 0.5 后切换；原地按 Shift 不再触发冲刺视觉。

- session161：按用户前台观感，将额外 `ViewmodelRoot` 最大移动 Roll 从 1° 微调为 2°；原 AnimBP 骨骼修正、2/8 插值速度与回正逻辑均不变。

- session160：重新核查原版完整侧倾来源。原 AnimBP 的定向 A/D 修正为 `spine_03` Roll 最大 1°、`hand_l` 再取 0.5 倍；Walking CameraShake Roll 振幅 0.2° 且调用 Scale=0.5（实际约 0.1°），Running CameraShake Roll=0°；`BodyRotator` 是冲刺/收枪过渡，Run 动画摆动不区分左右方向。因此撤销过强的自定义幅度，将额外 `ViewmodelRoot` Roll 从 3° 收敛为 1°，只用于补足当前构图下原骨骼侧倾的可见性。

- session159：用户前台确认 6° 左右移动枪械倾斜过强；最大 `ViewmodelRoot` Roll 收敛为 3°，插值速度、松键回正与原 AnimBP 1° 骨骼修正保持不变。待关闭编辑器后冷构建复核。

- session158：PIE 实测确认 `Lean_Sides_Amount` 在 A 输入时达到 `-1.0`，`spine_03/hand_l/hand_r` Pose 均发生变化，证明原 AnimBP 链路有效；但原骨骼 Roll 最大仅 1°，在当前 RepairGun/FOV 构图下肉眼不可辨。保留原骨骼修正，并新增同一平滑侧移量驱动 `ViewmodelRoot` Roll（满输入 6°），使左右移动枪械倾斜清楚可见。C++ 编译通过，冷构建仅因运行中的 UnrealEditor 锁定 DLL 而在链接阶段失败，需关闭编辑器后重跑并前台复核。

- 修复 VFXPack 移动倾斜驱动：不再用 CharacterMovement 速度归一化近似输入，改为缓存 Enhanced Input 的原始 A/D/W/S 轴，普通移动/冲刺继续按原版 2/8 插值，Completed/Canceled 后平滑回正。
- 修正前后倾斜写入名：`Look_Up_Down_Amount` 改为原版 AnimBP 实际读取的 `Look_Up_Amount`。
- 只读导出正式 `ABP_MaintenanceWorker_FirstPerson_Original`，确认 `spine_03` Additive Roll/Pitch 与 `hand_l` 0.5× Roll Modify Bone 均保留且连接；Development Editor / Win64 冷构建成功。待用户前台确认 A/D/W/S 可见倾斜。

- 第一阶段 HeadBob、Viewmodel sway 与 RepairGun CameraShake 已完成并保存在检查点 `b7ff993`。
- FPSShooter1 中完成 68 骨逐骨/参考姿势一致性验证；无 IK 重定向，VFXPack Skeleton 已成为 MaintenanceWorker 身体、腿与动画统一 Skeleton。
- RepairGun 不再保留原生 Walk；2D BlendSpace 精确复刻 VFXPack 的 Idle/Run 混合曲线（Speed 0/280/420/700，RateScale 0.8/0.5/1.0/1.5），按三个 Direction 档共 12 个有效样本。
- 撤销 session145 的错误动画验收结论：用户截图确认 ArmsViewMesh 为完整 T-Pose。
- 已定位根因是 `ABP_MaintenanceWorker + RepairGun Linked Anim Layer` 对 ArmsViewMesh 的最终输出；同 Mesh 手动播 Idle、以及直挂 VFXPack 原版 AnimBP 均能正常弯臂。
- 已将精简后的原版状态机整理为 `ABP_VFXPack_FirstPerson` 并持久化到 ArmsViewMesh；冷启动 PIE 的 Idle 与 420 cm/s 移动状态均脱离 T-Pose，正式资产验证通过。
- 供应商目录已清理；删除时出现一次 handled ensure，最终仍需冷重启日志复核。
- 第一人称构图按原 VFXPack 拾枪截图重新校准：`ViewmodelOffsetLocation` 从错误的 `(302.4,100,-210)` 调整为 `(100,75,-200)`；Idle/Run 均位于右下持枪区域。
- 影子已恢复非 T-Pose 持枪姿势；CharacterMesh0 继续使用主 ABP，ShadowBodyMesh/LegsMesh 正确跟随 CharacterMesh0，第一人称 ArmsViewMesh 独立使用 VFX AnimBP。
- 撤销 session147 的错误影子判断；用户截图与骨骼采样证实当时仍是 T-Pose。
- 已定位并修复 `TABP_BodyLocomotion` 的故障 WeaponUpperBody/AimOffset 覆盖链；身体改由稳定的 Locomotion Pose 直接进入 UpperBodySlot，Idle/420 cm/s Run 骨骼值与截图均不再 T-Pose。
- 第一人称 ArmsViewMesh 跨帧手骨变化且 Run 时 `Is_Moving=True`，确认 VFX AnimBP 确实在播放。
- 临时 Retarget、TempCharacter 与供应商目录已清场；四个相关蓝图编译保存成功，Development Editor 编译成功。

## 待办

- 冷重启编辑器后再次复核 Idle/Run、蓝图编译和日志，确认供应商目录删除时的 handled ensure 没有留下损坏引用。
- 前台实际输入复核走/跑/跳/开火观感；当前自动化已用持续 420 cm/s 速度验证 `Is_Moving=True`，但仍需真实 Enhanced Input 复核。
- 确认后决定是否把 VFXPack Jump/Fire/Recoil 继续接到现有状态机/蒙太奇；当前资产已整理但未强行覆盖本项目 RepairGun 装备/开火动作。
- VFX 动画按素材包枪型制作，RepairGun 前握把与左手尚未完全贴合；后续若要求精确贴握，需要外部项目制作专属最终动画或另行确认程序化左手 IK。

## 2026-08-02 session152 交接

- RepairGun 已换为 VFXPack `BP_Weapon_Rifle_Physical_01_Child` 使用的 Rifle 01 静态枪模，资产已归档到 `/Game/Weapons/RepairGun/`；旧骨骼枪模已清空隐藏。
- 第一人称 ArmsViewMesh 直接使用 `ABP_VFXPack_FirstPerson`，恢复原版上半身 Idle/Run 姿态与速度；下半身速度未调整。
- 最终 Viewmodel 为 `Location=(-18.107912,41,-150.00795)`、`Rotation=(-3,-15,-1)`；确定性 1920×1080 截图为 `Saved/Screenshots/PlayerFramingCurrent.png`。
- 静态枪模枪口回退已落入 `AFirearm::GetMuzzleWorldTransform()`；Development Editor / Win64 冷构建与 `TheManTest.Player.Viewmodel.FramingCapture` 均成功。

## 2026-08-02 session153 交接

- 已停用鼠标旋转枪械滞后、自创方向移动偏移与 gameplay 相机走跑 CameraShake；VFXPack 动画负责主要姿态，C++ 原参数波形只作用 ViewmodelRoot。
- Rifle Outline 作为原版附加描边壳与实体枪组合使用，不再单独替代实体枪。
- 影子上半身根因已修：ShadowBodyMesh Leader=ArmsViewMesh；运行时 spine_03/hand_r/hand_l 组件空间 Pose 完全一致。LegsMesh 仍跟随 CharacterMesh0，下半身速度未调整。
- 冷编译和 FramingCapture 均成功；最终截图 `Saved/Screenshots/PlayerFramingCurrent.png`。

## 工作区边界

## 2026-08-02 session154 交接

- session153 的自创 Viewmodel 波形与 `ShadowBodyMesh=ArmsViewMesh` 结论均已撤销；影子和腿恢复跟随 `CharacterMesh0`，修复第一人称视野中全身错位及影子上半身朝向异常。
- 移动反馈直接复用 VFXPack 原版 Walking/Running CameraShake 资产与原蓝图调用语义；C++ 只代替原蓝图内部状态更新，不强行替代资产。
- `ABP_VFXPack_FirstPerson` 由真实 Velocity 驱动 `Is_Moving/Is_InAir/Character_Speed`；MaintenanceWorker 运行值为 Walk 550、Sprint 750、Acceleration 2000、Braking 750。
- 真实 W 输入已重复验证 Idle→Run→Idle，长按期间手骨 Pose 持续变化；冷构建、`git diff --check`、UTF-8 JSON 解析与 FramingCapture 第 3 次运行均成功。
- 动态验证截图：`Saved/Screenshots/WindowsEditor/TMT_ExactVFX_Walk.png`。仍需用户在自己的前台游戏窗口按参考图主观确认运动节奏与最终画面对齐。

## 2026-08-02 session155 交接

- 第一人称现已直接运行原 VFXPack `FirstPerson_AnimBP` 的完整 AnimGraph/状态机与原动画资产，不再使用此前重建版；仅删除会拖入整套示例工程的 EventGraph Cast，三个驱动变量由 C++ 等价写入。
- 原版手臂 Mesh/Skeleton/Physics/材质和动画共 13 个最终资产已整理到 `/Game/Characters/MaintenanceWorker/FirstPerson/`，供应商目录冷重启后确认不存在。
- 原版武器插槽精确名称已修正为 `GripPoint`，自动化直接检查 Socket 存在、装备声明和实际挂载三项。
- 冷构建及冷启动 FramingCapture 1/1 成功；真实移动时原版 AnimClass、550 速度与三个状态变量已回读。最新动态截图：`Saved/Screenshots/WindowsEditor/TMT_OriginalAnimBP_Walk_Cold.png`。

## 2026-08-02 session156 交接

- `CharacterMesh0` 与 `ArmsViewMesh` 已恢复使用同一个原版 VFXPack AnimBP；Shadow/Legs 继续跟随 CharacterMesh0，修复影子上半身与第一人称姿态分叉。
- 原版 `Walk_Run_1D` 确认为 1D 速度轴；左右偏移不是 BlendSpace Direction 轴，而是 `Body_Sway -> Lean_Sides_Amount`。已按原参数恢复：侧移 Clamp `[-1,1]`，Walk 插值 2、Sprint 插值 8；不恢复 MouseX 枪械滞后。
- PIE A/D 实测 Lean 分别为 `+0.9782/-0.9782`，两个 AnimInstance 的 AnimClass、速度和 hand_r Pose 一致；Shadow Leader=`CharacterMesh0`。
- 冷构建、FramingCapture 1/1、蓝图编译和全部资产保存完成。截图：`TMT_VFXPack_StrafeRight.png`、`TMT_VFXPack_StrafeLeft.png`。


- FEAT-073 安全检查点：`34fbfaf`。
- 本轮修改 MaintenanceWorker 最终 Skeleton/身体动画、RepairGun 第一人称 locomotion、既有 Camera/Viewmodel/开火反馈与 Harness；不包含 IK Rig/Retargeter 或供应商工作目录。
## 2026-08-01 session149

- 撤销第一人称独立 AnimBP 与身体绕过武器层的临时方案；CharacterMesh0/ArmsViewMesh 重新统一使用 ABP_MaintenanceWorker，并在两边链接同一 RepairGun Anim Layer。
- 修复 TABP_BodyLocomotion 的 DefaultSlot 基础 Pose 断线；RepairGun Idle 指定 AS_Rifle_A_Idle，WalkRun 改为直接播放已验证有效的 AS_Rifle_A_Run（0.5×），避免 BlendSpace Player 运行时返回参考姿势。
- PIE 真实 W 输入验证：主 ABP 与 Linked Layer 两边 Speed=100；第一人称、身体、影子均为有效 Run 持枪 Pose。Idle 时三者 hand_r 组件空间 Pose 完全一致。
- 截图：TMT_UnifiedABP_Idle_Final.png、TMT_UnifiedABP_RunSequence.png。

## 2026-08-02 session150

- 撤销 `(100,75,-200)` 已匹配 VFXPack 的错误构图结论；此前截图只证明动画有效，未证明视角一致。
- 以 110° FOV、1920×1080 的确定性 SceneCapture 与 VFXPack 参考图逐项对照，四轮校准后采用 `ViewmodelOffsetLocation=(90,80,-185)`、`ViewmodelOffsetRotation=(0,-13,0)`。
- 当前枪口位于参考图对应的中心偏右区域，枪身轴线为左上到右下；RepairGun 比参考长步枪短，不能用相同轮廓长度作为验收条件。
- Live Coding、Development Editor / Win64 构建和 `TheManTest.Player.Viewmodel.FramingCapture` 自动化均成功；截图为 `Saved/Screenshots/PlayerFramingCurrent.png` 与 `TMT_VFXPack_Reframed_Idle.png`。

## 2026-08-02 session151

- 用户指定桌面 `微信图片_20260802100122_109_52.png` 和原项目 `UE389_MuzzleSource/.../VFXPack` 为唯一正确参考；session150 的参考图与 `(90,80,-185)` 结论作废。
- 按正确 1059×597 参考图量化枪口目标 `(58.5%,58.1%)`，使用同宽高比的 1920×1080 SceneCapture 迭代。
- 最终采用 `ViewmodelOffsetLocation=(0,41,-155)`、`ViewmodelOffsetRotation=(0,-13,0)`；实测枪口约 `(57.9%,57.2%)`，RepairGun 主体按参考尺度延伸并裁出右下边界。
- 确定性截图为 `Saved/Screenshots/PlayerFramingCurrent.png`；普通 PIE 截图 `TMT_CorrectVFXReference_Idle.png` 因当前内嵌面板为 1567×428 超宽比例，仅用于可见性检查，不参与 16:9 构图验收。
- 自动化断言已同步；Live Coding、FramingCapture、Development Editor / Win64 冷构建均成功。重启后 BP_MaintenanceWorker 资产验证与最终 Transform 冷回读通过。
## 2026-08-21 session208

- MaintenanceWorker 第一人称视图模型恢复 C++ 驱动：`BaseArmsRotation=(-3,-90,-1)`，奔跑以 0.2 秒过渡将 `ViewmodelRoot` Pitch 压至 -6°。
- 按用户最新要求暂不加入 WASD 位置滞后；相关运行逻辑、可调参数和缓存状态均已移除，`ArmsViewMesh` 保持 C++ 静态构图位置。
- 直接冷读原 VFXPack `FirstPerson_AnimBP` CDO 与图表确认两组数值职责不同：Walk/Sprint 的 `2/8` 是输入插值速度，而 `Lean_Sides_Offset=8`、`Look_Up_Offset=2` 是 Modify Bone 前的正式输出倍率。现恢复原版倍率，并把源组件 Yaw `-15°` 到当前 `-90°` 的 75° 基差映射到 Roll/Pitch；不叠加组件位移或侧移 Roll。
- 纠正本 session 对“统一架构”的错误理解：不能让 `CharacterMesh0` 直接运行纯第一人称 `ABP_VFXPack_FirstPerson`，否则完整身体的 Idle/Run/Jump 状态机消失。已从 Git 基线原样恢复误删的 `ABP_CharacterBase_Body`、`TABP_BodyLocomotion`、`BS_RunWalk_MaintenanceWorker` 与身体 Idle/Jump 序列；`CharacterMesh0`、`ArmsViewMesh` 均恢复运行 `ABP_CharacterBase_Body_C`，并各自通过现有 `ALI_WeaponAnim` 链接 `ABP_RepairGun_AnimLayer_C`，`LegsMesh` Leader 跟随身体。原 VFXPack 的 `spine_03` 组件空间 Additive Roll/Pitch 与 `hand_l` 半倍率 Roll 已接到 Body 主 AnimGraph 最终输出端（武器接口层之后）。PIE W/A/D 实测速度550，两个主实例与 Linked Layer 均同步；A/D 的 Lean/Look 分别为约 `-2.07/-7.73` 与 `+1.97/+7.35`，确认旋转方向翻转且主图实际收到驱动。
- 直接只读导出用户指定的原版 VFXPack 工程后确认：原 SK_ArmMesh Yaw=-15°，当前以动画 root=0° + ArmsViewMesh Yaw=-90°承担最终方向。为保持当前静态朝向且复现原版 Component Space 骨骼动态，C++ 将 Lean Roll / Look Pitch 按 75°坐标基差换轴后写入 AnimBP；组件位置保持不变。
- PIE 截图与运行时探针最终定位明显横移根因：`ArmsViewMesh` 被 BP 错绑为全身 `ABP_CharacterBase_Body_C`，A/D 时播放身体 Locomotion 导致整套手臂/枪明显换位。现恢复为原版第一人称 `ABP_VFXPack_FirstPerson_C`；复测 A 输入时 HeadCamera/ViewmodelRoot/ArmsViewMesh Transform 全部稳定，运行 AnimClass 与 Lean/Look 输入正确。证据截图：`TMT_Viewmodel_Idle_Audit.png`、`TMT_Viewmodel_Left_Audit.png`（修复前）及 `TMT_Viewmodel_Idle_Fixed.png`、`TMT_Viewmodel_Left_Fixed.png`（修复后）。
- `BP_MaintenanceWorker` 的 framing、sprint 参数以及 HeadCamera/ViewmodelRoot/ArmsViewMesh Transform 已执行 Reset to Default，蓝图继续只保留 Mesh/AnimBP 等资产配置。
- `TheManTestEditor Win64 Development` 完整构建成功；MaintenanceWorker 蓝图编译保存成功，冷回读 Transform 与 C++ 默认值一致。待用户前台 PIE 主观确认移动幅度和奔跑压枪观感。
## 2026-09-01 session252 交接

- 外部 VFXPack 跳跃为 JumpStart/JumpLoop/JumpEnd 三段骨骼动画加状态机混合；本项目对应 `AS_MaintenanceWorker_FP_*` 三条正式资产已存在并被维修工第一人称 AnimBP 引用，无需迁移或重定向。
- 已修复 `TABP_CharacterBase_FirstPerson`：JumpStart Sequence Player 从 Loop=true 改为 false；JumpLoop=true、JumpEnd=false 保持正确。模板与维修工子 AnimBP 已在 UE 内编译保存，原自动过渡警告消失。
- 待用户前台 PIE 复核跳跃、坠落和落地观感。

## 2026-09-01 session253 交接

- 根因补充：角色第一人称 Jump 状态机可用，但 RepairGun `WeaponUpperBody` 仅输出 Idle/WalkRun，覆盖了宿主持枪 Jump Pose。
- `TABP_FirstPersonFirearmBase.WeaponUpperBody` 现由 `bIsFalling` 选择：地面武器状态机、空中 `UpperBodyInPose`；RepairGun 子层继承。UE 编译保存与 Development Editor / Win64 构建成功。
- 待用户前台 PIE 验证 JumpStart、持续坠落 JumpLoop 和落地 JumpEnd 是否完整可见。

## 2026-09-01 session254 交接

- 已删除一次性编辑器工具中重复的宿主 AnimGraph 空中旁路生成代码与旧 fallback 日志；当前武器基类资产内的正式空中分支保持不变。
- 残留关键字扫描为0，Development Editor / Win64 构建成功。待用户前台 PIE 复核跳跃表现。

## 2026-09-01 session255-256 交接

- 错误删除宿主顶层空中 Blend 曾导致整个上半身无动作，已从 `3be9fb8` 恢复对应 AnimBP；该 Blend 不能直接删除，旧生成代码仍保持清理。
- 3 个 Jump Transition 已从旧 `Is_InAir` Getter 改绑到原生 `bIsFalling` 并保留全部连线。冷启动编译4个相关 AnimBP、Development Editor 构建均成功。
- 自测 PIE：地面持枪上半身恢复；强制腾空时 `bIsFalling=True/Is_InAir=True` 且关键骨骼发生变化。JumpLoop 素材本身接近 Idle，明显动作集中于 JumpStart/JumpEnd。
## 2026-09-01 session257 交接：第一人称完整 Locomotion 收敛到武器 Linked Layer

- `TABP_CharacterBase_FirstPerson` 现在只负责武器 Linked Layer 路由和通用 Lean/Look，不再保存 Idle/WalkRun/Jump 状态机或空中 Blend。
- `TABP_FirstPersonFirearmBase.WeaponUpperBody` 唯一拥有 Idle、WalkRun、JumpStart、JumpLoop、JumpEnd 状态机；`ABP_RepairGun_FirstPerson` 提供具体动画覆盖。
- 冷结构测试 `TheManTest.Player.Animation.WeaponOwnedLocomotion`、真实 PIE 跳跃测试 `WeaponOwnedJumpRuntime`、影子合成测试 `Shadow.UpperBodyEvidence` 全部 Success。
- `FramingCapture` 的既有 FOV=77 与 GripPoint 命名断言仍和用户当前蓝图值不一致，与本动画迁移无关，本轮未修改。
## 2026-09-01 session258 交接：第一人称武器状态机整理

- `TABP_FirstPersonFirearmBase.FirstPersonLocomotionSM` 删除 `Reset Animation` 与6条旧转换，Entry 改为直接进入 Idle。
- 状态现为 `Idle / Move / Jump Start / Fall Loop / Land`；`Grounded` State Alias 合并 Idle、Move 的共同起跳入口。节点数21→14，转换数固定为7。
- 冷结构、真实 PIE 跳跃、完整身体上半身复制三项自动化全部 Success，编辑器冷回读与 Development Editor 构建通过。

## 2026-09-01 session259 交接：纯位置式 Viewmodel Look Lag

- `ViewmodelRoot` 新增鼠标观察驱动的位置滞后：水平 1.8cm、垂直 1.2cm、进入速度 12、回正速度 16、死区 0.01；不改变旋转、相机、瞄准、弹道或 `ArmsViewMesh` 静态构图。
- 已删除旧的停用 Arms Pitch Follow 参数、状态及无效鼠标缓存。
- `TheManTest.Player.Viewmodel.PositionLagRuntime`、`WeaponOwnedJumpRuntime`、`Shadow.UpperBodyEvidence` 全部 Success；Development Editor 构建成功。

## 2026-09-01 session260 交接：保留原 Viewmodel 构图基准

- 修正 session259 首版错误地以零向量覆盖 `ViewmodelRoot.RelativeLocation`：现在 BeginPlay 保存蓝图 authored 位置，滞后只作为附加偏移，回正返回原构图。
- `PositionLagRuntime` 已改为以运行时初始构图为基准验证偏移和回正，单独冷启动 Success；Development Editor 重新构建成功。跳跃回归也在修正后 Success。

## 2026-09-01 session261 交接：Viewmodel 位置弹簧

- 目标位置插值改为速度冲量驱动的欠阻尼弹簧；默认水平/垂直冲量 7/5、刚度 85、阻尼 12，最大位移仍为 1.8/1.2cm。
- 修正垂直方向：向上鼠标输入先令枪械下移，随后自然回到蓝图原构图；没有旋转滞后。
- Development Editor 构建成功，更新后的 `TheManTest.Player.Viewmodel.PositionLagRuntime` 单独冷启动 Success。

## 2026-09-01 session262 交接：玩家可见文本统一为英语

- 屏幕倒计时改为 `[Round N | Phase N] Time Remaining: M:SS.s`；回合、阶段、快进、死亡、伤害、血量、切枪、角色生成与 GAS 初始化等现有屏幕 Debug 文本全部改为英语。
- 角色选择 UI 的默认结束文案改为 `GAME OVER`；Content 二进制资产未发现相关中文序列化文本。
- `.agents/harness/AGENTS.md` 与 `arch/13-game-flow.md` 已规定：所有玩家可见 UI、HUD 与屏幕 Debug 输出必须使用英语。Development Editor 构建成功。

## 2026-09-01 session263 交接：第一人称手臂/腿部灰模距离裁切

- 手臂专属 Masked 材质 Base Color 改为中性灰 `0.18`，保留原 40cm / 8cm 相机距离抖动裁切。
- 新增腿部专属 `M/MI_MaintenanceWorker_FirstPersonLegs`，参数 55cm / 12cm，并绑定 `SKM_MaintenanceWorker_LowerBody` 唯一材质槽；完整身体和影子原材质未改。
- 冷启动回读确认 Mesh 绑定、父实例和参数正确；`ArmDistanceClipEvidence` 与 `WeaponOwnedJumpRuntime` Success。`FramingCapture` 仍只因既有 FOV=77、GripPoint 命名旧断言失败，与本材质改动无关。

## 2026-09-01 session264 交接：恢复手臂/腿部各自原材质并保留裁切

- 源 VFXPack 冷审计确认原手臂使用 `MI_Placeholder_Lambert_INST`；正式迁入其 Master、Instance、`MF_Disintegration` 与 Noise 贴图完整依赖。本地手臂专属副本保留原 `Color=0.802083 / Metallic=0.5 / Specular=0.5 / Roughness=0.8` 和 Dissolve，仅在原 Opacity Mask 后叠加 40cm / 8cm 裁切。
- 纠正 session263 错误：腿部不再使用统一灰模；腿部专属父材质改由现有 `M_UE4Man_Body` 复制，只追加 55cm / 12cm 裁切，原外观图保持完整。
- 冷回读确认两 Mesh 绑定、父材质和参数正确；正常渲染 `ArmDistanceClipEvidence` 与冷启动 `WeaponOwnedJumpRuntime` 均 Success。
