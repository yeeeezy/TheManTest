# 当前工作面板

- Active feature：FEAT-082大厅角色展示。用户否决上一批IK握持；已改为每枪独立成品动画，只修左手腕旋转，等待前台观感确认。
- FEAT-081／080暂停，详见对应archive。

## 当前配置

- 正式LobbyMap及人物位置／镜头不变。Display Weapon Index：0维修枪、1爆破枪、2电击枪；读取三枪正式BP的实际模型／材质／组件变换，缩放未改。
- 每枪独立RelaxedAnimations[0/1]和ReadyAnimation，共九条按枪命名的AS_MaintenanceWorker_Lobby动画；RelaxedIdleIndex或SetRelaxedIdleIndex(0/1)选择放松版本，IA_Test／数字1仍切换放松与举枪。
- 大厅自定义IK AnimInstance、左手目标和AnimationCore依赖已删除；标准SingleNode直接播放。TMIIR仅修改hand_l旋转关键帧，原肩／肘／手腕位置与其他局部轨道不变；没有离线IK或新RTG。原五条动画保留，空手Standing仍用原动画。

## 验证与交接

- 目标与TMIIR辅助模块Development Editor编译通过；BP三枪动画引用编译保存通过。wrist-animation-validation实际LobbyMap PIE通过2806样本，九条持枪循环、两条Standing、模型材质缩放、原手臂位置、无效武器索引和One按键链均通过。运行时位置相对原Raw姿态最大差0.007172cm，旋转差0.179115度包含压缩误差。
- 三枪普通／v2放松和举枪近景已检查；截图为外部CoreMorph57Prep/Saved/Review/LobbyWristFinal-*。不以数值测试替代用户观感确认。wrist-animation-cold-final冷读50资产通过：九条成品均绑定本枪，所有骨骼局部位置和其他骨骼旋转保持，手腕旋转与外部制作规格一致，依赖只有既有大厅Mesh／Skeleton，无Rig／RTG／Redirector。全部后台验证编辑器已退出。
- checkpoint77b2434；本轮九条成品、展示BP、基类源码和harness未最终提交／push。正式武器、原动画、地图未改。源助手在TMIIR，备份及脚本在CoreMorph57Prep，目标不接收制作资产。
- Phantom当时只恢复hand_r_wepSocket和枪相对scale1；这次采用直接修动画是用户新要求，不是复用旧IK方案。
