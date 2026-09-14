# 当前工作面板

- Active feature：FEAT-082 大厅角色展示。已纠正为两种空手站立待机，待用户在LobbyMap摆放和主观校验。
- FEAT-081与FEAT-080暂停；头领导弹要求不装配但保留，第4批武器／附着弹适配未完成，详见对应archive。

## 本轮完成

- TMIIR复用RTG_RifleToMaintenanceWorker，将Rifle_01的NW_Stand_Relaxed_Rifle_Idle_IP、NW_MOB_Stand_Relaxed_Rifle_Idle_IP两种空手站立待机重定向到大厅维修工骨架，均2.1667秒。源和成品姿态均已查看。
- 仅迁入两个最终AnimSequence：MaintenanceWorker/Lobby/Animations/AS_MaintenanceWorker_Lobby_StandingIdle_01、02；与前批合计39资产。未覆盖模型、骨架或原有动画，不迁入RTG／IKRig／源骨架。
- ALobbyCharacterBase新增Standing姿态、StandingAnimations数组、StandingIdleIndex及SetStandingIdleIndex；BP默认Standing、索引0、隐藏武器。旧Relaxed放松持枪／Rifle举枪保留可选。
- LobbyMap／TestMap布局及既有Body／FirstPerson未修改；没有新增项目内测试代码、地图或保存验证摆件。

## 验证

- standing-build.log：Development Editor Win64 Succeeded。
- standing-target-pie.log／json：STANDING_TARGET_PIE_OK。两个版本动画时钟前进、实际手部姿态不同、武器隐藏、根位置保持；无效索引安全忽略；切到Rifle后返回Standing正常，播放中退出PIE且TestMap哈希不变。StandingTarget-0／1截图已查看。
- 最终冷回读结果见FEAT-082 archive；外部证据在D:/Unreal Projects/CoreMorph57Prep/Saved/Review/standing-*。

## 会话交接

操作前checkpoint644f4a7保留上一批大厅结果，最初旧Mesh可从8eee960恢复。本轮纠正结果未提交／push。蓝图：/Game/Characters/MaintenanceWorker/Lobby/Blueprint/BP_MaintenanceWorker_Lobby；默认Display Pose=Standing，Details的Standing Idle Index用0／1选择；UI调用SetStandingIdleIndex(0/1)即可进入对应站立版本。原Relaxed是持枪放松，不是用户所指空手站立。源RTG留在TMIIR，初次制备／迁移脚本不要重跑。详情见[FEAT-082](archive/FEAT-082-lobby-character-presentation.md)。

预留IA_Test仍为键盘1，未来手动测试统一复用；当前Controller中的场景仍为此前Manta入口。TestMap只保留基础环境。若恢复FEAT-081，应重新核对BP实际PhaseSkillSets；此前卸装仅改原生默认并通过编译，未冷核对BP配置。
