# 进度日志

## 当前状态

**最后更新：** 2026-07-31-session126-phantom-and-player-framing-complete
**当前功能：** 无（FEAT-057～FEAT-065 已完成归档）
**会话编号：** 126

Phantom 原始 Rifle_01 动画体系、公共巡逻/感知/丢失搜索、八方向战术移动、掩体、20 发弹匣、三连发/扫射/换弹、二阶段透明穿透与范围轰炸已完成。玩家初始持枪构图也已按参考截图通过 ViewmodelRoot 调整并建立确定性截图回归。

## 最近关键完成项

- Phantom 使用 `/Game/Enemy/Phantom/OriginalRifle` 原 `SK_Mannequin + UE4_Mannequin_Skeleton`；Patrol/Search 仅 Relaxed，Aim 后才使用 15-sample 二维 Aim BlendSpace。
- 公共状态链：`Patrol → Aim → SearchRush → SearchScan → Patrol`；丢失目标先冲向 LastKnown，再随机播放 Relaxed Fgt v1～v4 环视并恢复最近巡逻点。
- Aim 战术移动公共化：700±150cm 距离环；过近后撤+侧移、过远斜进+侧移、环带内绕行；BT Actor MoveTo 不再覆盖战术移动；无 NavMesh 时使用安全直移回退。
- 公共战斗模块：`UEnemyMagazineComponent`、`AEnemyCoverPoint`、Burst、SuppressiveFire、Reload、TakeCover、AreaBarrage。
- Phantom Phase 1：TakeCover/Burst/SuppressiveFire/Reload；Phase 2 保留一期能力并新增 Cloak projectile pass-through 与 AreaBarrage。
- 玩家构图：`Capsule → HeadCamera(110°) → ViewmodelRoot(Location 0,0,-7; Rotation 0) → ArmsViewMesh`；装备继续挂 ArmsViewMesh Socket，未移动 gameplay 相机或改骨架基础旋转。
- 玩家 1920×1080 自动截图在稳定 Idle 帧捕获，并同时验证 FOV、层级、Transform、装备与 Socket。

## 最终验证

- `TheManTestEditor Win64 Development`：Succeeded（UE 5.7）。
- `Saved/Logs/PhantomFullRegressionFinal3.log`：7/7 Success：
  - `TheManTest.Enemy.Phantom.AnimationOverrides`
  - `TheManTest.Enemy.Phantom.PIENoNavPatrol`
  - `TheManTest.Enemy.Phantom.PIENoNavSearch`
  - `TheManTest.Enemy.Phantom.PIESmoke`
  - `TheManTest.Enemy.Phantom.PIETacticalApproach`
  - `TheManTest.Enemy.Phantom.PIETacticalRetreat`
  - `TheManTest.Enemy.Phantom.ReusableCombatModules`
- 战术距离 PIE 证据：环带侧移、1200cm 斜进、300cm 后撤均产生横向移动并朝期望距离变化；详见 FEAT-064 归档。
- 无导航实测：Patrol 位移 94.3cm 后 `scanning=true`；SearchRush 位移 189.6cm 后进入 SearchScan 且 `scanning=true`。
- `Saved/Logs/PlayerViewmodelRegressionFinal.log`：`TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success。
- 最终截图：`Saved/Screenshots/PlayerFramingCurrent.png`（1920×1080）。
- `feature_list.json` 与 `feature_archive.json` 均通过 PowerShell `ConvertFrom-Json`；归档末项 FEAT-065，active feature 为 null。

## 已知旧问题

- 启动日志仍有项目既存 TestGun `A_HandFire` 无 Skeleton、`FCharacterType::CharacterIcon` 未初始化；这些发生在测试发现阶段，但目标测试仍被执行并报告 Success。
- 原公共 AimIK 模板把 `AimSocket` 当骨索引，在 Rifle 原骨架上输出初始化警告并安全失效；本系列没有重定向或创建 IK 中间资产。
- TestMap 运行时未发现 RecastNavMesh；Patrol、SearchRush 与 Aim 战术移动现均有无导航直移回退并通过 PIE。地图中的 NavMesh External Actor 状态属于用户/未知工作区改动，未擅自保存或覆盖。

## 工作区边界

- 用户/未知改动保持未纳入：`BP_MaintenanceWorker.uasset`、TestMap External Actor `D/YN/...`、未跟踪 `0/GS/` 与 `0/X7/`。
- 本系列没有在 TheManTest 内执行 IK Retargeter、批量重定向或留下源骨架/源项目工作目录。
- 安全 checkpoint：`a3e7e83`（AnimBP 修复前）、`b8121b9`（AnimBP 修复后）。本轮 FEAT-064/065 结果保留在工作区，未创建用户未要求的正式提交。
