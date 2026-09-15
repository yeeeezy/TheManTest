# FEAT-082 大厅角色展示

## 2026-09-14 三把正式武器与双手握持返修（实现与验证完成）

- 用户截图再次指出左手穿入护木，明确要求使用维修枪、爆破枪、电击枪，并授权自行调查尺寸和修复。checkpoint f74d947 保存此前大厅返修结果。之前“原配Rifle_01握持通过”的视觉结论被用户截图推翻。
- 实测正式 BP_RepairGun 的 StaticMesh 是95.29cm旧合并模型，但正式 Equip 优先使用 SkeletalMesh SK_SCFRIFLE，参考包围盒枪长76.80cm，三槽材质 MI_SCFR/MI_SCFR2/MI_SCFR1。三枪根和组件缩放均1；爆破/电击静态组件分别有Y=-16.757669/-27.277509偏移。不能用旧静态代理模型大小代表正式维修枪。
- 正在添加大厅专属武器展示配置：从正式武器CDO读取实际静态/骨骼模型、材质和组件变换；按武器校准右手挂点与左手握点。原生SingleNode动画代理在同帧右手骨骼空间求解左臂TwoBoneIK及手腕朝向，Standing不校正，不拉伸手臂。这是运行时握持IK，不是重定向；未向TheManTest导入源骨架/IKRig/Retargeter。
- 首次Development Editor Win64构建已通过，最终资产校准、完整循环和近景视觉验证尚未完成。证据目录沿用外部CoreMorph57Prep/Saved/Review的three-gun-*、fp-grip-inspection和GunGeometry-*。
- 中间问题与验证：Mesh索引误作SkeletonPose索引导致原生IK未作用，改用FBoneReference后3228样本验证通过。随后爆破枪厚护木握点向下校正，最终PIE发现Relaxed超出自然臂长2.018cm；不能放宽误差或拉伸手臂，正在单独后移Relaxed握点并增加三枪所有持枪动画逐帧可达裕量检查。此中间失败记录不得当作最终验收通过。

- 最终结果：最终Development Editor Win64构建通过（lobby-grip-final-build.log）；BP编译保存通过。最终冷加载实际LobbyMap PIE验证3244样本通过，三枪各普通Relaxed／Relaxed v2／Rifle完整循环、两条Standing、无效武器索引和One→Relaxed→Rifle按键链均通过。左手握点最大位置误差1.19e-13cm，右手相对原压缩动画误差小于0.006cm，手腕朝向和两段臂长断言通过。两张地图、全部正式武器和原Lobby动画退出前后哈希不变。
- 爆破枪厚护木的左手握点Z相对源参考下移5.5cm，沿枪身分别标定Relaxed与Rifle，所有九种持枪组合采样101个相位检查可达性；最小臂长余量1.7936cm，未拉伸手臂。原始约束失败已修复。
- 最终两侧近景：LobbyGripFinal-{0,1,2}-{0,1}-{-1,1}.png；爆破枪最新可达配置另见three-gun-reach-visual.log，其余两枪图来自three-gun-final-visual.log。冷编辑器打开LobbyMap已是Relaxed、索引0、原生LobbyCharacterAnimInstance和可见SK_SCFRIFLE，旧静态显示组件为空且隐藏（three-gun-lobby-cold-audit.json）。
- 本轮仅大厅C++、BP和harness改动，AnimationCore为唯一新增私有依赖；地图、三枪BP/模型/材质、原动画及外部资源项目未改。未提交最终结果或push。用户选择大厅人物实例的Display Weapon Index：0维修枪、1爆破枪、2电击枪；PIE上方1继续只切放松／举枪。

## 2026-09-14 最终持枪视觉返修与关机交接

- 用户指出枪过大、白膜和握持错位，要求自行验收后关机。之前仅有按键／状态通过不能代表视觉通过。操作检查点见Git最新WIP checkpoint before final lobby grip review。
- 最终BP使用Lobby自有SM_MaintenanceWorker_Lobby_Rifle／M_MaintenanceWorker_Lobby_Rifle，原配Rifle_01枪长约83.5cm，替换95.3cm且偏白的RepairGun合并静态网格。两套Transform恢复lobby-source-pose-check.json中源动画实测值，附着hand_r，不再组合RepairGun自身偏移。直接挂目标weapon骨骼的中间方案因方向错误撤回，未作为最终结果。未重新重定向或改动画序列，未改玩法RepairGun。
- 展示实例Yaw改为180面向远景；原Far／Near相机重新瞄准人物，更新实际对焦距离，Near焦距30mm。地图未新增相机。
- 最终LobbyFinalFar-Relaxed/Rifle、LobbyFinalNear-Relaxed/Rifle截图逐张查看；枪有完整黑色纹理，两个姿态右手握柄、左手托护木。LobbyGripFinal截图是临时PIE检查视角，未保存该临时相机变换。
- 冷重启后lobby-grip-cold-pie.json ok=true（360运行时样本）：40个Lobby成品加载，原配Mesh/材质/hand_r挂点持久化，原嵌套地图不存在且新BuiltData存在。实际One按键Relaxed→Rifle→Relaxed，各保持4秒覆盖完整循环；枪可见，退出PIE后地图哈希不变。日志lobby-grip-cold-pie.log；所有证据位于D:/Unreal Projects/CoreMorph57Prep/Saved/Review。
- 本轮仅改展示BP和LobbyMap及harness；沿用上轮已成功编译的C++，未新增源码或永久测试。自动验证编辑器已经正常退出。用户明日直接打开LobbyMap、PIE按上方1验收。结果未最终提交／push。


## 2026-09-14 正式大厅接线与测试入口

- checkpoint `7439d2a`保存恢复的328个场景资产与此前harness状态。用户确认将正式选角场景改名为`/Game/Maps/LobbyMap`并删除旧同名空地图；场景依赖保留在`/Game/Maps/SciFiIndustrialBase`，1.4GB预计算数据改为`/Game/Maps/LobbyMap_BuiltData`。旧嵌套地图和BuiltData路径均删除；GameInstance原有`LobbyMapName=LobbyMap`无需改代码即可进入正式大厅。
- 正式大厅在原角色焦点`(0,0,0)`放置唯一`MaintenanceWorker_LobbyDisplay`（BP_MaintenanceWorker_Lobby），实例初始Relaxed持枪。地图保持BP_CharacterSelectGameMode、CharacterSelectCameraSwitcher与原四个CineCameraActor，总Actor数1977。
- CharacterSelectPlayerController新增TestAction并仅在WITH_EDITOR绑定HandleTestInput；BP配置IA_Test，IMC_CharacterSelect配置One。每次按键查找ALobbyCharacterBase并切换SetWeaponReady，战斗PlayerController的CoreMorph测试入口未改。
- Development Editor Win64冷编译Succeeded。`lobby-display-input-pie.json`为ok=true：实际LobbyMap PIE使用BP_CharacterSelectPlayerController，第一次One从Relaxed进入Rifle，第二次返回Relaxed；唯一展示Actor保持，退出PIE后LobbyMap哈希不变。日志包含两次`[LobbyTest]`状态和`LOBBY_DISPLAY_INPUT_PIE_OK`。

## 2026-09-14 新版选角大厅恢复

- 用户指出当前`/Game/Maps/LobbyMap`是弃用的旧空地图，新版大厅已被删除。审计确认LobbyMap从初始提交到全部可达／悬空Git提交始终只有同一48,850字节旧版本，Autosaves、Backups和回收站没有新版LobbyMap；本轮大厅角色验证脚本也始终只加载TestMap且验证哈希，不曾保存LobbyMap。
- 根据FEAT-045历史继续定位，找到2026-08-01提交a03f30d删除的`/Game/Maps/SciFiIndustrialBase/Maps/SciFiIndustrialBase`及327个同目录依赖。该8.8MB地图Git LFS对象含CharacterSelectCameraSwitcher、FarCamera、NearCamera、CineCameraActor与BP_CharacterSelect字符串，确定为用户完成过的新选角大厅。
- 恢复前checkpoint197788a保存举枪展示批次。随后从`a03f30d^`恢复完整`Content/Maps/SciFiIndustrialBase`，共328文件，保持原路径，不覆盖旧LobbyMap。
- `restored-character-select-map.log/json`：RESTORED_CHARACTER_SELECT_MAP_OK。地图冷加载成功，328资产全部加载，场景1976 Actor；找到CharacterSelectCameraSwitcher与4个CineCameraActor。验证没有保存地图，全部后台编辑器已退出。
- 当前只恢复内容，未把GameInstance的LobbyMapName或项目启动流程改到新地图；旧流程仍可能指向弃用LobbyMap，接线需用户另行确认。恢复结果未提交／push，整个SciFiIndustrialBase目录当前作为Git恢复改动存在。

## 2026-09-14 可复用举枪接口与维修枪展示（进行中）

- 用户确认在ALobbyCharacterBase新增所有大厅人物可复用的SetWeaponReady(bool)，true进入Rifle举枪，false进入Relaxed持枪；IsWeaponReady供UI读取。空手Standing与SetDisplayPose保持。
- 操作前checkpoint832fa14。BP_MaintenanceWorker_Lobby的DisplayWeapon已改用既有`/Game/Weapons/RepairGun/Meshes/SM_RepairGun_Rifle`，不复制武器资产。仍附着hand_r；根据源动画hand_r_wepSocket相对hand_r的两套现有变换，再组合维修枪BP静态模型自身(-0.000656,-5.097503,3.554176)偏移，得到Relaxed／Rifle专属Transform，切换时同步应用。
- MCP在用户当前编辑器内完成BP编译保存；代码已写入LobbyCharacterBase.h/.cpp。用户关闭编辑器后，`lobby-weapon-ready-build.log` Development Editor Win64编译成功。
- 初次PIE确认接口、动画和偏移均正确，但维修枪显示为白色；检查发现正式BP_RepairGun在组件层覆盖`MI_RepairGun_Rifle`，随后同步该材质到DisplayWeapon并保存BP。最终截图仍呈亮白／浅灰，这是正式材质自身外观，不是缺失材质；冷回读确认材质引用存在。
- `lobby-weapon-ready-final.log`／json：LOBBY_WEAPON_READY_PIE_OK。默认Standing后调用SetWeaponReady(false)进入Relaxed且枪可见，再true进入Rifle且位置变化17cm以上；连续false／true／false均正确，动画持续播放、根位置不动、退出PIE且TestMap哈希不变。LobbyWeapon-Relaxed／Ready截图已检查。
- `lobby-weapon-ready-cold.log`：LOBBY_WEAPON_READY_COLD_OK，40个大厅资产可加载，无IKRig／RTG／Redirector；BP冷回读含SetWeaponReady／IsWeaponReady、正式RepairGun Mesh和MI_RepairGun_Rifle。允许的外部依赖仅归正式RepairGun所有者；未复制武器资产。
- Git范围：LobbyCharacterBase.h/.cpp、BP_MaintenanceWorker_Lobby及harness；地图、UI、玩法维修枪资产未修改。全部后台编辑器已退出，本轮未提交／push。

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

## 2026-09-14 用户否决 IK，改为逐枪手腕动画

- 用户明确要求三枪各用独立修正动画、保留原上臂／前臂位置，直接修手腕旋转，不使用IK。此前IK数值验证只说明到达指定目标，不能代表自然握持；上一批观感结论撤回。
- 操作前checkpoint77b2434。已删除LobbyCharacterAnimInstance及大厅AnimationCore依赖，恢复OverrideAnimationData／PlayAnimation；每枪改为RelaxedAnimations两版本和ReadyAnimation。新增RelaxedIdleIndex／SetRelaxedIdleIndex。目标wrist-animation-build.log编译成功。
- Phantom历史核实：FEAT-080的2026-09-05握持修复只恢复hand_r_wepSocket与枪相对scale1，未修改动画或启用IK。不能把那次描述成手腕烘焙。
- TMIIR外部助手只修改hand_l局部旋转关键帧，对全部轨道逐帧核对位置／缩放完全保留，其他骨骼旋转不变。制作九条独立成品；不使用运行时或离线IK。本轮源助手编译通过，源动画创建和冷读验证通过；仅九条成品通过AssetTools迁入，依赖为既有大厅Skeleton／Mesh。首轮放松手掌过度遮挡，再在TMIIR调整六条放松序列的手腕朝向并迁入覆盖。
- BP已编译保存三枪独立动画引用。原五条动画、正式武器BP／模型／材质／比例、右手挂点和地图均不修改。完整循环／输入验证见本节后续记录。

- 最终验证：wrist-animation-validation.log／json为WRIST_ANIMATION_VALIDATION_OK，实际LobbyMap PIE共2806样本；三枪各两放松／一举枪完整循环、两个Standing、标准SingleNode类、正式模型／材质／缩放、原肩肘手腕位置、无效枪索引和One→Relaxed→Rifle通过。原身体位置相对Raw姿态最大0.007172cm，旋转差0.179115度属于运行时压缩范围。测试前后地图、正式武器和原动画哈希不变。
- wrist-animation-cold-final.log／json：WRIST_ANIMATION_COLD_OK，50资产全部加载，九条引用独立且全部局部骨骼位置／非hand_l旋转相对原动画保持；hand_l旋转与TMIIR制作规格一致，只有既有大厅Mesh／Skeleton依赖，无IKRig／RTG／Redirector。初次额外要求迁移前后整包字节一致不成立（AssetTools迁移会重新序列化包），改用逐骨骼原始姿态和明确制作旋转验证，不能把包哈希差异误判为动画差异。
- 已查看三枪最终普通／v2放松及举枪近景，肘部保持原弯曲，手掌改为按各枪形状包覆；图为外部LobbyWristFinal-*。本次遵从只改手腕旋转，没有把手臂拉向固定握点；最终主观观感仍待用户确认。全部后台编辑器退出，无最终提交／push。

## 2026-09-14 截图102535返修，Blender对照Phantom重新摆握持（进行中）

- 用户截图102535明确显示维修枪举枪时左手腕穿进机匣。上一轮仅旋转手腕的结果被否决；数值验证不能说明握持正确。用户允许修不好的动作重新生成，并指定参考Phantom调整好的视觉姿态。
- checkpoint4d581c3。未改目标C++、BP、武器模型尺寸／材质／挂点或地图。独立工程位于D:/Blender Projects/LobbyWeaponGrip；实际人物和三枪FBX、Phantom当前OriginalRifle人物／枪／姿态只读导出，用于同时摆放检查。导出初试commandlet缺MeshObject崩溃，改完整隐藏Editor启动导出成功；未关闭用户编辑器／保存地图。
- Blender中按Phantom掌心／手指姿态参考重新进行FK摆姿。保留上臂与肘部位置，旋转lowerarm_l使手腕离开机匣，再调整hand_l及拇指／四指前两节；不使用IK求解、运行时约束或新Retargeter。三枪两放松／一举枪分别制作，帧数99／142／99，30fps，原循环身体动作保留。
- 已保存LobbyWeaponGrip.blend静态对照和LobbyWeaponGrip_Animated.blend九个命名角色Action及配套枪动画。已出两侧清晰预览，自动打开维修枪侧视图；Phantom-Comparison为参考同场对照。Blender骨骼位置与导出制作数据检查一致。
- TMIIR辅助模块ApplyLocalRotationOffsets已编译通过；只在外部成品副本写入已在Blender摆好／烘焙的12个骨骼旋转修正，位置／缩放轨道保留。目标只接收九条现有路径成品动画；最终源冷读、迁移、目标PIE与侧面近景验证进行中。

- 最终源验证：verify-migrate-grips.log为BLENDER_GRIPS_VERIFIED_AND_MIGRATED_OK，9条各99／142／99关键帧共1020帧，所有骨骼局部位置误差0，旋转与Blender已摆好规格一致；依赖只有既有大厅Mesh／Skeleton。没有新Rig／Retargeter。目标九条原路径成品已覆盖，BP引用无需修改。
- 最终实际PIE：validate-grips.log为BLENDER_GRIP_VALIDATION_OK，共2816样本，九持枪／二空手全循环及One→Relaxed→Rifle、无效索引、模型材质尺寸、标准SingleNode、修正的前臂手掌手指与原上臂肘部位置／右手身体输出均通过。不是旧版“手腕位置不动”的测试；现增加实际手腕偏离旧错位姿势超过4cm和当前成品输出检查。验证前后地图／武器／动画哈希保持。
- 已查看LobbyBlenderGrip三枪放松／举枪实机近景，与截图102535对照，前臂不再穿入机匣，手掌改为从下方支撑。保留用户视觉验收，不宣称每个接触面已做逐三角形无穿透证明。所有后台编辑器退出，无最终提交／push。工作区另见用户10:27:14保存的LobbyMap.umap变更；本轮未保存地图、不回退该改动。


## 2026-09-14 CHARACTER / WEAPON 展示菜单（实施中）

- 用户接受Blender三枪握持，随后授权按YouTube OZObfPgL_5A约30秒和最新三张截图制作展示导航。视频26至40秒已读取，参考左侧纵向深灰长按钮、浅色带字距文字和金色细描边，以及全身到局部的镜头过渡。
- 写入前checkpoint cf805ce保存前轮动画和用户LobbyMap变更。当前仅调整正式LobbyMap两台既有相机／Switcher，保留用户人物位置与其他场景对象。
- 新增可编辑UMG WBP_LobbyPresentation，父类ULobbyPresentationWidgetBase负责按钮事件与选中态；BP_CharacterSelectGameMode原UI为空，现引用新菜单。旧WBP_CharacterSelect继续负责选角开局，不复用其启动游戏按钮。
- 镜头原地图鼠标位移30／18、额外推进500、低阻尼5。现鼠标5／3，近景乘0.5；移除累加速度弹簧，以0.7秒smoothstep同步位置／旋转／焦距／手动对焦距离。相同目标忽略，切换中反向从当前画面继续，不从旧端点起跳；保留鼠标偏移为出发画面，过渡期间暂停新视差。
- Controller新增明确的SetWeaponPresentationView；移除背景点击切换的输入绑定。三枪动画／武器资源保持。Editor新增空布局初始化助手TheManLobbyAssetLibrary，UMGEditor仅编辑器依赖，Slate/SlateCore运行时供按钮样式使用。初次编译TObjectPtr推导错误已修，Development Editor Win64编译成功；UMG和GM已编译保存。实际PIE与截图验证继续。


### 展示菜单最终验证与交接

- Development Editor Win64最终lobby-menu-build.log Succeeded；新UMG和GM在编辑器编译保存。UI字体改为持久化引用/Engine/EngineFonts/Roboto，解决首轮截图缺字。UMG布局可在Designer编辑，助手不在运行时创建布局。
- 正式镜头最终画幅36×20.25mm，Far焦距26mm、位置人物原点+(14,-300,102)、看向原点+(70,0,96)；Near40mm、位置原点+(240,-165,138)、看向原点+(12,0,131)。以用户人物实际Z=24.121307为基准；先前44mm版本爆破枪枪口贴边，最终40mm和取景中心已纠正。原地图环境／角色位置／武器缩放／持枪动画保持。
- 最终冷启动真实LobbyMap PIE：Saved/Codex/lobby-menu-validation.log为LOBBY_MENU_VALIDATION_OK，json ok=true，540镜头样本。三枪各Relaxed／Rifle共六组、12次远近切换实际UMG Button.OnClicked绑定调用通过；重复点击不重启过渡，0.75秒内落到目标、无越界回弹、焦距连续且到位，中途反向无位置跳变，鼠标角落位移5.78755cm低于5／3cm合成上限；背景输入不切镜头。退出后地图、GM、UI、全部大厅动画包哈希不变。
- 已逐张查看最终Final-LobbyMenu-Weapon-{0..5}00002.png，0／1／2为维修／爆破／电击举枪，3／4／5为三枪放松；三枪枪口、握持均入镜，菜单清楚且不遮枪。全身视角Final-LobbyMenu-Character00002.png。截图为实机PIE含UI，未用假合成替代，位于Saved/Codex。数字1仍独立切姿势，菜单本身仅切相机。
- 测试用Python脚本和日志只在Saved/Codex，不新增测试地图或永久验证对象。所有后台编辑器已退出，未关闭用户编辑器（实施期间未发现用户编辑器进程）。Git范围为菜单UMG、GM引用、LobbyMap两镜头／Switcher、相关C++与harness，无动画或正式武器资产改动。未最终提交／push，等待用户实际观感反馈。


## 2026-09-14 场景相机预览与实际视图统一

- 用户指出实际视角与摆放相机不一致并授权修复。checkpoint82e1b44保存此前菜单、地图和UI工作区。此次仅改CharacterSelectCameraSwitcher.cpp与harness，不保存地图／资产。
- 每帧刷新Far／Near的CameraComponent世界变换，包含组件相对偏移，代替仅BeginPlay缓存Actor变换。运行中移动目标相机或组件后，已落位Rig同步更新。
- 补齐ProjectionMode、宽高比约束和轴约束、FOV LOD、正交取景字段、Overscan／AsymmetricOverscan／裁切／分辨率选项、PostProcessSettings与BlendWeight同步。保留既有Cine画幅／焦距／对焦同步和0.7秒过渡；鼠标视差仍为有意叠加，比较基准为关闭视差或鼠标居中且过渡结束、同一视口尺寸。
- lobby-camera-parity-build.log Development Editor Win64 Succeeded。validate_camera_parity.py实际LobbyMap PIE，比较源组件和Rig的GetCameraView输出：远／近位置旋转、FOV、宽高比／约束、投影／正交宽度、后处理权重一致；运行中改变Actor和组件位置、焦距、约束、overscan、vignette／PP权重后仍一致；返回远景清除近景参数。lobby-camera-parity.json ok=true／log CAMERA_PARITY_OK。
- 已查看CameraParity-Rig00000.png与CameraParity-PlacedCamera00000.png，同一视口分别通过运行时Rig和直接场景相机渲染，构图一致。测试首轮Python Rotator.equals不存在，改成轴角数值比较后通过；没有以脚本错误宣称产品失败。测试仅瞬态关闭视差和暂停角色用于对照，退出后LobbyMap SHA256不变，后台编辑器全部退出。无最终提交／push。

- 用户随后授权提交到远端；fetch确认origin/main相对本地为0落后／28领先。本次正常提交并推送当前main全部待发布历史、相机一致性修复和LFS资源，不squash／force push。产品验证沿用已通过的编译及四组PIE，不在发布过程中改代码或资产。


## 2026-09-14 两把大厅专用紧凑枪与握持返修

- 用户明确要求将第二／第三把枪复制为大厅专用并缩小，同时返修爆破／电击握持。写入前checkpoint bf5da75保留用户LobbyMap改动；本轮不再保存地图。
- Lobby/Meshes新增SM_MaintenanceWorker_Lobby_ExplosionGun、SM_MaintenanceWorker_Lobby_ElectricGun，复制原静态模型并将所有LOD BuildScale乘0.8。长度分别从95.2903→76.2323cm、92.8979→74.3183cm；材质继续引用原资源。正式武器和维修枪资源不变。
- Lobby/Blueprint新增BP_Lobby_ExplosionGunDisplay、BP_Lobby_ElectricGunDisplay，父类EquipmentBase，仅作为展示数据CDO使用，不生成战斗Actor。配置专用StaticMesh、原材质和0.8倍组件平移；BP_MaintenanceWorker_Lobby的WeaponPresentations[1/2]引用它们，两种Attachment平移同乘0.8，组件与Attachment缩放仍1。相对hand_r整体缩小，避免枪围绕原模型远端原点漂移。索引0仍直接读取正式RepairGun。
- Blender源工程D:/Blender Projects/LobbyCompactWeapons/LobbyCompactWeapons_Animated.blend，六条独立Action覆盖两枪各RelaxedIdle／RelaxedIdle_02／RifleIdle。保留原上臂、肘部枢轴、右臂和全身动画，仅直接编辑左前臂／手腕／手指局部旋转；爆破枪放松托手避开厚枪身，电击枪第二指节收拢32度。没有IK约束或运行时IK。
- 使用TMIIR已有ApplyLocalRotationOffsets工具烘焙六条成品，在TMIIR冷启动逐帧验证所有骨骼位置／旋转及依赖，再仅迁移六条最终AnimSequence到目标。原九条每枪独立动画关系保留；维修枪三条不改。未迁入源骨架、源Mesh、Rig或Retargeter。
- 三个蓝图在编辑器编译保存；冷启动正式LobbyMap PIE完成2693样本，三枪九种持枪动画、两种空手站立、数字1双向切换通过。实际模型路径、材质、0.8倍bounds、单节点动画类、关键骨骼输出验证通过；肩肘／右手位置误差最大约0.007cm，验证退出后地图／正式武器／动画包哈希不变。未改C++，无需构建。
- 证据：D:/Blender Projects/LobbyCompactWeapons/source-grips-verified.json、blender-grip-validation.json(ok=true)，Saved/Codex/compact-pie.log(BLENDER_GRIP_VALIDATION_OK)。已查看Blender双侧预览和UE实际双侧握持截图，实机截图前缀LobbyCompactGrip-。仅实现和客观检查完成，最终观感待用户反馈。
- 早期脚本对只读struct属性直接赋值失败，改用set_editor_property后编译保存成功；未把脚本失败当作完成。产品改动未最终提交／push。此前相机菜单已通过6151620推送到origin/main；旧归档中的待推送文字属历史。


## 2026-09-14 右手握柄重新标定

- 用户15:24两张截图指出爆破／电击右手握柄不贴合；上一轮2693样本仅证明播放／引用／骨骼输出正确，不能证明手掌与模型接触正确。用户批准先校准每枪挂点、再修左手成品动画，不用IK，不新增骨架插槽。
- checkpoint7b9c56d保存上一轮大厅副本和动画。Blender新工程D:/Blender Projects/LobbyGripAnchors，通过原模型正侧视坐标和右手双侧近景标定；保持80%模型尺寸与hand_r，分别调整WeaponPresentations[1/2]的RelaxedAttachment／ReadyAttachment平移。
- 相对于前轮枪局部坐标，爆破放松平移(0,-2.4,+5.6)cm、举枪(0,0,+4.8)cm；电击放松(0,0,+4.8)cm、举枪(0,+1.6,+4.8)cm。最终hand_r相对挂点完整变换保存在mounts.json；不是直接把这些枪局部增量写进手骨局部坐标。旋转经近景检查保留原值。
- 同步重制两枪各RelaxedIdle／RelaxedIdle_02／RifleIdle共六条左手FK动画，保留全身、上臂、肘部枢轴和右手骨骼。右手通过移动枪到掌心校准；左手通过独立前臂／手腕旋转重新托枪。旧80%Mesh与两个展示数据BP未再修改。
- 六条Blender动作已烘焙保存为LobbyGripAnchors_Animated.blend，TMIIR冷回读全骨骼逐帧与依赖验证后仅迁移六条最终动画。大厅角色BP已编译保存。正式LobbyMap PIE挂点矩阵、动画及两侧/右手特写检查已完成，结果如下。

- 最终冷启动实际PIE通过2602样本，三枪九种持枪动画、两种站立、数字1切换均正常；新增断言确认实际枪组件变换等于Blender最终mounts.json组合源组件变换。全部骨骼／引用／材质／80%模型尺寸检查通过，验证退出后相关资产与地图哈希保持不变。
- 已查看爆破／电击举枪与放松（含Relaxed02）的右手近景，及两侧托枪画面。右手握柄高度较旧版明显校正，左手重新贴合枪身下方／侧面。截图D:/Blender Projects/LobbyGripAnchors/LobbyAnchorGrip-{1,2}-{Relaxed,Relaxed02,Rifle}-{Left,Right,RightHand}.png；验证blender-grip-validation.json ok=true，日志Saved/Codex/grip-anchors-pie.log为BLENDER_GRIP_VALIDATION_OK。
- 本轮只改大厅角色BP的四组挂点和六条成品动画；模型、展示数据BP、维修枪、正式武器、地图、骨架和C++未改。无IK。后台编辑器已退出。最终视觉仍可由用户近景复核，本轮未最终提交／push。


## 2026-09-14 电击枪扳机食指返修

- 用户15:46:12特写指出电击枪食指仍在下方大护圈，未进入上方小扳机口；上一轮右手握柄校准没有解决食指位置，不能以握柄对齐代替扳机验收。
- checkpoint dad3466保存上轮挂点和左手修复。本轮只编辑电击枪三条独立动画的index_01_r／index_02_r／index_03_r局部旋转，保留挂点、枪模、hand_r、其余右手骨骼和左手修复，不使用IK。
- Blender工程D:/Blender Projects/LobbyElectricTrigger/LobbyElectricTrigger_Animated.blend，明确抬起近节并调整中/末节方向，使指尖进入上方扳机口；已查看放松/举枪近景。对比上一轮baked-grips.json，既有左手旋转偏移逐项不变，仅新增右食指三节。三条动作完成烘焙，TMIIR成品和目标PIE验证已通过。

- TMIIR冷回读三条动画全帧、全骨骼位置/旋转和依赖校验通过，仅迁移电击枪三条成品。目标正式LobbyMap PIE653样本通过，包含食指三个骨骼实际输出、原挂点矩阵、三种姿态与数字1切换；验证退出后包哈希不变。
- 已逐张查看三种姿态的UE右手特写，指尖进入上方扳机口。证据D:/Blender Projects/LobbyElectricTrigger/source-grips-verified.json、blender-grip-validation.json(ok=true)、LobbyTriggerGrip-2-{Relaxed,Relaxed02,Rifle}-RightHand.png；日志Saved/Codex/electric-trigger-pie.log。旧LobbyGripAnchors特写仍是食指未修版，不作为本次结果。
- git diff确认本轮产品只改三条ElectricGun动画，没有改BP、挂点、Mesh、地图、其他枪或C++。后台编辑器已退出；未最终提交/push。


## 2026-09-14 电击枪指尖高度微调

- 用户反馈仍差一点，并明确选择“指尖还偏低”。截图目录没有新增图，最新仍15:46:12；基于已生成当前版本的正侧/俯视扳机特写微调，不误用旧截图当作新状态。
- checkpoint4d1cbb5保存上轮食指修复。此次仅进一步抬起index_02_r和index_03_r局部旋转，近节index_01_r与其他骨骼、挂点和模型保持。三条动作偏移与上轮逐项对比确认只有这两节改变。Blender工程D:/Blender Projects/LobbyElectricTriggerFine/LobbyElectricTriggerFine_Animated.blend。
- 已检查TriggerDetail-Side/Top，指尖从靠近扳机口下沿进一步上移。三条成品已烘焙，TMIIR全帧校验与实际PIE复核通过。

- 最终实际LobbyMap PIE651样本通过，检查三姿势食指骨骼/挂点矩阵及数字1切换，退出包哈希不变。已查看正侧Blender放大图和UE举枪/放松特写，指尖在扳机口内较上轮小幅上移。证据目录LobbyElectricTriggerFine，blender-grip-validation.json ok=true，Saved/Codex/electric-trigger-fine-pie.log。
- 产品仅三条ElectricGun动画改动，其他资产/C++无变化。后台验证编辑器退出，未最终提交/push，待用户近景反馈。


## 2026-09-14 按钮联动姿势与仅悬停高亮

- 用户授权WEAPON进入Rifle，CHARACTER进入Relaxed且多条动画随机选择；初始无高亮，仅hover高亮。checkpoint7a4ed42保存上轮食指微调。
- CharacterSelectPlayerController::SetWeaponPresentationView同步首个大厅展示Actor姿势与远近镜头：WEAPON调用SetWeaponReady(true)，CHARACTER从当前枪非空RelaxedAnimations中均匀随机选一条调用SetRelaxedIdleIndex；没有可用条目时走SetWeaponReady(false)后备。每次点击CHARACTER重新选择，允许随机重复；选中动画按原SingleNode循环。
- LobbyPresentationWidgetBase去掉选择态ApplySelection，初始化统一中性Normal边框，Pressed使用Normal，Hovered继续使用UMG配置的高亮；点击不改变Normal，不保留选中高亮。
- Development Editor Win64构建Succeeded。首轮局部Character变量遮蔽Controller成员，改名LobbyCharacter后通过。实际LobbyMap PIE三枪分别点击WEAPON和64次CHARACTER，均进入对应姿势，随机集合覆盖0/1，实际AnimSequence与当前枪配置一致；初始与点击后的Normal无高亮，实际移动鼠标触发hover/移出通过。UI BP在编辑器编译，未保存资产。
- 验证Saved/Codex/lobby-pose-menu-validation.json ok=true，lobby-pose-menu-build.log Succeeded，lobby-pose-menu-pie.log POSE_MENU_OK；PoseMenu-Hover00000.png/Neutral00000.png已查看。首轮测试所需SlateBlueprintLibrary未暴露，改为依据已知布局移动真实指针后通过。
- 用户同时询问Rifle/Relax是否有过渡：当前OverrideAnimationData+PlayAnimation仍是硬切，只有相机0.7秒smoothstep。已建议约0.3秒姿势混合并同步枪挂点平滑；此轮仅回答并提出建议，尚未实现角色动画混合，等待用户决定。
- 本轮只有Controller.cpp、LobbyPresentationWidgetBase.h/.cpp与harness变更；无地图/资产改动，未最终提交/push。后台验证编辑器已退出。


## 2026-09-14 Rifle / Relax 0.5秒混合

- 用户明确授权0.5秒混合；checkpoint2540972保存此前按钮联动与hover样式。
- 2026-09-14：运行时新增ULobbyPoseBlendAnimInstance（继承UAnimSingleNodeInstance），从当前显示的局部骨骼姿势快照向继续播放的目标成品动画混合；ALobbyCharacterBase.PoseBlendDuration默认0.5秒，SmoothStep进度同步静态/骨骼枪挂点。反向切换重新捕获当前姿势，重复相同状态不重启。初次显示及换枪立即应用；编辑器静态预览保留OverrideAnimationData/PlayAnimation。没有IK，既有每枪独立成品不变。
- Development Editor Win64构建Succeeded（lobby-pose-blend-build.log）。实际LobbyMap PIE三枪、两条Relax版本、双向切换及中途反向通过：15组完成记录、525帧样本，关键手臂/手指局部旋转与期望混合最大误差0.033度以内；切换瞬间姿势/挂点连续，挂点每帧同步，重复请求不重启，0.5秒完成后关闭Actor Tick。退出后地图与大厅资产哈希不变。证据Saved/Codex/lobby-pose-blend-validation.json ok=true。首轮验证脚本调用未暴露get_current_time，改用mesh.get_position后通过；非产品故障。
- 本轮仅LobbyCharacterBase和新增LobbyPoseBlendAnimInstance及harness改动，没有修改成品动画、挂点配置、模型或地图；未最终提交/push。
- 已查看三枪实际UE放松/中间帧/举枪共9张截图（Saved/Codex/PoseBlend-*.png），WEAPON按钮触发混合，过渡姿态可见。视觉脚本末尾仅JSON写入因包含DelegateHandle失败，截图均已完成；已修正脚本序列化，未将其冒充完整自动验证，数值验证使用独立通过的lobby-pose-blend-validation.json。后台编辑器均已退出。

## 2026-09-14 大厅暗背景与红白人物灯光

- 用户参考16:40:13红白侧光人物与16:39:39暗背景大厅截图，要求调暗整个Lobby、顶部大面积白光向下、画面左红右白。checkpoint7baac17保存此前0.5秒混合。现有地图内灯光强度降至原2.5%，PostProcessVolume固定EV100=3、补偿0；新增LobbyLighting文件夹三盏可直接调节的Movable RectLight：Lobby_Top_SoftWhite 250lm/180×140cm，Lobby_Left_Red 750lm/75×190cm，Lobby_Right_White 500lm/85×190cm，衰减半径均520cm、间接照明0.1。侧灯仅Lighting Channel1，地图人物与两个枪显示组件启用Channel0+1；顶灯Channel0，保留暗淡落地光区。左右以实际镜头画面为准（红灯世界+X，白灯-X）。仅修改LobbyMap，不改蓝图、材质、正式地图或C++，所有原Actor变换逐项相等。
- 两轮实际PIE预览后收敛：首轮灯光偏亮、地面溢光且左右与画面相反；第二轮降低强度并限制侧灯通道，已查看远景。author脚本保存成功，正在冷启动回读与三枪远近景验证。
- 冷启动回读及实际LobbyMap PIE验证通过：三灯强度/照明通道、人物与两个枪组件通道、固定曝光均持久化；三枪远近镜头共6张画面已查看，验证退出后地图SHA256不变。Saved/Codex/lobby-lighting-validation.json ok=true，lobby-lighting-validation.log LOBBY_LIGHTING_OK，LobbyLighting-Review.jpg为汇总。没有C++/蓝图资产修改，无需重新编译；后台编辑器退出。
- 范围内仍有原场景两盏DirectionalLight同优先级与VSM Non-Nanite队列警告（此前截图已有），未作为本次失败或进行额外渲染架构调整。最终产品仅LobbyMap.umap；未最终提交/push，待用户观感反馈。

## 2026-09-14 武器详情 UMG

- 用户要求点击WEAPON后显示枪名、介绍、返回，风格统一。checkpoint38f0416保存此前灯光与当时磁盘地图状态。
- 原WBP_LobbyPresentation新增WeaponDetailsPanel：左侧深色半透明信息卡，金色WEAPON标识、Roboto枪名/介绍、BACK按钮；进入武器视图折叠PresentationMenu，返回恢复菜单并调用原远景/随机Relax路径。Back沿用原按钮样式，仅hover高亮。
- FLobbyWeaponPresentation新增可本地化FText DisplayName/Description（多行），配置在维修工大厅BP每枪条目。UMG缓存展示Actor，轻量检查镜头状态和枪索引，仅改变时刷新文字/可见性。空文本旧实例可按WeaponClass读取BP默认配置；不依赖枪索引写死文本。Editor库AddWeaponDetails只扩展现有WidgetTree，重复调用不覆盖布局。
- Development Editor Win64编译通过；中间一次新增WeaponClass比较缺完整EquipmentBase定义，补include后通过。配置脚本先遇到EditDefaultsOnly不允许改地图实例，以及Python数组Struct副本须显式写回items[index]，现按BP默认值逐项写回，不修改地图。正在冷启动PIE验证最终资产与显示。
- 最终确认：正确写回数组后，现有地图实例正常继承三枪文案；此前以为地图覆盖导致空文本的初步判断并不成立，直接原因是Python数组Struct副本未写回。空文本实例的默认值后备仍保留作为兼容。
- Development Editor Win64构建Succeeded；Widget BP与角色BP编译保存并冷回读。实际LobbyMap PIE验证三枪名称/介绍、打开详情时切枪刷新、BACK恢复主菜单/远景/Relax、64次连续进出覆盖Relax索引0/1、默认与Pressed中性/hover金边，通过后地图和两项资产哈希不变。lobby-weapon-details-validation.json ok=true，lobby-weapon-details-pie.log WEAPON_DETAILS_OK；三枪详情及返回截图均已查看，汇总WeaponDetails-Review.jpg。
- 本轮产品只有UI与Editor authoring代码、展示结构两项文本字段、WBP_LobbyPresentation与BP_MaintenanceWorker_Lobby；地图/灯光/相机/动画/正式武器资产均未修改。后台编辑器退出，未最终提交/push。

## 2026-09-14 简洁武器选择器与真实缩略图

- 用户否决旧信息卡，先看imagegen预览并明确去掉02/03及左右箭头，再授权实现。保留三张枪械选择缩略图、金色选中边框/角标、名称/两行介绍、左下角BACK；透明背景，无大底板。checkpoint16decf7保存此前详情版本。
- 从当前BP配置导出真实SK_SCFRIFLE及两把80%大厅StaticMesh，在D:/Blender Projects/LobbyWeaponIcons制作统一正交侧视、灰白材质/灯光、512×256透明PNG。三枪宽度76.804/76.232/74.318cm，模型顶点27742/3145/2106。来源路径exported.json；rendered.json和alpha-validation.json通过，Icons-Preview.png已查看并打开给用户。不是AI生成的枪轮廓。NullRHI骨骼模型导出触发引擎MeshObject断言，改为正常渲染进程导出后成功，没有修改源资产。
- 四张纹理（三枪+小三角角标）导入UI/Lobby/Textures，TC_EDITOR_ICON、UI组、无mip、sRGB。每枪FLobbyWeaponPresentation新增Thumbnail；BP配置图标与精简两行介绍。Widget有三个真实按钮，点击调用SetDisplayWeaponIndex并保持Rifle/近景；名称、模型、动画、图标及唯一选中角标同步。主菜单仍只hover高亮；枪缩略图选中标记按用户确认预览保留。
- RefineWeaponDetails重排既有WidgetTree，移除旧信息底板，把BACK放到根Canvas左下角；不会创建编号或左右翻页。首轮编译混合派生指针initializer_list推导失败，改显式UWidget指针数组后通过。首轮PIE切枪/返回已通过，但图标DesiredSizeOverride未持久化导致显示太小，改用序列化BrushSize并保存，正在最终回归。
- 最终尺寸修复：UE5.7的SetBrushSize也只是SetDesiredSizeOverride的别名，不会序列化。查阅引擎Image.cpp后改为复制FSlateBrush、SetImageSize、SetBrush并保存；RefineWeaponDetails重复调用仅同步这六个图像尺寸，最终冷加载图标100×50、角标18×18显示正常。Python尺寸断言遇到DeprecateSlateVector2D字段未暴露，改以最终实际截图验证布局，没有放宽交互/资产断言。
- Development Editor Win64构建Succeeded；UMG与角色BP编译保存；冷启动实际LobbyMap PIE通过三枪按钮、对应模型/动画/文案/图标、唯一选中边框角标、真实鼠标悬停、60次连续切枪、重复当前枪不重启动画、BACK返回远景/Relax及保留枪索引。lobby-minimal-selector-validation.json ok=true；地图、两BP与四纹理退出后哈希不变。最新三枪和返回截图MinimalSelector-{0,1,2}00002.png、MinimalSelector-Back00002.png已查看，汇总MinimalSelector-Review.jpg，单张实际界面MinimalSelector-Preview.png。
- 本轮只改展示UI/配置/图标与对应源码，无地图、灯光、动画、枪模或正式武器变更；后台验证编辑器已退出，重新打开用户项目供预览。未最终提交/push。

## 2026-09-14 远端发布

- 用户明确要求推送远端。fetch确认main相对origin/main领先9个本地checkpoint、没有分叉；提交最终简洁选择器与四纹理，并连同此前握持/枪模/动画混合/地图灯光版本一起发布。发布前diff检查通过，既有Development Editor构建与实际PIE证据仍通过，本轮未改产品实现。提交及远端同步结果以Git历史和origin/main为准。
- 主体b813b65已推送origin/main并核对远端SHA一致，34个LFS对象约25MB上传成功。随后发现编辑器18:41:28新保存LobbyMap（8682972字节，SHA256 caeafe5d373605a07db75d7a6d016f4b694aeb9b6c3955c52cf386cc55c0a7c6），按本次发布授权补充提交当前磁盘地图。仅核对包头和LFS，不将此前UI/地图PIE验证冒充这份新保存地图的单独验证。


## 2026-09-14 人物三点光重布

- 用户授权将现有人物/枪械灯分组关闭并重新布置三点光。checkpoint cbc41c3 保存用户最新地图。检查发现三盏旧RectLight之外还有SpotLight3/4，两盏聚光灯外锥角只有3度，容易在换姿势后照偏。
- 五盏旧灯归入LobbyLighting/Previous_Disabled，LightComponent Visibility=false，保留原位置、颜色与强度供恢复。新增LobbyLighting/ThreePoint：Lobby_Key_SoftWhite、Lobby_Fill_SoftWhite、Lobby_Rim_Red，宽面光覆盖人物与枪的姿态范围。只照Channel1；当前曝光与环境灯保持。已保存地图，正在冷启动PIE检查三枪放松/举枪。

- 最终冷启动实际LobbyMap PIE验证通过：三枪Relax/Rifle六张截图（ThreePoint-{0,1,2}-{Character,Weapon}00001.png）已查看，红轮廓220lm；旧5灯编辑器/运行时关闭、新3灯强度与通道正确，1982个原Actor变换及非目标灯配置保持，退出地图SHA256不变。lobby-three-point-validation.json ok=true，three-point-validation-final.log THREE_POINT_OK；最终汇总ThreePoint-Review.jpg。仅修改LobbyMap与harness，无C++或蓝图资产改动，无需编译。
- 首轮红色反射使举枪枪身过红，轮廓光由650降至220lm后完成最终回归；没有增加动态跟随灯光系统。未最终提交/push。

- 用户要求提交远端：fetch确认main领先1个安全检查点且无分叉；本次提交LobbyMap与四份harness记录，连同cbc41c3推送origin/main。沿用此前通过的三枪两姿态PIE验证；发布不修改灯光配置，提交与同步结果以Git为准。
