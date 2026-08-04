# FEAT-075 — Nightmare FlyingBug2 Locomotor 贴地爬行与结构修正

**状态：** done

**关闭：** 2026-08-04 session184

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
