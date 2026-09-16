# 核心框架（Core）

FEAT-085（2026-09-16）大厅展示选角：CharacterSelectPlayerController.CharacterPresentations 在 BP 中配置维修工和执行官的 ID、英文名字／介绍、展示类。GetDisplayCharacter 返回当前可见展示对象，SelectPresentationCharacter 缓存各角色实例、保留各自枪索引、切换可见性并进入 Relax／远景。UI、Weapon 和预留1键都使用当前对象，不再遍历取首个。该入口只选择大厅展示，不调用旧 SelectCharacterAndStart 或切换战斗 Pawn。

**何时读取：** 修改输入处理、角色切换流程、GameMode / PlayerState 初始化时。

> 回合系统 / 死亡 / 大厅选角色 / 关卡切换 / GameInstance 跨关卡持久 → 见 `13-game-flow.md`。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Core/TheManGameModeBase.h` | 测试地图 GameMode：`CharacterRosterTable` + override `GetDefaultPawnClassForController`（据 GameInstance 选定 ID 生成角色，详见 13） |
| `Source/TheManTest/Private/Core/TheManGameModeBase.cpp` | 含 `[PawnSelect]` 临时诊断（查清后删） |
| `Source/TheManTest/Public/Core/TheManLobbyGameMode.h` / `.cpp` | 大厅 GameMode：`DefaultPawnClass=nullptr`；BeginPlay 建选角色 UI + UI 输入模式（详见 13） |
| `Source/TheManTest/Public/Core/CharacterSelectGameMode.h` / `.cpp` | 新选角场景专用 GameMode：不生成 Pawn；默认使用 `ACharacterSelectPlayerController`；可选创建新选角 UI；不依赖旧 LobbyMap / 旧 WBP |
| `Source/TheManTest/Public/Core/CharacterSelectPlayerController.h` / `.cpp` | 新选角场景专用 Controller：GameAndUI 输入模式 + Enhanced Input IMC；UI通过SetWeaponPresentationView明确切角色／武器镜头，IsWeaponPresentationView读取状态；不再绑定背景点击切镜头。原SetPointerOverUI接口保留兼容 |
| `Source/TheManTest/Public/Core/CharacterSelectCameraSwitcher.h` / `.cpp` | 新选角场景摄像机控制：引用远/近 Cine Camera 目标点；运行时自动生成内部 `ACineCameraActor` Rig 作为 ViewTarget；Rig以BlendTime（0.7秒）smoothstep同步位置、四元数旋转、焦距、光圈、手动对焦距离；同目标忽略，反向切换从当前画面开始。鼠标视差5／3cm、近景乘0.5，切换期间暂停视差；无大推进／弹簧速度累加 |
| `Source/TheManTest/Public/Core/TheManGameInstance.h` / `.cpp` | 跨关卡持久容器：`SelectedCharacterID` / `CarriedRoundNumber`；`SelectCharacterAndStart` / `HandlePlayerDeath`（详见 13） |
| `Source/TheManTest/Public/Core/TheManPlayerController.h` | 增强输入绑定、`SwitchCharacter(FName)`、`DT_CharacterRoster` 指针；`PrimaryFireAction` / `SecondaryFireAction` / `ReloadAction`；`DebugSkipTimeAction`(调试快进)；本地 `CombatHUDWidget` 生命周期与 Equipment/Firearm 委托绑定 |
| `Source/TheManTest/Private/Core/TheManPlayerController.cpp` | 输入回调；BeginPlay 加 IMC + **重置 GameOnly 输入模式**（覆盖大厅 UIOnly 残留，详见 13 BUG-037-001）；`HandleDebugSkipTime`；本地创建 `UCombatHUDWidgetBase`，在 Possess/UnPossess/切枪时解绑重绑装备、弹药和 PlayerState ASC 血量委托，不做 UI Tick |
| `Source/TheManTest/Public/Core/TheManPlayerState.h` | ASC 和 AttributeSet 的声明（GAS 所有者） |
| `Source/TheManTest/Private/Core/TheManPlayerState.cpp` | ASC / AttributeSet 构造 |
| `Source/TheManTest/Public/Core/TheManCharacterTypes.h` | `FCharacterType` 结构体（DataTable 行类型，含角色类引用、图标、描述） |
| `Source/TheManTest/Public/UI/CharacterSelectWidgetBase.h` / `.cpp` | 选角色 UI 基类（BindWidget 三按钮自动绑点击，详见 13） |
| `Source/TheManTest/Public/UI/Combat/CombatHUDWidgetBase.h` / `.cpp` | 原生战斗 HUD：视口中心半径46.08px、线宽2px的80段空心圆；底部显示当前血量、大号当前子弹数和小号备用弹夹数，不显示容量；Slate Paint 绘制、Hit Test Invisible、事件驱动更新 |
| `Source/TheManTest/Public/Core/TheManGameStateBase.h` / `.cpp` | 回合倒计时 + 半场二阶段 + 强度波 + DebugSkipTime（详见 13） |

## 测试地图



- 2026-09-13 用户清理：额外7张测试地图（CoreMorph×5、GASPTest、VFXTestMap）及两份地图专属材质已删除。只保留TestMap作为测试地图；LobbyMap正式流程保持。TestMap删除58个后加测试Actor，基础环境与PlayerStart保持。旧VFX房自动化改用TestMap，临时对象不保存回关卡。

## 统一预留测试入口

IMC_Default 的 IA_Test = One（键盘1）。BP_TheManPlayerController 的 TestSwitchCharacterAction 保留旧字段名／IA引用，Started事件现在调用HandleTestInput，不再切维修工。入口仅PIE执行；当前按1临时生成CoreMorph头领和闭合路线，以正式主BT测试空中导弹，再按1清理；OnUnPossess／EndPlay也清理头领、AIController及路线。主逻辑位于既有PlayerController.cpp的WITH_EDITOR段，不新增测试地图、相机或按键。未来手动验证替换该入口当前场景；地图持续保持干净。

## FEAT-082 展示导航

正式LobbyMap使用BP_CharacterSelectGameMode，CharacterSelectWidgetClass引用/Game/UI/Lobby/WBP_LobbyPresentation（原为空）。父类ULobbyPresentationWidgetBase绑定Button_Character和Button_Weapon、维护选中描边，UMG资产持有布局／样式／英文文字；它不调用SelectCharacterAndStart。原WBP_CharacterSelect和回合选角流程独立。Editor辅助TheManLobbyAssetLibrary仅初始化空WidgetBlueprint，不覆盖已有布局。

- FEAT-082相机一致性：Switcher每帧读取绑定CameraComponent世界变换（含组件相对偏移），同步取景约束／过扫描／后处理；场景相机是最终取景来源，不能再用一次性Actor位置缓存。鼠标居中、切换结束、相同视口尺寸时与场景相机视图一致。


FEAT-082按钮姿势联动（2026-09-14）：SetWeaponPresentationView现在同步大厅展示Actor和镜头。true=Rifle/近景，false=从当前枪非空RelaxedAnimations均匀随机选一条并进入Relaxed/远景，每次调用重新选，允许重复。UI不再保留选中高亮，仅Hovered高亮。角色仍SingleNode硬切，没有动画混合；既有0.7秒平滑仅用于相机。此说明覆盖之前“按钮仅切相机”的行为记录。

FEAT-087（2026-09-16）：ULobbyPresentationWidgetBase 将主菜单 START、角色箭头/TAB、VIEW WEAPONS 与武器页 START GAME 串联。GoBack 从武器回角色，从角色回主菜单。StartGame 复用 TheManGameInstance.SelectCharacterAndStart 并传当前 CharacterPresentations.CharacterID。旧 Button_Character/Button_Weapon 绑定名保留以兼容资产；前者显示 START，后者显示 SETTINGS 且禁用。

FEAT-088（2026-09-16）：CharacterSelectCameraSwitcher 的运行时相机独立计算目标对焦平面，场景相机继续提供焦段、光圈、构图与后处理。远景取当前 LobbyCharacter.DisplayMesh Bounds 上半身（中心Z+半高×0.35）；近景优先可见且有资源的 DisplayWeapon，其次 DisplaySkeletalWeapon，取枪体 Bounds 中心；无有效枪则回退身体。焦距为目标点沿相机前向的轴向距离，跟随当前展示角色、枪型与鼠标视差；强制 Manual/FocusOffset0，沿已有 TransitionAlpha 混合旧焦距到目标。无需保存关卡相机的旧手动距离；不改变原焦段/光圈/屏幕比例。
