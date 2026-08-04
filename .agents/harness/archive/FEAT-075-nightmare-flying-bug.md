# FEAT-075 — Nightmare FlyingBug2 Locomotor 贴地爬行与结构修正

**状态：** in_progress

**重新打开：** 2026-08-04 session185

## session185 错误验收纠正

- session184 的视觉验收无效：`CharacterMesh0` 被保存为 `Pitch=-90°`，实际截图中 FlyingBug2 侧倒；玩家影子截图显示漂浮白色上半身和分离枪体。
- FlyingBug2 蓝图 Mesh 已先恢复 `Pitch=0°`，冷回读及平地/单坡 `Mesh Up · Actor Up > 0.99` 断言通过。
- 当前仍未完成：明显高低起伏区域连续爬行、玩家完整影子动画、VFXPack 枪械出现效果精确复刻与多轮冷启动画面验收。

## session185 实施与复验

- `TestMap` 新增远端七段连续验证区：平地、15°升坡、坡顶、15°降坡、凹地、12°横坡和终点。
- 修复 Character 爬坡结构：碰撞胶囊始终保持世界竖直，Actor 只跟随移动 Yaw；`CharacterMesh0` 单独按地面法线 Pitch/Roll，避免胶囊在坡缝侧倾卡死。
- 起伏路线最终三轮冷启动均 Success，18 秒平面位移分别为 `2104.1 / 2104.5 / 2102.4 cm`；Mesh 保持背朝上。
- 删除错误的 ShadowUpperBody 运行链：组件无 Mesh 且不投影；完整 `ShadowBodyMesh` 作为唯一身体影子并跟随 `CharacterMesh0`。可见 Viewmodel 枪体不再投影，新增附着完整身体 `GripPoint` 的 shadow-only 枪体。
- 枪械出现时序改为先创建 MID 并写入 VFXPack `Amount (S)=1`，再显示 Actor；避免首帧完整枪体先弹出。保留原 `0.5s` Hermite `1→0` 与起点切线 `-5.434987`。
- Shadow 与 Viewmodel 自动化各三轮 Success；仍需用户前台实际观察 Idle/WASD/冲刺/开火影子和开局/快速切枪出现效果后才能关闭功能。

## 实现

- `AEnemyBase` 明确保留为所有敌人的公共基类；`AHumanoidEnemy` 才是人形敌人公共层。
- Enemy 资产统一为 `/Game/Enemy/_Shared`、`Humanoid/_Shared`、`Humanoid/Phantom`、`Nightmare/FlyingBug2`；旧 `EnemyBase`、旧 `Phantom` 和 Redirector 清零。
- FlyingBug2 撤销 `MOVE_Flying`，使用 `MOVE_Walking`、重力、地面探针、阻尼后的平面速度及地表法线对齐；无 Controller 时也运行 CharacterMovement 物理。
- 新建 `CR_NightmareFlyingBug2_Locomotor`：Locomotor + FullBodyIK，Hips RootGoal，8 个足端分两相驱动；最终 Rig 直接配置在 Nightmare Mesh 上。
- 玩家新增仅投影的 `ShadowUpperBodyMesh`，与 `ArmsViewMesh` 使用同一 SkeletalMesh，并通过 Leader Pose 同步最终上半身持枪 Pose；主体影子隐藏重复手臂 section。

## 验收

- Development Editor / Win64 冷构建成功。
- `LocomotorCrawlEvidence`：`MOVE_Walking` + 两秒位移大于 20 cm，Success。
- `LocomotorSlopeEvidence`：18°坡面保持 Walking、实际爬行、Actor Up 与坡面法线点积大于 0.9，Success。
- `Shadow.UpperBodyEvidence`：Leader Pose 指向 Arms、同 SkeletalMesh、`hand_r` 组件空间 Pose 一致，Success。
- Enemy 冷加载：`_Shared=2`、`Humanoid/_Shared=10`、`Humanoid/Phantom=152`、`Nightmare/FlyingBug2=11`；旧目录资产 0、Redirector 0；Phantom、Nightmare 蓝图和 Locomotor Rig 均可加载。
- 证据：`Saved/Logs/NightmareCrawlRenderTest4.log`、`NightmareSlopeRenderTest.log`、`ShadowUpperBodyRenderTest2.log`、`ValidateEnemyStructure2.log`；截图位于 `Saved/Screenshots/WindowsEditor/TMT_NightmareLocomotor_Crawl.png`、`TMT_NightmareLocomotor_Slope.png`、`TMT_ShadowUpperBody_Runtime.png`。
