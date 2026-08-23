# FEAT-077 — 第一人称手臂与完整身体双 AnimBP 合成

**创建日期：** 2026-08-21
**状态：** in_progress

## 2026-08-23 session232 — 构图旋转统一到相机中心支点

- 按用户确认删除 `BaseArmsRotation` / `Viewmodel Arms Rotation`，不再依赖导入手臂 Mesh 自身原点调整静态旋转。
- `Viewmodel Offset Rotation` 改为 `EditAnywhere`，默认值合并为原 Arms 基础朝向 `(-3,-90,-1)`；PIE Tick 每帧直接应用到相机中心的 `ViewmodelRoot`，并在此基础上叠加冲刺 Pitch。
- `Viewmodel Offset Location` 仍每帧写入子组件 `ArmsViewMesh`；Arms 自身 RelativeRotation 固定为零。这样不同导入模型共用相机中心旋转支点。
- 写入前检查点 `c7f1532`；`TheManTestEditor Win64 Development` UHT、编译与链接全部成功。待用户前台 PIE 手调并确认最终值。

## 2026-08-23 session231 — PIE 实例构图每帧直接覆盖

- 纠正 session229 对用户要求的误解：删除 `PostEditChangeProperty` 变化判断，不再依赖编辑器属性回调。
- Tick 每帧无条件将 `ViewmodelOffsetLocation` 和 `BaseArmsRotation` 写入 `ArmsViewMesh`，PIE 运行实例修改 Location/Rotation 后可在下一帧生效。冲刺 Pitch 仍独立叠加在父级 `ViewmodelRoot`。
- 写入前检查点 `8fb7f78`；`TheManTestEditor Win64 Development` 完整编译链接成功。用户未提交的 `BP_MaintenanceWorker.uasset` 和 TestMap ExternalActor 删除未被覆盖或纳入本次修改。

## 2026-08-22 session230 — 未解决构图问题停机交接

- 用户明确确认当前枪械位置/朝向构图问题仍未解决，计划次日在 PIE 运行实例中手动调整 `Viewmodel Offset Location` 与 `Viewmodel Arms Rotation` 后复核。
- 测试前需清零旧的父级 `Viewmodel Offset Rotation`；保留当前未提交的 `BP_MaintenanceWorker.uasset` 手调，不覆盖、不提交。
- 本次只记录状态，没有新增实现或验证；FEAT-077 继续保持 `in_progress`，FramingCapture 待手调完成后重跑。

## 2026-08-22 session229 — PIE 实例即时构图编辑

- Location 与 Arms Rotation 改为实例可调；编辑器 `PostEditChangeProperty` 仅在用户修改属性时更新 Arms Transform，Tick 保持无静态构图检测/覆盖。
- 检查点 `0b54ba2`；Development Editor 冷构建成功，用户 BP 手调资产未覆盖。

## 2026-08-22 session228 — Arms 自身旋转参数暴露

- `BaseArmsRotation` 改为 `Viewmodel|Framing` 下显示的 `Viewmodel Arms Rotation`，保持默认 `(-3,-90,-1)`，用于不改变组件 Location 的 Arms 自身静态朝向调整。
- 检查点 `3af3217`；Development Editor 冷构建成功，Shadow/EquipDissolve Success。FramingCapture 因用户 BP 中仍保存旧父枢轴 Rotation/Location 手调而失败，待用户重置旧 Rotation 并改用新参数后复核；未覆盖用户 `.uasset`。

## 2026-08-22 session227 — 实际速度驱动冲刺压枪

- `SprintTransitionAlpha` 仅负责0.2秒 MaxWalkSpeed 切换；`ViewmodelRoot` 压枪改由实际水平速度在550..750映射出的 `SprintVisualAlpha` 驱动。
- 原地 Shift、受阻或速度不超过 WalkSpeed 时不压枪；专项运行时断言通过。写入前检查点 `102ffab`，Development Editor 冷构建及3项玩家回归全部 Success。

## 2026-08-22 session226 — 静态构图停止逐帧覆盖

- ViewmodelRoot 静态位置、Arms 的 Offset Location/Base Rotation 改为 `BeginPlay` 应用一次；Tick 只更新冲刺动态 Pitch 和 AnimBP 输入。
- 写入前 WIP 检查点 `2ba4834`。首次冷链接因运行中的 Unreal Editor 锁定 DLL 报 LNK1104；关闭编辑器后 Development Editor 冷链接成功，3项玩家回归全部 Success。

## 2026-08-22 session225 — Body Sway 插值速度参数化

- 新增 `ViewmodelBodySwayInterpSpeed`（`Viewmodel|Movement`，默认 `6.0`），替代普通移动硬编码 `2.0`；进入倾斜和松键回弹同步加快。冲刺仍使用原版 `8.0`。
- 写入前 WIP 检查点 `2e3b5ad`；Development Editor 冷构建及3项玩家回归全部 Success。

## 2026-08-22 session224 — 恢复用户认可的 session220 表现

- 按用户决定精确恢复 `e1c24eb` / session220 的完整75° Lean/Look 骨骼映射，保留当时倾斜观感及其已知上下平移。
- 删除随后加入的装备 Actor 独立 Roll、Arms 位置补偿、输入符号修正和专项测试；三份相关源码与目标检查点定向比较一致。
- Development Editor 冷构建及恢复后的3项 `TheManTest.Player` 回归全部 Success。

## 2026-08-22 session223 — A/D 倾斜符号纠正

- session222 只验证旋转幅度与枪口稳定，遗漏视觉方向；用户前台发现方向相反。
- 按原版运行记录 A→正 Lean、D→负 Lean，将侧移目标改为 `-MoveInput.X`。专项测试新增 D 输入 `Lean_Sides_Amount < -5°` 断言，并以真实 D 输入截图对照原版 `TMT_VFXPack_StrafeRight.png`，两者枪身均为左上→右下。
- Development Editor 构建及全部4项 `TheManTest.Player` 回归 Success。

## 2026-08-22 session222 — VFXPack 骨骼倾斜与枪口枢轴补偿

- 撤销 session221 的装备根节点独立 Roll；按原 VFXPack 恢复 `spine_03` Component Space Additive Roll、`hand_l` 半倍率 Roll，以及 `hand_r/GripPoint` 随骨骼 Pose 带枪。
- 删除 75° Roll/Pitch 交叉映射，纯 A/D 不再写入 `Look_Up_Amount`。专项审计先复现无补偿时枪口约 `-18.6cm` 高度弧线，再以中性 `spine_03→muzzle` 向量计算圆弧并补偿 Arms 构图位置。
- 最终自动化实测 `spine_03=6.92°`、`hand_r=7.82°`、枪口高度变化 `-0.006cm`；装备 Actor 相对 Transform 不变。Development Editor 构建及全部4项 `TheManTest.Player` 回归 Success，待用户前台 PIE 观感验收。

## 2026-08-22 session220 — 恢复原枪体绕轴旋转

- 用户澄清枪体必须保持原地，目标是保留修改前沿枪管前向轴的倾斜旋转；session218/session219 的位置滞后与旋转拆分属于误解，已全部撤回。
- 玩家源码及测试精确恢复至 `e1c24eb`：ViewmodelRoot 零位移，完整 75° Lean/Look 映射恢复。临时最终枪体旋转探针及错误位置专项测试均已删除。
- Development Editor 构建和三项玩家动画回归通过。

## 2026-08-22 session219 — 侧移 Roll 幅度恢复

- 修正 session218 遗留的侧移 Roll 衰减：不再乘 `cos(75°)`，恢复原始 `±8°` 绕枪轴倾斜；A/D 到 Look/Pitch 的串扰继续保持为0。
- MovementLagDirections 现同时断言平滑后 Roll >5°、Pitch <0.05°；构建、专项测试和三项玩家动画回归均通过。

## 2026-08-22 session218 — 移动位置滞后与骨骼旋转解耦

- 隔离采样确认 A/D 上下偏移主要来自侧移 Lean 经 75°基差交叉写入 Look/Pitch；WalkRun 仅贡献约0.23cm，组件 Location 原本没有任何移动滞后。
- 正式实现保留侧移的枪械绕轴 Lean/Roll，只删除侧移到 Look/Pitch 的交叉项；新增独立 `ViewmodelRoot` 相机局部 XY 反向位置滞后，Z 永远为0。默认 A/D 3cm、W/S 2cm、插值速度6。
- 新增 MovementLagDirections 自动化；四向符号、零Z、Roll保留、Pitch隔离均通过，原三项玩家动画回归及 Development Editor 构建也通过。

## 2026-08-22 session217 — 枪械 WalkRun 专用 Blend Space Player

- 按用户审查纠正 session216：枪械模板 Idle 使用空 Sequence Player，WalkRun 使用空 Blend Space Player，不能用 Run Sequence 代替移动 BlendSpace。
- WalkRun 的 `Speed` 已连接 Blend Space X；RepairGun 子类通过 Parent Asset Override 指定 `BS_WalkRun_RepairGun`，模板不直接引用该具体资产。
- 冷导出结构审计、Development Editor 构建和三项玩家动画冷启动回归全部通过。

## 2026-08-22 session216 — 枪械模板断线节点清理

- 冷导出确认 `TABP_FirstPersonFirearmBase` 的 Idle、WalkRun 实际输出均为无具体资产的 `Sequence Player`；WalkRun 内另有旧方案遗留且完全断线的 `Blend Space Player` 和 Speed Getter。
- 删除上述两个孤立节点。`ABP_RepairGun_FirstPerson.ParentAssetOverrides` 继续分别为两个 Sequence Player 提供 Idle、Run 资产，模板对 RepairGun 目录具体资产依赖为 0。
- Development Editor 构建成功；三项玩家动画自动化均通过。截图型测试须使用渲染冷启动，不能使用 `-nullrhi`。

## 目标

- `ArmsViewMesh` 独立运行第一人称 AnimBP，不接收身体 root/pelvis locomotion。
- `CharacterMesh0` 保留完整身体 locomotion，并在自身 AnimBP 中从上半身混入 `ArmsViewMesh` 的最终局部骨骼 Pose。
- `CharacterMesh0` 继续作为唯一完整隐藏影子来源，不增加第三个 Shadow Mesh 或第三套动画职责。

## 已确认方案

1. `ArmsViewMesh` 使用独立 `ABP_CharacterBase_FirstPerson`。
2. `CharacterMesh0` 使用 `ABP_CharacterBase_Body` / `TABP_BodyLocomotion` 完整身体状态机。
3. Body AnimBP 以自身 locomotion 为 Base Pose，以 `Copy Pose From Mesh(ArmsViewMesh)` 为 Blend Pose，从 `spine_01`（最终以骨架审计为准）以上执行 `Layered Blend Per Bone`。
4. 不复制第一人称 root/pelvis；第一人称组件的相机空间 Transform 不进入身体影子。
5. 通过组件 Tick prerequisite 保证 Arms 先求值、CharacterMesh0 后合成。

## 2026-08-21 session210

- 用户确认按上述双 AnimBP 合成方案实施。
- 写入前检查点：`b382f95`。
- FEAT-076 自动化已完成但仍待用户主观验收，状态改为 `needs_improvement`；本功能成为唯一活动功能。

## 实施结果

- `UFPSCharacterAnimInstance.FirstPersonPoseSource` 在运行时指向角色的 `ArmsViewMesh`。
- `TABP_BodyLocomotion` 最终输出新增 `Copy Pose From Mesh -> Layered Blend Per Bone(spine_01)`；复制使用局部骨骼空间，不接收 Arms 组件的相机空间 Transform，root/pelvis 保留 Body locomotion。
- `ArmsViewMesh` 固定先于 `CharacterMesh0` Tick；后者再合成完整影子，避免上一帧 Pose。
- `BP_MaintenanceWorker` 明确分配两类：Body=`ABP_CharacterBase_Body_C`，Arms=`ABP_CharacterBase_FirstPerson_C`。
- Equipment Linked Layer 与 Montage 仅进入 Arms AnimInstance；Body 不再重复 Link/Unlink 或重启武器层。
- 自动化增加 Body/Arms 不同类、Copy Pose 来源指针、`spine_03/hand_r/hand_l` 局部旋转一致性断言。

## 验证

- `TheManTestEditor Win64 Development`：Success（三轮增量/冷链接均通过）。
- `TheManTest.Player.Shadow.UpperBodyEvidence`：Success。
- `TheManTest.Player.Viewmodel.FramingCapture`：Success。
- `TheManTest.Player.Viewmodel.EquipDissolveEvidence`：Success。

## 2026-08-22 session221：枪械前轴 Roll 独立于骨骼滞后

- A/D 的上下漂移确定来自 AnimBP 上游骨骼 Modify Bone 旋转使 `GripPoint` 沿弧线位移，而不是 ViewmodelRoot/ArmsViewMesh 的组件 Location。
- Lean/Look 骨骼滞后输入暂时关闭；A/D 改为只对当前装备根节点施加即时本地 X/Roll `±8°`，同时把该根节点相对 Location 固定为零。
- 自动化实际注入左右输入并断言 Location=0、Roll=`-8°/+8°`；Development Editor 构建及三项玩家回归均 Success，待用户前台观感验收。
- 待用户前台实际输入复核切枪、Idle/移动/跳跃/开火/冲刺观感后决定是否关闭。

## 正式目录与命名整理

- 第一人称正式运行类移除素材来源名：`ABP_VFXPack_FirstPerson` → `ABP_CharacterBase_FirstPerson`。
- 身体正式运行类从技术性的 `Animations/Skeleton` 迁至功能目录：`Animations/Body/Logic/ABP_CharacterBase_Body`。
- 第一人称类保留于 `Animations/FirstPerson/Logic/`；无骨架身体模板继续位于共享 `Animations/Logic/`。
- 迁移通过 Unreal AssetTools 完成并修复引用；旧 `Animations/Skeleton` Asset Registry 为 0 且磁盘目录已删除。

## 2026-08-21 session211 — Linked Anim Layer 模板化与目录归属

- 第一人称常驻宿主改为 `ABP_MaintenanceWorker_FirstPerson`（`UCharacterBaseAnimInstance`），地面持枪 Pose 从 `ALI_WeaponAnim.WeaponUpperBody` 获取；当前枪械模板尚无 Jump 状态，因此 `bIsFalling` 时显式切回宿主原 Jump 状态机，避免相对改前退化。通用骨骼晃动继续位于最终输出端。
- 枪械模板改名 `TABP_FirstPersonFirearmBase`（`UFirearmAnimInstance`），维修枪骨架子类改名 `ABP_RepairGun_FirstPerson`。武器切换继续只替换 Linked Layer。
- C++ 层级统一为 `UBaseLocomotionAnimInstance -> UFPSCharacterAnimInstance -> UCharacterBaseAnimInstance -> UFirearmAnimInstance`；速度、方向、垂直速度、腾空、移动状态和通用 Lean/Look 不再由素材包 EventGraph重复计算。
- 角色专属 AnimBP 已从 `CharacterBase` 移至 `MaintenanceWorker/Animations/{FirstPerson,Body}/Logic`；无引用空模板 `TABP_CharacterBase` 删除。
- Development Editor 构建通过；三项冷启动自动化 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 全部 Success。

## 2026-08-22 session213 — 身体模板提升至 CharacterBase

- 纠正模板所有权：无骨架 `TABP_MaintenanceWorker_BodyLocomotion` 不绑定 MaintenanceWorker Skeleton 或动画资产，其职责是提供玩家身体 Locomotion 状态机与上半身 Pose 合成框架。
- 通过 Unreal AssetTools 改名并迁移为 `/Game/Characters/CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion`。
- `ABP_MaintenanceWorker_Body` 仍留在具体角色目录，继续承担 MaintenanceWorker Skeleton 与动画 Asset Override；迁移后已重新编译保存。
- 写入前 WIP 检查点：`a3da38b`。
- 新模板与具体 Body AnimBP 资产加载验证通过；旧路径 Asset Registry 为0且无 Redirector，新模板唯一确认引用方为 `ABP_MaintenanceWorker_Body`。
- 关闭编辑器后 `TheManTestEditor Win64 Development` 冷构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session214 — 第一人称宿主模板化

- 新增无骨架 `/Game/Characters/CharacterBase/Animations/FirstPerson/Logic/TABP_CharacterBase_FirstPerson`，承载原宿主完整 AnimGraph、基础 locomotion/Jump、武器 Linked Layer 路由和最终 Lean/Look。
- `ABP_MaintenanceWorker_FirstPerson` 保留具体 Skeleton 与原资产路径，ParentClass 改为新模板生成类并删除本地 AnimGraph，成为纯具体角色子类。
- 冷审计确认模板 `TargetSkeleton=None` 且 AnimGraph 存在；子类 Skeleton 仍为 `SK_Mannequin_Arms_Skeleton`、本地 AnimGraph 为 None，并明确依赖模板。
- 写入前 WIP 检查点 `20be43d`；Development Editor 冷构建及三项玩家动画回归均 Success。

## 2026-08-22 session215 — 模板播放器占位与具体资产下沉

- 将 `TABP_CharacterBase_FirstPerson` 的状态机正式命名为 `FirstPersonLocomotionSM`。
- Body 与 FirstPerson 两个 CharacterBase 模板均只保留无具体资产的 `Sequence Player` / `Blend Space Player` 结构；冷启动依赖审计确认对 `/Game/Characters/MaintenanceWorker/` 的依赖均为 0。
- Body 模板原已采用子类 Asset Override，无需迁移。FirstPerson 模板的 7 个播放器绑定迁入 `ABP_MaintenanceWorker_FirstPerson.ParentAssetOverrides`：Idle、Still 两处、JumpStart、JumpLoop、JumpEnd 和 WalkRun BlendSpace。
- 因模板 Sequence Player 不再绑定资产，Jump 的 3 条剩余时间 Getter 改为状态机原生自动剩余时间过渡，模板与具体子类均重新编译保存。
- `TheManTestEditor Win64 Development` 构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。仍待用户前台主观复核。
