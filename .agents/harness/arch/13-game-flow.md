# 游戏流程（回合 + 死亡 → 大厅选角色 → 重开）

**何时读取：** 修改回合/倒计时/阶段升级逻辑、死亡处理、关卡切换、选角色流程、跨关卡持久数据（GameInstance）时。

> 涉及功能：FEAT-022（回合系统）/ FEAT-036（半场二阶段 + 强度伤害递增）/ FEAT-037（死亡→大厅→选角色→重开）。

---

## 涉及文件

| 文件 | 关键内容 |
|---|---|
| `Public/Core/TheManGameStateBase.h` / `.cpp` | 回合倒计时驱动 + 半场二阶段广播 + 强度波广播；`AdvanceRound()` / `DebugSkipTime()` |
| `Public/Core/TheManGameInstance.h` / `.cpp` | 跨关卡持久：`SelectedCharacterID` + `CarriedRoundNumber`；`SelectCharacterAndStart()` / `HandlePlayerDeath()` |
| `Public/Core/TheManGameModeBase.h` / `.cpp` | 测试地图 GameMode：`GetDefaultPawnClassForController` 据选定 ID 在花名册查类生成角色；`CharacterRosterTable` |
| `Public/Core/TheManLobbyGameMode.h` / `.cpp` | 大厅专用 GameMode：`DefaultPawnClass=nullptr`；BeginPlay 建选角色 UI + 显鼠标 + UI 输入模式 |
| `Public/UI/CharacterSelectWidgetBase.h` / `.cpp` | 选角色 UI 基类：BindWidget 三按钮 + 自动绑点击 → `SelectCharacter(ID)` |
| `Private/Core/TheManPlayerController.cpp` | BeginPlay 末尾重置 `FInputModeGameOnly` + 隐藏鼠标（覆盖大厅残留的 UI 输入模式） |
| `Private/Characters/FPSCharacterBase/FPSCharacterBase.cpp` | `OnDeath()` → `GI->HandlePlayerDeath(RoundNumber)`（不再原地复活） |

---

## 回合系统（GameState，FEAT-022/036）

`ATheManGameStateBase` 每帧 `Tick → AdvanceRound(DeltaSeconds)`：

- **倒计时**：`TimeRemaining` 递减（钳 0 不为负）；归零 → `OnCountdownExpired()`（= 玩家死亡，见下）。
- **强度波（细粒度递增）**：每过 `StrengthIncreaseInterval`（默认 150s=2.5min）广播一次 `OnMidRoundStrengthIncrease` + `ElapsedStrengthWaves++`。敌人订阅后伤害倍率累加（见 `11-enemy-ai.md`）。
- **半场二阶段（粗粒度）**：`TimeRemaining ≤ CurrentRoundDuration × Phase2TriggerRemainingFraction`（默认 0.5）时广播一次 `OnCombatPhaseChanged(2)`（`bPhase2Triggered` 守卫）。敌人据此 `SetCombatPhase(2)` 换技能集——**阶段只换技能集，不碰伤害**。
- `StartNewRound()`：`RoundNumber++`，按 `BaseCountdownDuration - CountdownReductionPerRound×(Round-1)`（钳 `MinCountdownDuration`）算时长，重置波数/阶段标记。
- `BeginPlay`：先从 GameInstance 读 `CarriedRoundNumber` 赋 `RoundNumber`，再 `StartNewRound()`（+1 衔接），实现回合数跨关卡延续。

**默认值定稿**：`BaseCountdownDuration=600`(10min) / `StrengthIncreaseInterval=150`(2.5min) / `Phase2TriggerRemainingFraction=0.5`。

**调试**：`DebugSkipTime(Seconds=150)`（BlueprintCallable，复用 `AdvanceRound`，减过头钳 0 触发回合结束）；Controller `DebugSkipTimeAction`(IA) 绑键调用。屏幕显示剩余时间「分:秒」。

事件（BlueprintNativeEvent，蓝图覆写须调 Parent）：`OnRoundStarted(Round, Duration)` / `OnCountdownExpired()`。

---

## 死亡 → 大厅 → 选角色 → 重开（FEAT-037）

```
死亡两源：
  ① 被打死：Health≤0 → AttributeSet → AFPSCharacterBase::OnDeath()
  ② 时间到：TimeRemaining≤0 → GameState::OnCountdownExpired()
        ↓ 都 funnel 到
UTheManGameInstance::HandlePlayerDeath(当前RoundNumber)
  - bPendingTransition 守卫（防同帧两源重复切换）
  - CarriedRoundNumber = 当前回合
  - OpenLevel(LobbyMapName="LobbyMap")
        ↓
LobbyMap（GameMode=BP_LobbyGameMode : ATheManLobbyGameMode）
  - DefaultPawnClass=nullptr（大厅不生成战斗角色）
  - BeginPlay：CreateWidget(LobbyWidgetClass=WBP_CharacterSelect) + AddToViewport
              + bShowMouseCursor=true + SetInputMode(UIOnly)
        ↓ 玩家点按钮
UCharacterSelectWidgetBase::SelectCharacter(ID)
  → GI->SelectCharacterAndStart(ID)
      - SelectedCharacterID = ID；bPendingTransition=false（解除守卫）
      - OpenLevel(TestMapName="TestMap")
        ↓
TestMap（GameMode=BP_TheManGameMode : ATheManGameModeBase）
  - GetDefaultPawnClassForController：读 GI.SelectedCharacterID → CharacterRosterTable 查行 → 返回 CharacterClass
        → 引擎在 PlayerStart 生成该角色并 Possess（GAS/装备照常 PossessedBy/BeginPlay 初始化）
  - GameState.BeginPlay：RoundNumber=CarriedRoundNumber → StartNewRound()（回合+1，敌人初始强度+1）
  - PlayerController.BeginPlay：AddMappingContext + SetInputMode(GameOnly) + 隐藏鼠标
```

### 跨关卡数据（GameInstance 是唯一能存活过 OpenLevel 的对象）
- `SelectedCharacterID`：选了谁 → 测试地图 GameMode 据此生成角色。
- `CarriedRoundNumber`：死亡写入当前回合 → 测试地图 GameState 读取后 +1。**回合内的波次增强/伤害倍率不携带，每回合重置**（对应"前面增强消失、只留逐回合 +1 的初始增强"）。

### 关键约束 / 坑
- **测试地图 GameMode 必须填 `CharacterRosterTable=DT_CharacterRoster`**，且 World Settings GameMode Override 指向它，否则 `GetDefaultPawnClassForController` 查不到 → 回退默认 Pawn（角色错）。
- **大厅必须用 `ATheManLobbyGameMode`**（非测试地图 GameMode），否则会在菜单里生成战斗角色。
- **输入模式跨关卡持久**（`SetInputMode` 改的是持久的 `UGameViewportClient`）：大厅设 UIOnly，进游戏关卡靠 `ATheManPlayerController::BeginPlay` 重置 GameOnly，否则角色动不了（BUG-037-001）。
- 选角色 UI 不拖进关卡——填在 `BP_LobbyGameMode.LobbyWidgetClass`，由大厅 GameMode BeginPlay 自动创建。
- 按钮↔角色对应 = `WBP_CharacterSelect` 的 `Character1/2/3ID`（默认 Infiltrator/MaintenanceWorker/TheExecutive），**不按 DT 行顺序**（DataTable 底层 TMap 遍历顺序不保证）。

### 临时调试（查清后删）
- `GetDefaultPawnClassForController` 内 `[PawnSelect]` 屏幕/Log 诊断（逐步定位生成失败原因）。

---

## 目录约定
- UI 类（UMG widget）放 `Source/.../UI/`。
- 框架级（GameMode / GameState / GameInstance / PlayerController / PlayerState）放 `Core/`。
