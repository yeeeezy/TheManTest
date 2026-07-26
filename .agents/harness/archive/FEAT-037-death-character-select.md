# [FEAT-037] 死亡 → 选角色关卡 → 重开下一回合

**创建日期：** 2026-06-20
**状态：** done（用户 session42 验证通过）
**Archive 文件：** `archive/FEAT-037-death-character-select.md`

---

## 功能概述

把原来的"原地复活"死亡循环改成跨关卡的选角色循环：

- 玩家死亡（**被打死** 或 **倒计时归零**）→ 切到**选角色关卡**。
- 选角色关卡有 **3 个按钮**对应 3 个角色（Infiltrator / MaintenanceWorker / TheExecutive）。
- 选完 → 进**测试地图**，**开始下一回合**。

跨关卡数据靠 `UTheManGameInstance` 携带（GameInstance 是唯一能在 `OpenLevel` 之间存活的对象）：
- `SelectedCharacterID`：选了谁，测试地图 GameMode 据此生成角色。
- `CarriedRoundNumber`：死亡时写入当前回合数，测试地图 GameState 读取后 `StartNewRound()`（+1）。这正是"敌人初始强度逐回合 +1"（FEAT-036）依赖的 `RoundNumber` 跨关卡延续。

游戏首次启动也从选角色关卡进入（入口统一）。

---

## 设计决策（session41 与用户确认）

- **回合数继续递增、不清零**：死亡重选后回合数接着涨（敌人初始强度逐回合 +1）。故必须用 GameInstance 携带 `RoundNumber`（GameState 过 OpenLevel 会重建归零）。
- **每回合内的增强重置**：`ElapsedStrengthWaves` / `CurrentDamageMultiplier` 不携带，新回合归零——对应"前面的波次增强消失，只留逐回合 +1 的初始增强"。
- **角色生成走 GameMode `GetDefaultPawnClassForController`**（map-scoped）：测试地图 GameMode 据 `SelectedCharacterID` 在花名册查类、在 PlayerStart 生成；选角色地图用各自 GameMode（不带本逻辑），不会在菜单生成战斗角色。
- **首启也走选角色**：选角色关卡即游戏入口。
- 死亡两源（被打死 / 倒计时归零）funnel 到 `GameInstance::HandlePlayerDeath`，`bPendingTransition` 守卫防同帧重复切换。

---

## 范围

**涉及 C++ 文件：**
- `Public/Core/TheManGameInstance.h` / `Private/.../TheManGameInstance.cpp`（新建）
  - `SelectedCharacterID` + `CarriedRoundNumber` + `LobbyMapName`(默认 "LobbyMap") / `TestMapName`(默认 "TestMap")（可配）
  - `SelectCharacterAndStart(ID)`：记角色 → OpenLevel(测试地图)；解除切换守卫
  - `HandlePlayerDeath(Round)`：记回合 → OpenLevel(LobbyMap)；`bPendingTransition` 守卫
- `Public/Core/TheManLobbyGameMode.h` / `Private/.../TheManLobbyGameMode.cpp`（新建）
  - 大厅专用 GameMode：构造 `DefaultPawnClass=nullptr`（不生成战斗角色）；`LobbyWidgetClass` 属性；BeginPlay 自动 CreateWidget+AddToViewport + bShowMouseCursor + SetInputMode(UIOnly)
- `Public/UI/CharacterSelectWidgetBase.h` / `Private/UI/CharacterSelectWidgetBase.cpp`（新建，放独立 UI/ 目录）
  - 选角色 UI 基类（UUserWidget），**逻辑全在 C++**：
    - `BindWidget` 三个按钮 `Button_Character1/2/3`（UMG 必须有同名控件）+ `BindWidgetOptional` 三个 `Text_Character1/2/3`
    - `NativeOnInitialized` 自动给按钮挂 OnClicked → 各调 `SelectCharacter(CharacterNID)`；并把花名册 DisplayName 填进 Text
    - `Character1/2/3ID`（默认 Infiltrator/MaintenanceWorker/TheExecutive，可改）+ `CharacterRosterTable`
    - `SelectCharacter(ID)`→GI.SelectCharacterAndStart；`GetCharacterInfo(ID,Out)` 供蓝图自定义显示（头像等）
  - Build.cs 增加 `UMG` 模块依赖（UUserWidget / UButton / UTextBlock）
  - 目录约定：UI 类放 `UI/`（框架级 GameMode/GameState/Controller/GameInstance 留 `Core/`）
- `Public/Core/TheManGameModeBase.h` / `Private/.../TheManGameModeBase.cpp`
  - 新增 `CharacterRosterTable`（DataTable）+ override `GetDefaultPawnClassForController_Implementation`：据 GI.SelectedCharacterID 查花名册返回角色类
- `Private/Core/TheManGameStateBase.cpp`
  - `BeginPlay`：从 GI 读 `CarriedRoundNumber` 赋 `RoundNumber`，再 `StartNewRound()`（+1 衔接）
  - `OnCountdownExpired_Implementation`：从调试占位改为 `GI->HandlePlayerDeath(RoundNumber)`
- `Private/Characters/FPSCharacterBase/FPSCharacterBase.cpp`
  - `OnDeath`：去掉原地复活（StartNewRound+回血+传送），改为 `GI->HandlePlayerDeath(RoundNumber)`

**涉及蓝图 / 编辑器（用户操作）：**
- 新建 `BP_TheManGameInstance`（父类 `UTheManGameInstance`）→ Project Settings → Maps & Modes → Game Instance Class 设为它；确认 `LobbyMapName`=LobbyMap / `TestMapName`=TestMap
- 新建 **`LobbyMap.umap`** + 基于 **`ATheManLobbyGameMode`** 建 `BP_LobbyGameMode`（填 LobbyWidgetClass），设为 LobbyMap 的 GameMode Override
- 新建 `WBP_CharacterSelect`（父类 `UCharacterSelectWidgetBase`），填 `CharacterRosterTable=DT_CharacterRoster`；UMG 里放三个 Button 命名 `Button_Character1/2/3`（点击逻辑 C++ 已绑，无需连线），可选放 `Text_Character1/2/3` 自动显示名字。头像等自定义可用 `GetCharacterInfo`
- 测试地图 GameMode = `BP_TheManGameMode`，`CharacterRosterTable` 填 `DT_CharacterRoster`
- Project Settings → Maps & Modes → Editor/Game Default Map 设为 `LobbyMap`（首启入口）

**依赖：**
- FEAT-005 角色花名册 `DT_CharacterRoster` / `FCharacterType`
- FEAT-022/036 回合系统（`RoundNumber` 跨关卡延续是核心）

**完成标准（与 feature_list.json 一致）：**
- [ ] C++ 编译无错误无警告
- [ ] GameInstance 类在 Project Settings 设定；两张地图 + 选角色 UMG + 各 GameMode 配好
- [ ] PIE：首启进选角色 → 选角色进测试地图开回合 1
- [ ] PIE：被打死 → 回选角色 → 再选 → 测试地图回合 +1（敌人初始强度 +1）
- [ ] PIE：倒计时归零 → 同样回选角色

---

## 实现日志

### 2026-06-20 — 功能创建 + C++ 实现（待编译）

- **背景：** 用户要求死亡（被打死/时间到）后进选角色关卡，3 按钮选角色，选完进测试地图开下一回合；首启也走选角色。
- **关键确认（用户）：** 跨关卡需要带的只有 `RoundNumber`（敌人初始强度逐回合 +1，对应已留的 GameState.RoundNumber，在 EnemyBase::BeginPlay 以 `BaseStrength + RoundNumber` 消费）+ `SelectedCharacterID`。回合内波次增强/伤害倍率不带，每回合重置。
- **实现：** 见范围。角色生成选 GameMode 默认 Pawn 路径（map-scoped，避免在选角色菜单误生成战斗角色）。
- **编译结果：** ✓ 编译通过（session41/42）。

---

### 2026-06-20-session42 — 游戏结束（回合时长减到下限 → 回大厅显示"游戏结束" + 禁用选角色）

- **背景：** 用户要求每回合倒计时递减（默认改 2.5min/回合），减到 2:30 下限以下时游戏结束；游戏结束 = 回大厅、显示"游戏结束"、并禁用原本的选角色 UI。
- **设计：** 下限 `MinCountdownDuration` 语义从"钳住继续玩"升级为"游戏结束阈值"。回合走向：1=10:00 / 2=7:30 / 3=5:00 / 4=2:30 / 5=游戏结束（撑过第 4 回合即结束）。
- **实现：**
  - `TheManGameStateBase.h`：`CountdownReductionPerRound` 5→150、`MinCountdownDuration` 10→150。
  - `TheManGameStateBase.cpp` `StartNewRound`：先算 `RawDuration = Base - Reduction×(Round-1)`，若 `< MinCountdownDuration` → `bRoundActive=false` + 屏幕红字 + `GI->HandleGameOver()` + return（不开回合）；否则正常用 RawDuration。
  - `TheManGameInstance.h/.cpp`：新增 `bGameOver`（BlueprintReadOnly）+ `HandleGameOver()`（共用 `bPendingTransition` 守卫，置 `bGameOver=true` + OpenLevel(Lobby)）+ `IsGameOver()`。
  - `UI/CharacterSelectWidgetBase.h/.cpp`：`NativeOnInitialized` 读 `GI->IsGameOver()` → `ApplyGameOverState(bool)`：禁用三个 `Button_CharacterN`（`SetIsEnabled(!bGameOver)`）+ 显示可选 `Text_GameOver`（文本 `GameOverText`，留空回退 `TEXT("游戏结束")`）+ 调 `BlueprintImplementableEvent OnGameOverState(bool)` 供蓝图自定义。
- **死亡时预判（消除测试地图闪烁，session42 二次修订）：** 死亡两源统一走 `ATheManGameStateBase::RoutePlayerDeath()`——算下一回合 `NextRound=RoundNumber+1` 的理论时长 `ComputeRoundDuration(NextRound)`（抽出公共助手，`StartNewRound` 也用），`< MinCountdownDuration` → 直接 `GI->HandleGameOver()`（**跳过测试地图加载**），否则 `GI->HandlePlayerDeath(RoundNumber)`。`FPSCharacterBase::OnDeath`（原直接调 GI）与 `OnCountdownExpired_Implementation` 都改调 `RoutePlayerDeath`。`StartNewRound` 内同款检查保留为防御兜底（正常流程已不会加载到游戏结束回合）。
- **编辑器待配（可选）：** `WBP_CharacterSelect` 加 TextBlock 命名 `Text_GameOver`（不加则只禁用按钮无文字）；或用 `OnGameOverState` 蓝图事件做结束画面。
- **编译结果：** ✓ session42 全量重编通过。
- **验证：** ✓ 用户 session42 PIE 通过——撑过第 4 回合 → 死亡瞬间直接回大厅、显示"游戏结束"、三按钮禁用（无测试地图闪烁）。

---

## Bug 记录

### BUG-037-001 — 选角色进 TestMap 后角色移动不了

**发现日期：** 2026-06-20
**严重程度：** 中
**状态：** fixed

**现象：** 角色按选择正确生成，但 WASD 无法移动。

**根本原因：** 大厅 `ATheManLobbyGameMode` 设了 `FInputModeUIOnly`，而 `SetInputMode` 实际改的是 `UGameViewportClient` 的鼠标捕获/输入路由，ViewportClient **跨关卡持久**（不随 OpenLevel 重建）。进 TestMap 后无人重置，游戏输入仍被 UI Only 挡住。

**修复方案：**（2026-06-20）
- `ATheManPlayerController::BeginPlay` 末尾加 `SetInputMode(FInputModeGameOnly())` + `bShowMouseCursor=false`，游戏控制器一启动即强制游戏输入模式，覆盖大厅残留。

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | 2026-06-20 | ✓ | session42 全量重编通过 |
| 编辑器：GameInstance/地图/UMG/GameMode 配置 | 2026-06-20 | ✓ | session41 配好 |
| PIE——首启选角色→进测试地图 | 2026-06-20 | ✓ | session41 |
| PIE——被打死→回选角色→回合+1 | 2026-06-20 | ✓ | session42（敌人初始强度逐回合+1） |
| PIE——倒计时归零→回选角色 | 2026-06-20 | ✓ | session42 |
| PIE——撑过第4回合→游戏结束（回大厅显示+禁用按钮，无闪烁） | 2026-06-20 | ✓ | session42 死亡时预判 RoutePlayerDeath |

---

## 最终备注

> - 选角色大厅用 **`ATheManLobbyGameMode`**（非测试地图 GameMode），`DefaultPawnClass=nullptr` 不生成战斗角色；它自动建 UI + 设鼠标/UI 输入模式。
> - `OnCountdownExpired` 仍是 `BlueprintNativeEvent`：若 `BP_TheManGameState` 覆写它，必须调用 Parent，否则死亡路由不执行。
> - 角色生成依赖测试地图 GameMode 的 `CharacterRosterTable` 已填 `DT_CharacterRoster`；ID 用花名册行名（"Infiltrator" / "MaintenanceWorker" / "TheExecutive"）。
> - 玩家死亡不再原地回血/传送，全靠重载测试地图自然重置（新角色满血、PlayerStart 出生）。

**完成标准全部满足日期：** 2026-06-20（session42）
**功能关闭日期：** 2026-06-20（session42）
