# 核心框架（Core）

**何时读取：** 修改输入处理、角色切换流程、GameMode / PlayerState 初始化时。

> 回合系统 / 死亡 / 大厅选角色 / 关卡切换 / GameInstance 跨关卡持久 → 见 `13-game-flow.md`。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Core/TheManGameModeBase.h` | 测试地图 GameMode：`CharacterRosterTable` + override `GetDefaultPawnClassForController`（据 GameInstance 选定 ID 生成角色，详见 13） |
| `Source/TheManTest/Private/Core/TheManGameModeBase.cpp` | 含 `[PawnSelect]` 临时诊断（查清后删） |
| `Source/TheManTest/Public/Core/TheManLobbyGameMode.h` / `.cpp` | 大厅 GameMode：`DefaultPawnClass=nullptr`；BeginPlay 建选角色 UI + UI 输入模式（详见 13） |
| `Source/TheManTest/Public/Core/CharacterSelectGameMode.h` / `.cpp` | 新选角场景专用 GameMode：不生成 Pawn；默认使用 `ACharacterSelectPlayerController`；可选创建新选角 UI；不依赖旧 LobbyMap / 旧 WBP |
| `Source/TheManTest/Public/Core/CharacterSelectPlayerController.h` / `.cpp` | 新选角场景专用 Controller：GameAndUI 输入模式 + Enhanced Input IMC；点击非 UI 区域调用场景 `CharacterSelectCameraSwitcher` 切远近景；UI 可用 `SetPointerOverUI` 阻止空白点击逻辑 |
| `Source/TheManTest/Public/Core/CharacterSelectCameraSwitcher.h` / `.cpp` | 新选角场景摄像机控制：引用远/近 Cine Camera 目标点；运行时自动生成内部 `ACineCameraActor` Rig 作为 ViewTarget；Rig 用弹簧切远近景、复制目标 Cine Camera 镜头参数，并叠加鼠标四方向视差 |
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

- `/Game/Maps/VFXTestMap`：FEAT-080 使用的独立暗场武器 VFX 测试房。World Settings 使用 `BP_TheManGamemodeBase_C`，PlayerStart 正对预放置的静止 `BP_Phantom`；固定 Manual Exposure，包含冷暖低强度灯光和环境命中靶面。它不替代主流程 `TestMap`。
