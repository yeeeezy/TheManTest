# 当前工作面板

- Active feature：FEAT-082大厅角色展示。FEAT-080／081继续暂停。
- CHARACTER／WEAPON菜单、0.7秒相机过渡、小幅鼠标视差、场景CameraComponent与运行时Rig同步已完成，已随6151620推送origin/main。以用户当前地图相机位置为准。
- 最新授权：第二／第三把爆破枪和电击枪复制为大厅专用并缩小，返修独立握持动画。本轮已完成两份80%静态Mesh、两份EquipmentBase展示数据BP和大厅角色BP引用；六条成品动画直接修左前臂／手腕／手指，无IK。
- 三枪各两条放松、一条举枪，共九条独立动画；DisplayWeaponIndex=0维修／1爆破／2电击，数字1切放松／举枪。新副本位于Characters/MaintenanceWorker/Lobby/Meshes，展示BP位于相邻Blueprint目录。

## 验证与交接

- 三个蓝图编译保存；TMIIR六条动画全骨骼逐帧／依赖校验并迁移；正式LobbyMap冷启动PIE2693样本通过，含九种持枪、两种站立、数字1双向切换及模型／材质／尺寸检查。实机双侧截图已查看，用户主观观感待反馈。
- 工程与证据：D:/Blender Projects/LobbyCompactWeapons，主工程LobbyCompactWeapons_Animated.blend、source-grips-verified.json、blender-grip-validation.json(ok=true)；目标日志Saved/Codex/compact-pie.log。
- checkpoint bf5da75已保留此前用户LobbyMap编辑。本轮未改地图、正式武器、维修枪和C++；没有向目标引入源骨架／Rig／Retargeter。辅助后台编辑器已退出。实现尚未最终提交／push。
- 详细尺寸、路径、动画制作方式与验证见archive/FEAT-082-lobby-character-presentation.md最新章节。旧Blender工程LobbyWeaponGrip保留用于原版比较。
