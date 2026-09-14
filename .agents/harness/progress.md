# 当前工作面板

- Active feature：FEAT-082 大厅角色展示。三把正式武器与左手握持返修已完成编译、冷加载、实际PIE和两侧近景自查；等待用户前台观感确认。
- FEAT-081／080暂停；详见对应archive。

## 当前大厅配置

- 正式场景仍为`/Game/Maps/LobbyMap`，唯一`MaintenanceWorker_LobbyDisplay`保持原位置和镜头配置，默认Relaxed。
- Display Weapon Index：0维修枪、1爆破枪、2电击枪。大厅读取正式武器BP的实际显示模型、材质和组件变换；维修枪用SK_SCFRIFLE骨骼模型，另两把用各自静态模型，缩放保持1。不再用Rifle_01替代正式武器。
- 新增大厅专用左手握点IK，放松／举枪分别标定，左手跟随当前帧枪身；右手和原身体动画保持，手臂不拉伸。两条空手Standing隐藏枪并不启用握持校正。
- 复用IA_Test／上方数字1切换Relaxed与Rifle。换展示枪通过实例Display Weapon Index或SetDisplayWeaponIndex；没有新增按键、相机或测试地图。

## 验证与交接

- 最终Development Editor Win64构建通过（lobby-grip-final-build.log）；BP编译保存通过。最终冷加载实际LobbyMap PIE验证3244样本通过，三枪各普通Relaxed／Relaxed v2／Rifle完整循环、两条Standing、无效武器索引和One→Relaxed→Rifle按键链均通过。左手握点最大位置误差1.19e-13cm，右手相对原压缩动画误差小于0.006cm，手腕朝向和两段臂长断言通过。两张地图、全部正式武器和原Lobby动画退出前后哈希不变。
- 爆破枪最终最小可达余量1.7936cm；三枪两种持枪状态的两侧近景已自查。外部证据：CoreMorph57Prep/Saved/Review/three-gun-*、LobbyGripFinal-*。
- 本轮安全检查点f74d947；本轮大厅源码、展示BP和harness结果未最终提交／push。正式武器、原动画、地图和外部源项目未改。下一步用户打开LobbyMap，选展示人物设置枪索引，PIE按1检查观感。
