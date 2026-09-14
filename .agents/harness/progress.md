# 当前工作面板

- Active feature：FEAT-082大厅展示；FEAT-080/081暂停。三枪独立动画、两把80%大厅副本、挂点与电击食指微调已保存，详见archive。
- WEAPON进入Rifle/近景；CHARACTER从当前枪非空Relaxed动画随机选择并进入远景，允许重复。按钮初始和点击后无常驻高亮，仅hover高亮。
- 最新：用户授权Rifle/Relax使用0.5秒混合。新增LobbyPoseBlendAnimInstance，当前姿势快照向目标动画混合，枪挂点同步SmoothStep。支持中途反向和重复点击，不使用IK。可调参数Lobby | Animation → Pose Blend Duration，默认0.5。

## 验证与交接

- Development Editor Win64构建通过；实际LobbyMap PIE完成15组转换、525帧样本，覆盖三枪两条Relax、双向与途中反向、重复请求、同步挂点及0.5秒时长。局部骨骼旋转最大误差小于0.033度。lobby-pose-blend-validation.json ok=true，地图与资产哈希不变。
- checkpoint2540972保存此前菜单改动；本轮产品仅LobbyCharacterBase.h/.cpp与新增LobbyPoseBlendAnimInstance.h/.cpp。没有动画资产、模型或地图修改，未最终提交/push。
- 相机继续使用原0.7秒过渡；初次显示及切换枪种立即应用，编辑器静态预览仍显示目标姿态。下一步根据用户实际观感调整。
- 已查看三枪放松/过渡中间/举枪9张UE截图，后台编辑器退出；视觉截图脚本收尾JSON序列化问题及修复记录见archive。
