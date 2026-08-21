# FEAT-077 — 第一人称手臂与完整身体双 AnimBP 合成

**创建日期：** 2026-08-21
**状态：** in_progress

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
