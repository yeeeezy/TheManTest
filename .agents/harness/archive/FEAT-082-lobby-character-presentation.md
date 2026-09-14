# FEAT-082 大厅角色展示

## 2026-09-14 补齐持枪Relax待机

- 用户要求持枪Relax的所有版本也重定向导入。Rifle_01有W2_Stand_Relaxed_Idle_IP与W2_Stand_Relaxed_Idle_v2_IP两个版本；普通版已迁入，本次只补v2，Root_Motion对应版本不重复迁入。
- checkpoint541d8d6保存前轮空手站立结果。在TMIIR复用既有RTG，新增最终AS_MaintenanceWorker_Lobby_RelaxedIdle_02（4.7秒），普通版RelaxedIdle（3.2667秒）保留。源成品姿态截图RelaxRetarget-0／1已查看。
- 只迁入一个AnimSequence，依赖仅既有大厅成品Mesh／Skeleton，当前合计40资产。用户在BP的Relaxed Animation选择RelaxedIdle或RelaxedIdle_02，Display Pose选择Relaxed即可展示。默认Standing保持，不新增C++或保存BP／地图。
- 当前目标用户编辑器35076运行中；验证使用独立隐藏编辑器进程，NoSaveConfig／Multiprocess，不保存瞬态BP参数和地图，不关闭用户编辑器。尝试通过本机HTTP读取MCP工具列表曾被自动审批拦截，未执行，改用外部编辑器脚本。
- relax-target-pie.log／json最终RELAX_TARGET_PIE_OK：冷加载40资产、owner-local依赖及无IKRig／RTG／Redirector断言通过，v2骨架与时长正确。实际PIE播放v2、展示武器附着可见、根位置不动，切到Rifle及返回普通Relax正常，退出PIE且TestMap哈希不变，RelaxTarget-Relaxed.png已查看。Git确认唯一Content变化为新增v2，验证进程已退出，用户35076保留。
- 外部验证前两次分别因临时CDO配置未传入PIE、EditDefaultsOnly禁止实例set_editor_property而失败；最终在瞬态展示Actor进入Relaxed后用现有DisplayMesh播放v2验证成品，不改产品代码或保存默认配置。没有声称永久装配v2；用户可自行在BP Class Defaults选择。无C++修改，无需重新编译。

## 2026-09-14 站立待机纠正（当前）

- 用户明确要求空手站立待机，多个版本全部重定向导入；此前Relaxed为持枪放松，并不满足该要求。操作前检查点644f4a7保留上一批结果。
- Rifle_01中确认NW_Stand_Relaxed_Rifle_Idle_IP与NW_MOB_Stand_Relaxed_Rifle_Idle_IP两个空手版本，均2.1667秒，源与重定向成品截图已检查。Root_Motion对应版本不重复迁入；W2持枪和Crouch蹲姿不作为空手站立版本。
- TMIIR复用RTG_RifleToMaintenanceWorker，向既有成品大厅骨架导出AS_MaintenanceWorker_Lobby_StandingIdle_01／02。只迁入这两个AnimSequence，依赖仅为目标已存在的大厅Mesh／Skeleton，未覆盖模型或带入工作资产。
- 基类新增Standing（追加枚举以保留旧值）、StandingAnimations、StandingIdleIndex、SetStandingIdleIndex；默认空手站立并隐藏武器。保留Relaxed／Rifle可选，无效运行时索引忽略。
- standing-build.log：Development Editor Win64 Succeeded。standing-target-pie.json ok=true：两个站立版本实际播放并输出不同手部姿态，武器隐藏，根位置不变；无效索引忽略，切到Rifle再返回Standing正常，播放中退出PIE且TestMap哈希不变。StandingTarget-0／1截图已查看。
- standing-target-cold.log：STANDING_TARGET_COLD_OK，39资产全部可加载且/Game依赖均为所属Lobby目录；四条动画统一成品骨架、无根位移；BP默认Standing和两个索引配置持久化，冷生成Actor隐藏武器。没有IKRig／RTG／Redirector。Git范围仅本轮两个动画、展示BP、两个基类文件与harness，模型／骨架／地图未变，全部后台编辑器已退出。
- 本轮结果未提交／push，待用户主观校验。下方早期Relaxed／Rifle说明为此前持枪批次历史，当前默认和用法以上述Standing配置为准。

用户授权：删除MaintenanceWorker/Meshes下刚确认无引用的三个旧Mesh；从TMIIR重新迁入大厅用完整维修工及Rifle_01待机／持枪动画。RTG仅在TMIIR完成，TheManTest仅接收成品和必要依赖。新建大厅人物展示基类及维修工派生类，专属资产全部归维修工目录。

- 操作前检查点：8eee960；FEAT-081暂停，导弹不装配但保留，第4批未完成。
- 目标结构：ALobbyCharacterBase : AActor → AMaintenanceWorkerLobbyCharacter → BP_MaintenanceWorker_Lobby。基类负责模型、循环姿态和展示武器挂点；具体蓝图负责成品资产。
- 专属资产路径：/Game/Characters/MaintenanceWorker/Lobby/{Blueprint,Meshes,Animations,Materials,Textures}。不覆盖现有Body／FirstPerson资源，不写入展示摆件到TestMap。
- 源检查：TMIIR UE5.7，FullBodyA；Rifle_01 W2_Stand_Relaxed_Idle_IP／W2_Stand_Aim_Idle_IP各3.2667秒。现有RTG_NewRetargeter方向是Cyberpunk→Rifle，与本次相反，需在TMIIR新建正确方向Rifle→Cyberpunk的RTG。
- 删除已完成：完整硬／软／管理／搜索引用（含ExternalActors）为0，EditorAssetLibrary删除三个旧Mesh，日志lobby-old-mesh-delete记录LOBBY_OLD_MESHES_DELETED。可从8eee960恢复。

## 实现与迁移

- TMIIR新建`/Game/LobbyPrep/MaintenanceWorker/RTG_RifleToMaintenanceWorker`，复用源／目标IKRig，目标AutoAlign后导出两条循环待机。源RTG方向相反未覆盖，原模型／动画保留。
- 在TMIIR复制独立大厅Mesh／Skeleton／PhysicsAsset成品；用该工程的`LobbyAssetPreparationLibrary`清除成品骨架的失效Manny／Quinn重定向预览来源、重新绑定成品Mesh与布料碰撞依赖。助手仅在TMIIR的TMIIREditor模块，不迁入TheManTest。源准备脚本首次因UE5.7只读属性／Python未暴露setter失败，最终使用本地C++编辑器API并冷编译通过。最早单纯反向RTG的参考姿势不合适，已AutoAlign并重新导出，不使用早期结果。
- 只迁入36个最终依赖包：完整模型、目标骨架、必要物理资产、2条AnimSequence、展示用M4静态网格及实际材质／纹理依赖。目标通过AssetTools重命名归入维修工Lobby目录，再创建BP_MaintenanceWorker_Lobby，共37资产。没有源Rifle_01骨架、IKRig或IKRetargeter；既有Body／FirstPerson资源不变。
- 基类用OverrideAnimationData处理编辑态预览，用PlayAnimation刷新运行时实例；两个姿态分别配置手部挂点局部变换，来自源动画实际hand_r_wepSocket相对hand_r变换。仅展示静态武器，不生成游戏武器Actor。根位置不消费动画位移。

## 验证

- lobby-character-pose-build.log：目标Development Editor Win64 Succeeded；lobby-source-helper-deps-build.log：TMIIR编辑器辅助API编译成功。
- lobby-source-preview-final.log：两种源成品姿态截图已检查。源第一次预览仅修改瞬态实例导致A Pose／切换没有刷新；改用OverrideAnimationData并强制更新实例后两种姿态正常。
- lobby-target-pie-final.log：LOBBY_TARGET_PIE_OK，实际播放时钟前进、切换Rifle并返回Relaxed、无ASC／Pawn、根位置保持、退出PIE且TestMap哈希不变。首轮失败是外部验证脚本Vector.equals签名错误，已改为向量距离；高分辨率截图期间重入回调的阶段推进已修正。
- 最终`lobby-target-visual.log` LOBBY_TARGET_PIE_OK，增加实际hand_r位置差异验证，放松与举枪确实输出不同姿态。已查看LobbyTarget-Relaxed／Rifle.png，两种姿态、武器、衣服和完整身体正常；截图中的临时地面、灯光和相机仅用于外部验证，不保存到项目地图。
- `lobby-target-cold-audit.log` LOBBY_TARGET_COLD_OK：37资产全部可加载，全部/Game依赖均留在MaintenanceWorker/Lobby内；没有源骨架／IKRig／IKRetargeter／Redirector，骨架、材质、2条动画及根位移检查通过。迁入供应商目录和旧Meshes目录的Registry与磁盘均清空。
- 空目录批量清理命令被自动审批拒绝，未执行；随后只读枚举确认均为空，改为明确列出每个空目录、不递归的Remove-Item完成，未扩大删除范围。
- Git确认只删除授权的3个旧Mesh，新增37个大厅资产／3个C++文件和harness记录；现有Body／FirstPerson、武器、LobbyMap／TestMap未修改。LFS过滤器确认有效。全部后台编辑器退出，操作前8eee960可恢复旧Mesh；本轮未提交／push。

## 使用与交接

- 拖入`/Game/Characters/MaintenanceWorker/Lobby/Blueprint/BP_MaintenanceWorker_Lobby`到用户自己的LobbyMap布局。
- Details → Lobby → Presentation → Display Pose选Relaxed或Rifle；UI可调用SetDisplayPose。Relaxed是放松持枪待机，Rifle是举枪待机。两个循环都长3.2667秒，无根位移；本次只提供人物展示基础，不改既有Lobby UI或地图布局。
- 源RTG留在TMIIR的LobbyPrep/MaintenanceWorker。外部脚本与证据在D:/Unreal Projects/CoreMorph57Prep/Scripts和Saved/Review的lobby-*文件，验证已完成，不必重新批量迁移或运行初次制备脚本。
- FEAT-081与第4批武器适配暂停；此前导弹卸装仅修改原生默认并通过编译，若恢复头领工作应再核对BP实例的PhaseSkillSets实际配置，不把编译当作蓝图运行时已核对。
