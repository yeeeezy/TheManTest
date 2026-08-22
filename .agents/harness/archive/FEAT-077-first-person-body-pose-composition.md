# FEAT-077 — 第一人称手臂与完整身体双 AnimBP 合成

**创建日期：** 2026-08-21
**状态：** in_progress

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
