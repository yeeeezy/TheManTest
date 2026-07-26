---
feature_id: FEAT-024
name: Patrol 状态机
status: in_progress
created: 2026-06-11
closed: ~
---

## 目标

在 ABP_HumanoidEnemy 中搭建巡逻层状态机，覆盖巡逻阶段的全部动画过渡：
Idle → Patrol_Walk → Stopping（减速踏步）→ 回 Idle；以及原地转身 Turn_L90 / Turn_R90 / TurnAround。

## 架构决策

- **UAnimNotify_TurnComplete**：C++ AnimNotify 类，挂在三个 Turn 动画末尾帧，触发时调用 `AHumanoidEnemy::OnTurnComplete()`，清除 `bPendingTurn` 标志，通知 BehaviorTree 继续。
- **状态机结构**：Patrol 大状态机，Idle / Patrol_Walk_Run / Stopping / Turn_L90 / Turn_R90 / TurnAround / Dead。
- **Patrol_Walk_Run 内部**：使用 `BS_HumanoidPatrol`（BlendSpace1D，X 轴 Speed），采样点覆盖 Walk → Run，当前巡逻速度慢只用到 Walk 段，后续提速无需改状态机。
- **设计决策**：状态名改为 `Patrol_Walk_Run`（原 `Patrol_Walk`）为后续提速/冲刺扩展预留；BlendSpace1D 优于单一动画，代价极低。
- **过渡条件**：
  - `Idle → Patrol_Walk_Run`：Speed > 10
  - `Patrol_Walk_Run → Stopping`：Speed ≤ 10（或 BT 发出停步信号）
  - `Stopping → Idle`：动画播完（remaining fraction ≤ 0.1）
  - `任意 → Turn_L90/R90/TurnAround`：bIsTurning == true，TurnAngle 决定选哪个
  - `Turn_xxx → Idle`：bIsTurning == false（AnimNotify 触发后自动满足）
  - `任意 → Dead`：bIsDead == true（Any State 节点）

## C++ 工作（本功能）

| 文件 | 说明 |
|---|---|
| `Public/Characters/Enemy/Humanoid/AnimNotify_TurnComplete.h` | AnimNotify 声明 |
| `Private/Characters/Enemy/Humanoid/AnimNotify_TurnComplete.cpp` | `Notify()` → Cast → `OnTurnComplete()` |

## 蓝图工作（编辑器完成）

1. 新建 `ABP_HumanoidEnemy`（`Content/Characters/Enemy/Humanoid/`），Parent Class = `UHumanoidEnemyAnimInstance`，骨架 = Phantom/对应人形怪骨架
2. AnimGraph 输出链：`State Machine（PatrolSM）→ Output Pose`
3. PatrolSM 内添加 6 个状态：`Idle / Patrol_Walk / Stopping / Turn_L90 / Turn_R90 / TurnAround`
4. 按「过渡条件」一节配置所有 Transition Rule
5. 三个 Turn 动画资产末尾帧添加 `AnimNotify_TurnComplete`
6. 全局过渡：`bIsDead == true → Dead` 状态（可单独小状态机或 Any State）

## 动画资产依赖

| 状态 | 所需动画 |
|---|---|
| Idle | 待机循环（无武器） |
| Patrol_Walk_Run | `BS_HumanoidPatrol`（BlendSpace1D，Speed 0→Walk→Run） |
| Stopping | 减速踏步（可复用 Walk 最后几帧或专门资产） |
| Turn_L90 | 原地左转 90° |
| Turn_R90 | 原地右转 90° |
| TurnAround | 原地转 180° |

> 若无对应动画资产，可临时用 Idle 占位，待美术补充后替换。

## 完成标准

- [ ] C++ 编译无错误无警告（AnimNotify_TurnComplete）
- [ ] ABP_HumanoidEnemy 在编辑器编译无错误，Parent Class = UHumanoidEnemyAnimInstance
- [ ] Patrol 层四类状态（Idle / Walk / Stop / Turn）过渡条件正确
- [ ] AnimNotify TurnComplete 正确回调 OnTurnComplete()
- [ ] PIE 测试：敌人巡逻走路、到端点停步、请求转身后播放转身动画并完成

## 实现日志

### 2026-06-11（Session19）
- 确认 FEAT-023 完成，开始 FEAT-024
- 新建 `UAnimNotify_TurnComplete` C++ 类（`Humanoid/` 目录），调用 `OnTurnComplete()`
- 待编译 + 编辑器搭建 ABP_HumanoidEnemy 状态机
