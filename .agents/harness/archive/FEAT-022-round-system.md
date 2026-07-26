# [FEAT-022] 回合系统（倒计时 + 怪物强度递增）

**创建日期：** 2026-06-10
**状态：** done
**Archive 文件：** `archive/FEAT-022-round-system.md`

---

## 功能概述

全局回合驱动系统，放在 `ATheManGameStateBase`。核心规则：
- 每回合有倒计时，倒计时归零 → 玩家死亡 → 新回合开始
- 每经历一个回合，下一回合的倒计时时长减少（有下限）
- 怪物基础强度 = 当前回合数
- 回合进行中每隔固定秒数，怪物强度额外 +1

---

## 参数设计

| 参数 | 类型 | 说明 |
|---|---|---|
| `BaseCountdownDuration` | float | 第一回合倒计时（秒），默认 60 |
| `CountdownReductionPerRound` | float | 每回合减少的时长（秒），默认 5 |
| `MinCountdownDuration` | float | 倒计时下限（秒），默认 10 |
| `StrengthIncreaseInterval` | float | 回合内怪物强度增加间隔（秒），默认 10 |

## 运行时状态

| 变量 | 说明 |
|---|---|
| `RoundNumber` | 当前回合数（从 1 开始） |
| `TimeRemaining` | 当前回合剩余时间（UI 读取） |
| `CurrentMonsterStrength` | 当前怪物强度（Spawner 读取） |

## 事件（BlueprintNativeEvent）

| 事件 | 触发时机 |
|---|---|
| `OnRoundStarted(RoundNumber, Duration)` | 新回合开始 |
| `OnCountdownExpired()` | 倒计时归零，蓝图负责杀死玩家并调用 `StartNewRound()` |
| `OnMonsterStrengthChanged(NewStrength)` | 强度变化，Spawner 响应 |

---

## 完成标准

- [ ] `ATheManGameStateBase` C++ 编译无错误无警告
- [ ] `ATheManGameModeBase` 指定 `GameStateClass = ATheManGameStateBase`
- [ ] 蓝图中创建 `BP_TheManGameState`，覆写三个事件
- [ ] PIE 测试：倒计时正常递减，归零触发 `OnCountdownExpired`，新回合倒计时缩短，怪物强度递增

---

## 实现日志

### 2026-06-10 — 功能创建 + C++ 实现

- 新建 `ATheManGameStateBase`（Public/Core/ + Private/Core/）
- 新建 `ATheManGameModeBase` 更新 GameStateClass

---

## Bug 记录

（暂无）

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
