# 当前工作面板

- Active feature：FEAT-082 大厅角色展示。实现／编译／PIE／冷引用检查完成，待用户在LobbyMap摆放和主观校验。
- FEAT-081与FEAT-080暂停；头领导弹要求不装配但保留，第4批武器／附着弹适配未完成。详细历史见对应archive。

## 本轮完成

- 删除用户确认的MaintenanceWorker/Meshes下3个无引用旧Mesh：CyberpunkMetalhead_Arms、CyberpunkMetalhead_Legs、SKM_CyberpunkMetalhead_FullBodyA。可从操作前Git检查点8eee960恢复。
- TMIIR完成Rifle_01→完整维修工的RTG与参考姿势对齐，输出放松持枪／举枪两条3.2667秒循环待机；独立大厅模型、目标骨架、必要布料物理依赖。源模型／源动画／既有RTG保留，新RTG在TMIIR的LobbyPrep/MaintenanceWorker。
- TheManTest仅迁入36个必要成品包并创建维修工展示BP，共37资产，全部在`/Game/Characters/MaintenanceWorker/Lobby/{Blueprint,Meshes,Animations,Materials,Textures}`。源Rifle_01骨架、IKRig、IKRetargeter均未迁入；既有Body／FirstPerson未修改。
- 新结构：ALobbyCharacterBase（AActor）→AMaintenanceWorkerLobbyCharacter→BP_MaintenanceWorker_Lobby。仅模型、循环动画和无碰撞展示步枪；无Pawn／ASC／游戏装备。SetDisplayPose切换Relaxed／Rifle，编辑器Details中也可选择。
- 未修改LobbyMap布局／UI／GameMode或TestMap；用户将展示BP拖入自己的大厅场景即可。

## 验证

- lobby-character-pose-build.log：Development Editor Win64 Succeeded。
- lobby-source-preview-final.log：TMIIR最终两姿态已查看。
- lobby-target-visual.log／lobby-target-pie.json：LOBBY_TARGET_PIE_OK，两动画时钟推进、手部姿态确实不同、切换并返回、根位置不动、武器附着、播放中退出PIE且TestMap哈希不变。最终Relaxed／Rifle截图已查看。
- lobby-target-cold-audit.log／json：37资产全部可加载且/Game依赖全部位于所属Lobby目录；骨架／材质／动画和根位移一致，无源骨架／IKRig／RTG／Redirector。
- 旧Meshes和迁移供应商目录在Registry／磁盘均清空；只有授权3个旧Mesh删除，其余既有Content不变。新资产使用LFS。全部后台编辑器已退出。

## 会话交接

本轮结果未提交／push，操作前检查点8eee960。展示蓝图：`/Game/Characters/MaintenanceWorker/Lobby/Blueprint/BP_MaintenanceWorker_Lobby`；Details的Display Pose选择Relaxed或Rifle，UI调用SetDisplayPose。TMIIR增加的LobbyAssetPreparationLibrary仅为外部编辑器成品制作辅助，不在目标源码或依赖中。外部脚本／证据在`D:/Unreal Projects/CoreMorph57Prep/Scripts`和`Saved/Review/lobby-*`，已完成迁入，不要重跑初次制备或迁移。详情见[FEAT-082](archive/FEAT-082-lobby-character-presentation.md)。

预留IA_Test仍为键盘1，未来手动测试统一复用；当前Controller中的场景仍为此前Manta入口，并非大厅展示开关。TestMap只保留基础环境，不新增独立测试地图或保存测试摆件。若恢复FEAT-081，应重新核对BP实际PhaseSkillSets；此前卸装仅改原生默认并通过编译，未冷核对BP配置。
