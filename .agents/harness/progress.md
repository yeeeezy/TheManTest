# 进度日志

## 当前状态

**最后更新：** 2026-07-31-session127-complete
**当前功能：** FEAT-066 — 玩家持枪姿势与 Enemy 移动动画纠偏
**状态：** completed（`feature_list.json` 当前无法解析，主索引状态未安全改写）

## 完成内容

- 玩家初始 Viewmodel 固定为 16:9、FOV 110 下校准结果：Location `(-30, 3, -4)`，Rotation `(0, -12, 0)`；枪口和主体构图按用户 09:29 目标截图反复截图对齐。
- 删除 Patrol、SearchRush、Aim 的全部无 NavMesh / 寻路失败直线移动回退；Enemy 不再调用 `AddMovementInput`，只使用导航系统投射与 `MoveToLocation`。
- 删除两项无 NavMesh 直移自动化测试，避免继续把错误策略当作验收条件。
- 定位 Enemy 无动画根因：两个程序生成的 Phantom BlendSpace 只有 SampleData，缺少可运行采样数据；Patrol 的输入约定还与模板接线相反。已将 Patrol 统一为 X=Speed、Y=Direction，并重新生成/保存 Patrol 与 Aim BlendSpace 运行数据。
- 重新编译保存公共 Humanoid AnimBP 与 `ABP_Phantom_OriginalRifle`，保留原 Rifle_01 Mesh/Skeleton，不做重定向。
- 新增实际 TestMap 摆放 Phantom 的 PIE 审计：检查真实 Mesh/Skeleton/AnimClass、动画蓝图模式和最终骨骼姿势；移动时参考姿势差值必须大于阈值。

## 最终验证

- `TheManTestEditor Win64 Development`：Succeeded。
- `TheManTest.Enemy.Phantom + TheManTest.Player.Viewmodel.FramingCapture`：7/7 Success。
- 实际摆放 Phantom：`velocity=88.3`、`pose_delta=1.284`、Patrol BlendSpace 权重 `1.00`、播放时间推进到 `0.488`，不再 T-Pose/滑行。
- TacticalApproach、TacticalRetreat、PIESmoke、AnimationOverrides、ReusableCombatModules、FramingCapture 全部 Success。
- 源码检索确认 Enemy 范围无 `AddMovementInput`、NoNav、DirectMove 回退。
- 最终截图：`Saved/Screenshots/PlayerFramingCurrent.png`。
- 最终日志：`Saved/Logs/FEAT066FinalRegression3.log`；最新 Viewmodel `X=-30` 已包含在本轮构建、截图和 7/7 回归中。

## 工作区边界

- 安全 checkpoint：`b38d549`。
- 未改写/未纳入用户资产：`BP_MaintenanceWorker.uasset`、TestMap External Actor `D/YN/...`、未跟踪 `0/GS/` 与 `0/X7/`。
- 未在 TheManTest 内执行动画重定向或创建 IK Retargeter。
- 已发现 `.agents/harness/feature_list.json` 无法通过 `ConvertFrom-Json`；按 AGENTS.md 要求已报告，未擅自重构整个索引。
