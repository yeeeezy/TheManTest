# 当前工作面板

- Active feature：FEAT-082 大厅角色展示，等待用户在LobbyMap摆放与主观校验。
- FEAT-081／080暂停；导弹不装配但保留，第4批武器适配未完成，详见对应archive。

## 当前结果

- 维修工Lobby目录共40资产：完整展示模型及依赖、两种空手Standing、两种持枪Relax、一种Rifle举枪。
- 最新在TMIIR复用RTG将W2_Stand_Relaxed_Idle_v2_IP导出为AS_MaintenanceWorker_Lobby_RelaxedIdle_02（4.7秒）。普通RelaxedIdle（3.2667秒）此前已存在；Root_Motion版本不重复迁入。
- 新动画路径：/Game/Characters/MaintenanceWorker/Lobby/Animations/AS_MaintenanceWorker_Lobby_RelaxedIdle_02。
- 展示BP默认Standing；Standing Idle Index=0／1切换空手版本。Display Pose=Relaxed时，BP Class Defaults的Relaxed Animation可选普通RelaxedIdle或RelaxedIdle_02。本轮只新增动画，没有修改C++、BP或地图。
- RTG／源骨架／IKRig只留TMIIR，目标仅成品。源与成品姿态已检查，详细验证结果见[FEAT-082](archive/FEAT-082-lobby-character-presentation.md)。

## 会话交接

最新补齐Relax前检查点541d8d6保存上一轮空手结果；本轮未提交／push。展示蓝图：/Game/Characters/MaintenanceWorker/Lobby/Blueprint/BP_MaintenanceWorker_Lobby。初次制作或迁移脚本不要重跑。外部证据在D:/Unreal Projects/CoreMorph57Prep/Saved/Review/relax-*与RelaxTarget-*.png。目标用户编辑器35076保持运行，验证使用独立隐藏进程，不保存瞬态BP配置／地图。新动画可在原Relaxed Animation字段选择，不存在Relaxed索引字段。

预留IA_Test仍为键盘1，当前仍是Manta入口；未来手动测试统一复用。TestMap只保留基础环境。恢复FEAT-081时需冷核对实际BP PhaseSkillSets，此前卸装仅改原生默认并编译。
