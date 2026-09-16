# FEAT-089 执行官无人机跟随与大厅展示

状态：in_progress。用户已确认专属Pawn/AIController/BT/BB方案，本轮仅跟随+Lobby悬停偶尔转一圈。不接攻击GAS、独立血量或降落玩法。原六动画迁入保留，当前使用Idle/TurnLeft/TurnRight。

开始时工作区干净，基线6491f27（已推送）。遵守源项目动画边界，不修改地图，外部证据D:/UnrealWork/ExecutiveDrone。

- 14包由TMIIR AssetTools迁移，源文件哈希不变；目标通过AssetTools重命名到TheExecutive/Drone，18项成品含BP/ABP/BT/BB，旧AssetRegistry目录为0。
- Dedicated Drone Pawn/AIController/Follow BT任务/飞行组件与原生动画变量已实现。执行官战斗及Lobby都通过专属Companion组件生成，Lobby不创建AIController，独立悬停与12秒周期完整转身。
- Development Editor Win64构建通过。首次构建修正auto*推导TObjectPtr与局部Pawn遮蔽错误。资产脚本修正非反射RootNode字段访问后完成，author.json ok=true，BP/ABP编译保存。实际PIE验证进行中。

- 实际Lobby第一轮确认无人机生成/动画有效，但位置出画；BP缩至60cm宽并移到执行官肩侧，Mesh启用角色灯光Channel1。修正后实际截图完整可见且不遮角色或UI。Lobby轮转按游戏时间，验证脚本等待完整周期；隐藏查询改读Actor.hidden。
- 移除无人机对WorldDynamic的阻挡响应，避免拦截玩家弹体；移动体只阻挡静态场景，跟随路径查询仍会检测阻碍。局部避障/路径点不是全局3D寻路。
