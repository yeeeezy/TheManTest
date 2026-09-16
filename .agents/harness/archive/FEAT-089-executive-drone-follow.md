# FEAT-089 执行官无人机跟随与大厅展示

状态：done。用户已确认专属Pawn/AIController/BT/BB方案，本轮仅跟随+Lobby悬停偶尔转一圈。不接攻击GAS、独立血量或降落玩法。原六动画迁入保留，当前使用Idle/TurnLeft/TurnRight。

开始时工作区干净，基线6491f27（已推送）。遵守源项目动画边界，不修改地图，外部证据D:/UnrealWork/ExecutiveDrone。

- 14包由TMIIR AssetTools迁移，源文件哈希不变；目标通过AssetTools重命名到TheExecutive/Drone，18项成品含BP/ABP/BT/BB，旧AssetRegistry目录为0。
- Dedicated Drone Pawn/AIController/Follow BT任务/飞行组件与原生动画变量已实现。执行官战斗及Lobby都通过专属Companion组件生成，Lobby不创建AIController，独立悬停与12秒周期完整转身。
- Development Editor Win64构建通过。首次构建修正auto*推导TObjectPtr与局部Pawn遮蔽错误。资产脚本修正非反射RootNode字段访问后完成，author.json ok=true，BP/ABP编译保存。实际PIE验证进行中。

- 实际Lobby第一轮确认无人机生成/动画有效，但位置出画；BP缩至60cm宽并移到执行官肩侧，Mesh启用角色灯光Channel1。修正后实际截图完整可见且不遮角色或UI。Lobby轮转按游戏时间，验证脚本等待完整周期；隐藏查询改读Actor.hidden。
- 移除无人机对WorldDynamic的阻挡响应，避免拦截玩家弹体；移动体只阻挡静态场景，跟随路径查询仍会检测阻碍。局部避障/路径点不是全局3D寻路。

- Weapon framing repair: original temporary combat setup missed ViewmodelRoot SceneComponent transform. Worker offset (-6.153601,30.210103,-12.871626), both target BPs were zero. User authorized correction; copied only this transform to Executive/Infiltrator. Safety checkpoint14adb1f, no push. weapon-fix.json and rendered three-character weapon-pie.json ok=true; positions/FOV/animation/GripPoint1 attachment match. Lobby untouched.
- Drone detour passed427 samples/79cm final error, subsequent pause validation timed out. Final drone regression and cold audit outstanding. Old vendor folder removed.
- Secondary monitor requirement verified with launch_secondary.py; UE window rect(2580,20)-(4180,920).

- Final regression: pie.json ok=true after final mesh yaw correction and weapon framing fix. obstacles.json ok=true: detour359 samples/71.08cm arrival, pause freezes and unpossess removes both Pawn/controller. Prior timeout was external script returning early when controlled pawn became null, now fixed. cold.json ok=true:18packages/20registry entries (generated classes included), zero vendor dependencies/redirectors, six sequences use final skeleton, four BPs/ABP compile without log errors. LobbyMap/TestMap SHA256 unchanged.

- Completed: final rendered F11 editor viewport captures Final_Character00000.png/Final_Weapon00000.png inspected; drone visible beside Executive and clear of UI. Initial Shot captures selected the editor scene until F11 activated the PIE viewport; those earlier screenshots are not final visual evidence. Existing directional-light/VSM editor warnings are outside this change; no lighting/map edits. All launched UE editors exited. Scope remains follow/lobby only; no combat/GAS, landing or global3D path planner. Architecture07/11/12 updated.
