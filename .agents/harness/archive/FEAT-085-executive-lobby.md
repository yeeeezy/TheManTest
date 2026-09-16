# FEAT-085 执行官 Lobby 展示

状态：done，关闭日期2026-09-16。以下各批历史中的进行中／待制作记录已被最终验收覆盖。

## 最终验收（2026-09-16）

- selection_pie.json ok=true：实际LobbyMap的13个检查点通过，包括角色页Relax、执行官选择、Back保留角色、Weapon举枪、快速逆向切换、回Relax、维修工三枪及跨角色保留枪索引；始终仅一个可见展示Actor，地图SHA256不变。
- selection_preview.json ok=true：沉浸视口截取并查看 Character_Executive_UI00002.png／Weapon_Executive_UI00002.png，名字、介绍、选中框及Back可见；未添加角色缩略图。截图临时关闭景深／屏幕调试消息，不写入游戏配置。
- selection_audit.json ok=true：执行官32成品包冷加载，零源工作依赖、重定向资产和Redirector，Relax与Ready引用正确。源码Development Editor Win64构建成功、三项Blueprint编译保存；文档已同步。
- 本批未修改LobbyMap、战斗Pawn或Phantom。正式大厅初始仍为维修工，Character选择时生成执行官展示。没有新增测试地图／持久相机／按键；外部脚本和证据在D:/Blender Projects/ExecutiveLobby。
- 实现完成并移入feature_archive.json；没有自动开启其他功能。操作前checkpoint4cf536c，最终代码／资产／harness改动未提交或push。

## 第二批：角色选择与 Weapon 举枪（2026-09-15，进行中）

- 用户接受首版，要求 Character 打开真正的角色选择，参考武器 UI 但只放名字与介绍；角色选择和返回保持 Relax，选执行官后再次点击 Weapon 必须举枪。
- 操作前本地检查点 4cf536c 保存首批成品，未 push。新增 Controller 可配置角色展示列表及当前 Actor 引用，缓存两个角色实例与各自枪索引；所有姿态和 UI 均使用当前展示 Actor，不再任取场景第一个。
- UI 分为主菜单、角色页、武器页；Back 回主菜单并 Relax，Character 页文字按钮选择维修工／执行官，使用现有武器页面字体、间距、选中金边和 Back。执行官只有自带狙击枪，武器页显示名字介绍并隐藏维修工三枪选择器。
- 举枪源 W2_Stand_Aim_Idle_IP 在 TMIIR 原 RTG 生成，外部 Blender 修正枪方向及左手支撑，成品 AS_Executive_Lobby_ReadyIdle 迁入目标；30fps、3.2667 秒，99帧有限值和首尾检查通过，最大接缝0.0178cm，FBX回读骨位置最大误差0.000125cm。
- Development Editor Win64 构建成功（selection-build-final.log）；三项 Blueprint 编译保存，selection_author.json ok=true。正在实际 LobbyMap PIE 验证角色／菜单往返与原三枪回归，地图无预设替换。

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
