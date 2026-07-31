# FEAT-066 — 玩家持枪姿势与 Enemy 移动动画纠偏

**状态：** completed  
**创建/完成：** 2026-07-31

## 目标

- 玩家初始持枪姿势与 09:29 目标截图在相同 16:9、FOV 110 条件下对齐。
- TestMap 实际 Phantom 移动时播放 Relaxed/Aim 动画，不再参考姿势或滑行。
- Patrol、SearchRush、Aim 仅使用项目现有 NavMesh，不保留直线移动回退。

## 实现

- Viewmodel：Location `(-30, 3, -4)`；Rotation `(0, -12, 0)`。
- 删除 `AHumanoidEnemy` 与 `AHumanoidAIController` 的直接移动状态和 `AddMovementInput` 路径；导航投射/请求失败时不伪造直线运动。
- 删除 `PIENoNavPatrol`、`PIENoNavSearch`。
- 修复 `BS_Phantom_RelaxedPatrol2D` 的输入约定为 X=Speed、Y=Direction，并重新生成 Patrol/Aim BlendSpace 运行采样数据。
- 重新编译保存 `ABP_HumanoidEnemy`、`ABP_Phantom_OriginalRifle`。
- `PIEPlacedAnimation` 对 TestMap 实际 Phantom 做最终骨骼快照审计，而不是只判断 AnimInstance 非空。

## 根因

原验收只确认了 Asset Override 引用存在，没有确认最终姿势。运行时数据显示 Phantom 正在移动且 Patrol BlendSpace 权重为 1，但播放时间始终为 0、最终骨骼等于参考姿势。BlendSpace 缺少有效运行采样数据；Patrol 还存在输入轴约定冲突。重建采样数据后播放时间正常推进，最终姿势脱离参考姿势。

## 验证证据

- Development Editor build：Succeeded。
- 最终自动化：7/7 Success。
- 实际 Phantom：OriginalRifle Mesh/Skeleton/AnimClass，`pose_delta=1.284`，移动速度 `88.3`，BlendSpace 权重 `1.00`、时间 `0.488`。
- 最终日志：`Saved/Logs/FEAT066FinalRegression3.log`，7/7 Success。
- Enemy 源码范围检索：无 `AddMovementInput`、NoNav、DirectMove 回退。
- 受控 1920×1080 / FOV 110 截图：`Saved/Screenshots/PlayerFramingCurrent.png`。

## 已知 harness 问题

`.agents/harness/feature_list.json` 当前不可解析；遵循项目指令已先报告，并从可读 archive/progress 完成交接，未擅自修复整个主索引。
