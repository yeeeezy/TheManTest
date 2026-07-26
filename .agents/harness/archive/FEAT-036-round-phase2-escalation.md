# [FEAT-036] 回合二阶段升级（半场切阶段 + 强度伤害系数）

**创建日期：** 2026-06-20
**状态：** done（用户 session41 测试通过）
**Archive 文件：** `archive/FEAT-036-round-phase2-escalation.md`

> 默认值定稿：`BaseCountdownDuration=600`(10min)、`StrengthIncreaseInterval=150`(2.5min)、`StrengthDamageBonusPerWave=0.2`、`Phase2TriggerRemainingFraction=0.5`。新增 `DebugSkipTime` 快进（默认 150s）+ Controller `DebugSkipTimeAction` 按键钩子 + 剩余时间「分:秒」显示。倒计时归零的"占位不真死亡"后被 **FEAT-037** 改为真实死亡路由。

---

## 功能概述

在 FEAT-022 回合系统 + FEAT-032 敌人技能集（`PhaseSkillSets` / `CurrentPhase`）之上，接通"回合内战斗升级"链路：

- 回合倒计时默认时长缩短到 **10s**。
- 倒计时进行到 **一半**（剩余比例 ≤ `Phase2TriggerRemainingFraction`，默认 0.5）时，GameState 广播 `OnCombatPhaseChanged(2)`，所有敌人 `SetCombatPhase(2)`，切到二阶段技能集（`PhaseSkillSets[1]`）。**阶段只换技能集，不影响伤害。**
- 敌人伤害**按更细的时间粒度递增（与阶段解耦）**：复用 FEAT-022 已有的 `StrengthIncreaseInterval`（回合内每隔 N 秒广播 `OnMidRoundStrengthIncrease`，即"强度+1 那波"），每波伤害倍率累加 `StrengthDamageBonusPerWave`（累加百分比，默认 0.2）。倍率 = 1 + 系数 × 已增强波数。例：一轮 600s、间隔 150s（2.5min）→ 每 2.5min 增强一波。
- 配套调试输出：屏幕显示回合剩余时间、玩家扣血、二阶段切换、倒计时归零占位（不真死亡）。

代码位置：`ATheManGameStateBase`（回合/阶段驱动）、`AEnemyBase`（阶段订阅 + 伤害倍率）、`UGA_EnemyShoot::SpawnProjectiles`（伤害系数注入点）、`UTheManAttributeSetBase`（玩家扣血调试）。

---

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/Public/Core/TheManGameStateBase.h` / `Private/.../TheManGameStateBase.cpp`
  - `BaseCountdownDuration` 60 → 600（10 分钟）
  - 抽出私有 `AdvanceRound(DeltaSeconds)`（倒计时+强度波 while+半场二阶段+归零结束），Tick 与调试快进共用
  - 新增 `DebugSkipTime(float Seconds=150)`（BlueprintCallable）：快进 N 秒，复用 `AdvanceRound`；减过头由 `AdvanceRound` 钳到 0 并触发回合结束
- `Source/TheManTest/Public/Core/TheManPlayerController.h` / `Private/.../TheManPlayerController.cpp`
  - 新增 `DebugSkipTimeAction`（IA UPROPERTY，Input|Debug）+ `HandleDebugSkipTime()` → `GameState->DebugSkipTime()`
  - `SetupInputComponent` 中按 `TestSwitchCharacterAction` 同模式绑定（仅填了 IA 资产才绑）
  - 新增 `Phase2TriggerRemainingFraction`（默认 0.5）
  - 新增广播 `FOnCombatPhaseChanged(int32 NewPhase)` + `OnCombatPhaseChanged`
  - 新增 `ElapsedStrengthWaves` 计数 + `GetElapsedStrengthWaves()`（每广播一波 +1，新回合归零，供中途生成回补）
  - Tick：剩余时间屏幕调试（key=1 原地刷新）+ 半场触发一次 `Broadcast(2)`
  - StartNewRound：记 `CurrentRoundDuration`、重置 `bPhase2Triggered` / `ElapsedStrengthWaves`
  - `OnCountdownExpired_Implementation` 从空壳改为输出调试信息（不真死亡）
- `Source/TheManTest/Public/Characters/Enemy/EnemyBase.h` / `Private/.../EnemyBase.cpp`
  - 新增 `StrengthDamageBonusPerWave`（EditDefaultsOnly，默认 0.2，每波累加百分比）+ `CurrentDamageMultiplier`（运行时，初始 1）+ `GetDamageMultiplier()`
  - `HandleMidRoundStrengthIncrease`（已绑 FEAT-022 `OnMidRoundStrengthIncrease`）：每波 `CurrentStrength++` + `CurrentDamageMultiplier += StrengthDamageBonusPerWave`（钳到 `MaxDamageMultiplier`）
  - **（session42 增补）** 新增 `MaxDamageMultiplier`（EditDefaultsOnly，默认 2.0，ClampMin 1.0）伤害倍率上限；BeginPlay 初始化与每波递增两处均 `FMath::Min(..., MaxDamageMultiplier)` 钳制
  - **（session42 增补）** BeginPlay 伤害倍率改为**逐回合携带**：`RoundCarriedWaves = Max(RoundNumber-1, 0)`，`CurrentDamageMultiplier = 1 + StrengthDamageBonusPerWave × (RoundCarriedWaves + ElapsedWaves)`（钳上限）。原先每回合重置回 1.0，现每个新回合初始攻击力加成比上一回合高一波（被打死/倒计时归零同理）
  - `SetCombatPhase`：只切 `CurrentPhase`（换技能集），**不碰伤害倍率**
  - BeginPlay：读 `GS->GetElapsedStrengthWaves()` 回补 `CurrentStrength` + `CurrentDamageMultiplier`；绑定两个广播
  - BeginPlay 绑定 `OnCombatPhaseChanged → HandleCombatPhaseChanged → SetCombatPhase`（仅切阶段）
- `Source/TheManTest/Private/GAS/Abilities/GA_EnemyShoot.cpp`
  - `SpawnProjectiles`：生成子弹后 `Bullet->Damage *= Enemy->GetDamageMultiplier()`
- `Source/TheManTest/Private/Characters/BaseCharacter/TheManAttributeSetBase.cpp`
  - 玩家（`AFPSCharacterBase`）扣血时红色屏幕调试输出（扣血量 + 剩余血量）

**涉及蓝图资产（需在 UE 编辑器内配置）：**
- boss/敌人蓝图（如 `BP_Phantom`）：`PhaseSkillSets` 数组**至少 2 个元素**，`[1]` 填二阶段技能集；`StrengthDamageBonusPerWave` 按需设置（默认 0.2）
- `BP_TheManGameState`：若覆写 `OnCountdownExpired` 事件需调用 Parent，否则 C++ 调试不执行
- **调试快进按键**：新建一个 IA（如 `IA_DebugSkipTime`，绑某个键），加入 `IMC_Default`；`BP_TheManPlayerController` 的 `DebugSkipTimeAction` 填该 IA 即生效（C++ 绑定与处理已就绪）

**依赖的其他系统 / 功能：**
- FEAT-022 回合系统（GameState 倒计时基座）
- FEAT-032 敌人技能集 `PhaseSkillSets` / `SetCombatPhase` / `UseRandomSkill`
- FEAT-035 `UGA_EnemyShoot` 子弹射击管线
- FEAT-034 `ABulletBase.Damage` + `GE_BulletDamage` 伤害链路

**完成标准（与 feature_list.json 保持一致）：**
- [ ] C++ 编译无错误无新增警告
- [ ] 编辑器：敌人蓝图 `PhaseSkillSets[1]` 填二阶段技能集，`StrengthDamageMultiplier` 配置
- [ ] PIE：回合 10s，屏幕显示剩余时间；半场屏幕提示进入二阶段 + 敌人切阶段
- [ ] PIE：二阶段后敌人技能伤害按系数提升（玩家扣血屏幕数值变大）
- [ ] PIE：倒计时归零输出占位调试信息（不真死亡）

---

## 实现日志

### 2026-06-20 — 功能创建 + C++ 实现（待编译）

- **背景：** 用户要求回合时长改 10s、屏幕显示剩余时间、半场进二阶段、二阶段敌人技能伤害乘强度系数；同时希望玩家扣血与倒计时归零有屏幕调试反馈。
- **设计决策：**
  - 阶段切换沿用 FEAT-022 已有的 multicast 广播模式（参照 `OnMidRoundStrengthIncrease`），新增 `OnCombatPhaseChanged(int32)`，敌人 BeginPlay 订阅 → `SetCombatPhase`。boss 用二阶段技能**无需额外逻辑**：`UseRandomSkill` 本就读 `PhaseSkillSets[CurrentPhase-1]`。
  - 伤害系数注入点选 `GA_EnemyShoot::SpawnProjectiles`（所有射击技能/散射连发子类的公共出膛点），乘在**每发子弹实例的 `Damage`** 上，不改子弹 BP 基础配置。覆盖所有走 `ABulletBase` 管线的技能；未来若有非子弹直接 ApplyGE 的技能需单独读 `GetDamageMultiplier()`。
  - **伤害递增与阶段解耦（用户 session41 修订）**：原设计"二阶段后伤害 ×固定系数"改为复用 `StrengthIncreaseInterval` 每波累加。把 2.5min 这种更细的递增节奏交给 `StrengthIncreaseInterval`（设 150），阶段（半场）只管换技能集。`StrengthIncreaseInterval` 是 FEAT-022 既有变量，无需新建。
  - 剩余时间用 `AddOnScreenDebugMessage(key=1, ...)` 固定 key 原地刷新，避免滚屏。
  - 半场阈值做成可配 `Phase2TriggerRemainingFraction`，每回合 `bPhase2Triggered` 守卫只触发一次。
- **编译结果：** 待编译（本会话改完未编）。

### 2026-06-20-session42 — 伤害倍率逐回合携带 + 上限钳制（用户需求增补）

- **背景：** 用户实测 FEAT-037 死亡循环通过后提出两点：①死亡重开（被打死/时间到都算）后，新回合 boss 初始攻击力加成必须已比上一回合高一波；②增强需要上限，默认最多到初始伤害的 2 倍，且可调。
- **问题根因（①）：** 原 `BeginPlay` 里 `CurrentDamageMultiplier = 1 + 系数 × ElapsedWaves`，`ElapsedWaves` 每回合归零 → 每个新回合伤害加成都从 ×1.0 重来，没有逐回合累积（`CurrentStrength` 那条抽象强度虽随回合涨，但不参与子弹伤害）。
- **实现：**
  - ① `EnemyBase.cpp` BeginPlay：引入 `RoundCarriedWaves = FMath::Max(RoundNumber-1, 0)`，与 `ElapsedWaves` 合计后算倍率。第 1 回合 = 0 波基线（×1.0），第 2 回合起每回合初始 +1 波。死亡两源都过 `HandlePlayerDeath → RoundNumber+1`（FEAT-037），故新回合敌人 BeginPlay 自然读到更高回合数，无需区分死因。
  - ② `EnemyBase.h` 新增 `MaxDamageMultiplier`（EditDefaultsOnly，默认 2.0，ClampMin 1.0，蓝图可调）；BeginPlay 初始化与 `HandleMidRoundStrengthIncrease` 每波递增两处均 `FMath::Min(..., MaxDamageMultiplier)`。
- **触顶节奏：** 默认 +0.2/波、上限 2.0 → 累计 5 波（第 6 回合初始或回合内累计 5 波）后封顶。
- **编译结果：** ✓ session42 全量重编通过。
- **验证：** ✓ 用户 session42 测试通过（逐回合携带 + 上限）。同会话另把 `CountdownReductionPerRound` 默认改 150（2.5min/回合）。

---

## Bug 记录

（暂无）

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | | | |
| 编辑器：PhaseSkillSets[1] + StrengthDamageMultiplier 配置 | | | |
| PIE——剩余时间显示 + 半场切阶段 | | | |
| PIE——二阶段伤害系数生效 | | | |
| PIE——倒计时归零调试占位 | | | |

---

## 最终备注

> - "阶段"（`PhaseSkillSets` 战斗阶段，FEAT-032）与"伤害递增"（`OnMidRoundStrengthIncrease` 每波，FEAT-022 间隔）是两套并行机制，互不冲突；阶段只换技能集。
> - 系数乘在子弹生成瞬间即定，整发飞行命中按缩放后值扣血。
> - **中途生成回补（session41）**：GameState 记 `ElapsedStrengthWaves`（每广播一波 +1，新回合归零）。敌人 `BeginPlay` 读它：`CurrentStrength = BaseStrength + RoundNumber + ElapsedWaves`、`CurrentDamageMultiplier = 1 + StrengthDamageBonusPerWave × ElapsedWaves`，使回合中途刷出的敌人与开局即在场的敌人强度/伤害一致。
> - `OnCountdownExpired` 仍是调试占位，未实现真实玩家死亡 + 自动开新回合（沿用 FEAT-022 设计，由蓝图负责）。

**完成标准全部满足日期：** —
**功能关闭日期：** —
