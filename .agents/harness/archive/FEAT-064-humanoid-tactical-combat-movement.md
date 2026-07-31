# FEAT-064 — 通用人形 Enemy 八方向战术移动

**状态：** done  
**创建：** 2026-07-31  
**依赖：** FEAT-057、FEAT-058、FEAT-063

## 目标

替换 Aim 状态下行为树持续 `Move To(TargetActor)` 导致的直线逼近表现。公共人形 Enemy 应持续面向并瞄准目标，同时按距离环带选择侧移、后撤或斜向接近；实际局部速度继续驱动骨架兼容的二维 Aim BlendSpace。

## Session124

- 已定位根因：公共 BT 的目标 Actor MoveTo 与 Aim Focus 同时生效，结果是始终正面直线接近。
- 开始在 `AHumanoidAIController` 增加公共战术落点计算与 NavMesh 投影：过近后撤、过远斜向收拢、合适距离切向绕行，短时保持侧移方向并偶尔换向。
- 自动化将覆盖过近/过远/左右侧移几何不变量；最终仍需真实 NavMesh PIE 验证行为节奏和二维动画方向。

## Session125 完成与迭代

- `CalculateCombatMoveDestination` 形成公共距离环带：默认 700±150 cm；过近同时后撤+侧移，过远同时收拢+侧移，环带内切向绕行。
- `MoveTo` 覆盖仅拦截 Aim 状态下 BT 的 Actor 目标直线 MoveTo，并返回 AlreadyAtGoal 让技能序列继续；战术 Location MoveTo 仍走 NavMesh。
- 正式地图优先投影到 NavMesh；TestMap 无 RecastNavMesh 时使用 CharacterMovement 直接移动回退，因此动画/战斗仍可运行且不持续直冲。
- 侧移方向以 1～2 秒决策周期短时保持，并以 35% 概率换向，避免逐帧抖动。
- 首版 2.5 秒 PIE 被 TestMap 自动切 LobbyMap 干扰；验收调整为关卡切换前的独立短 PIE，并将错误的“净位移”指标改为累计轨迹长度，能够正确覆盖换向后回到起点附近的合法移动。
- `TheManTest.Enemy.Phantom.PIE*` 三项最终均 Success：
  - 环带侧移：travel=144.4，lateral_max=296.7，distance=[700.0,718.4]
  - 远距斜进：travel=105.7，lateral_max=160.4，distance=[1125.9,1200.0]
  - 近距后撤：travel=151.9，lateral_max=152.0，distance=[300.0,432.3]
- `ReusableCombatModules`、`AnimationOverrides` 均 Success；Aim BlendSpace 15 samples；Development Editor 构建成功。
- session126 最终全回归：环带 travel=102.7/lateral=280.0/distance 700.0～707.5；远距 1200→1115.9；近距 300→388.2，三项均 Success。

## 待办

- [x] C++ 编译与自动化通过。
- [x] 处理 BT MoveTo 与公共战术移动的所有权，确保不会互相抢写移动请求。
- [x] PIE 验证环带侧移、近距后撤、远距斜进、Aim/AnimInstance 和距离稳定性；TestMap 无 NavMesh 时回退路径生效。
- [x] 根据多轮随机 PIE 轨迹修正验收时长、出生高度和累计路程指标。
