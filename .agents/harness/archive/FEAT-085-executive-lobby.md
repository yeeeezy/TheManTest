# FEAT-085 执行官 Lobby 展示

## 用户授权（2026-09-15）
- 用户接受保留A0102原身材/骨架，重定向已有持枪动画并按展示图修正双手，接入Lobby。
- 首批先完成素材自带狙击枪的Relax持枪待机与预览；举枪版本待首批观感确认后继续。
- 外部制作D:/Blender Projects/ExecutiveLobby；重定向仅在TMIIR，TheManTest只接收成品。Phantom腿部与接地问题明确暂缓。
- 操作前检查点56ce39f，保存此前Phantom实现和待修记录，未push。
- 状态in_progress：开始恢复原CAT/Skin与材质，尚未生成或迁入执行官资产。

## 首批交付与验证（2026-09-15）

- 恢复Max原CAT/Skin：身体33832顶点、披风4080顶点，全37912顶点有权重。原Max SHA256 `e4971a3653c95dbc327de927cd1e2d135a005818540bf75e43e51f573dad04f5`，未保存修改。原Blender面索引三角化用于准确恢复材质归属，保留原身材。
- 在外部Blender规范英寸→厘米导出，解决2.54缩放残留；原62骨导入UE为63骨（增加容器根ExecutiveRoot），UE骨名将空格转连字符。身体与披风合并为一套显示网格，原权重保持。
- TMIIR的`/Game/ExecutiveWork`拥有两套IK Rig及RTG_Rifle_Executive；18条链映射并对齐目标参考姿势，源动画`/Game/Rifle_01/Animation/In-Place/W2_Stand_Relaxed_Idle_IP`。
- 外部Blender对已重定向的同骨架动画进行握持修正：使用自带狙击枪，左臂两骨解算后烘焙每帧FK，使手掌持续托住前护木；右手沿用重定向握姿、食指离扳机，枪以固定右手挂点跟随。未使用运行时IK。原披风辅助骨随胸部继承，无布料物理模拟。
- 成品`AS_Executive_Lobby_RelaxedIdle`：30fps、99个采样、3.2667秒。原动画呼吸/重心变化保留。source_validation.json：FBX回读最大骨位置误差0.0001031cm；循环首尾头/手/脚位置差最大0.0378cm；全99帧有限值验证通过。
- ground_check.json五帧身体高度约182.05–182.08cm，最低鞋底Z=0.646–0.667cm（网格原点基准）；放置时可将Actor设为地面Z减0.65cm贴底。未宣称实现运行时地面IK。
- 30个成品通过AssetTools从TMIIR迁入TheManTest的`/Game/Characters/TheExecutive/Lobby`：骨骼网格、目标Skeleton、静态狙击枪、一条动画、5材质、21纹理。未迁入ExecutiveWork/Rifle_01源依赖、IK Rig或Retargeter。材质按原贴图重建，深色倍率和红色全息参数可调，未宣称复现旧V-Ray渲染。
- 新增`Blueprint/BP_Executive_Lobby`直接继承ALobbyCharacterBase，默认Relaxed；使用原LobbyPoseBlendAnimInstance、Bip01-R-Hand挂点与显示枪。当前仅Relax成品，未配置Rifle/Standing，待首批观感确认后继续。
- 实际LobbyMap PIE临时换入执行官做远景/近景预览；未保存地图或替换正式维修工展示。lobby_pie.json ok=true，63骨、3个身体材质槽、武器显示与挂点、真实动画播放均通过，地图SHA256不变。修正截图景深/纹理流送设置后查看两张清晰截图；仅本次验证进程改变渲染设置。
- 独立冷审计target_audit.json ok=true，31个资产（含BP）全部加载；范围内零重定向资产、零源工作依赖及零Redirector。无C++变化，无需编译；BP在UE编译验证。
- 外部动图Executive_Relaxed_Loop.gif为Blender制作预览；Lobby_Executive_Relaxed.png / Lobby_Executive_Hands.png为实际大厅PIE截图。可编辑工程Executive_Relaxed.blend，全部脚本/报告位于D:/Blender Projects/ExecutiveLobby。
- 首批可供用户评审，功能保持in_progress。按此前约定，用户确认Relax握持/比例后再做举枪版本、姿态切换和正式大厅选择接入。Phantom待修继续暂停。结果未最终提交或push。
