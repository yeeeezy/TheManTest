# FEAT-032 人形怪 AI 基础框架（行为树 + AIController + 黑板）

**状态：** in_progress  
**创建：** 2026-06-13  
**最后更新：** 2026-06-18-session40（感知正式启用 + 接入 FEAT-035 射击）

---

## 目标

为人形怪建立可扩展的 AI 基础：视觉感知检测玩家、行为树驱动高层状态切换（巡逻↔追击），C++ 巡逻逻辑保持不动，BT 只负责状态层。

---

## 架构决策

**BT 只管高层状态，C++ 负责巡逻移动**

- C++ 巡逻（MoveToActor 循环）已经工作正常，不移入 BT
- BT 结构：Selector → 追击序列（有目标时）+ Wait 0.1s（无目标时让 C++ 巡逻自驱）
- AIState 切换时 SetAIState 负责 StopMovement / 重启巡逻，BT 不直接调用移动 API

**感知回调驱动状态，不轮询**

- `OnTargetPerceptionUpdated` 委托：感知到玩家 → SetAIState(Combat)；丢失 → SetAIState(Patrol)
- 黑板同步更新，供 BT Decorator 读取

---

## C++ 实现

### 新增文件

| 文件 | 内容 |
|---|---|
| `Public/Characters/Enemy/Humanoid/HumanoidAIController.h` | 声明；BB key static const；UAIPerceptionComponent；OnTargetPerceptionUpdated |
| `Private/Characters/Enemy/Humanoid/HumanoidAIController.cpp` | 构造（SightRadius=1500, LoseSightRadius=1800, PeripheralVision=60°）；OnPossess 运行 BT + 绑定委托；感知回调写黑板 + SetAIState |

### 修改文件

| 文件 | 变更 |
|---|---|
| `HumanoidEnemy.cpp` | include HumanoidAIController.h；构造函数 AIControllerClass 改为 AHumanoidAIController |
| `HumanoidEnemy.cpp::SetAIState` | 切 Combat：ClearTimer + StopMovement；切 Patrol from Combat：MoveToNextPatrolPoint() |
| `HumanoidEnemy.cpp::MoveToNextPatrolPoint` | 开头加 `if (AIState != Patrol) return` |
| `HumanoidEnemy.cpp::OnPatrolMoveCompleted` | 开头加 `if (AIState != Patrol) return` |

### 黑板 Key 名称（static const FName）

| Key | 类型 | 用途 |
|---|---|---|
| `BB_TargetActor = "TargetActor"` | Object(Actor) | BT Decorator 判断是否有追击目标 |
| `BB_LastKnownPlayerLocation = "LastKnownPlayerLocation"` | Vector | 丢失后最后已知位置（SearchRush 预留） |

---

## 待完成（编辑器操作）

- [ ] 编译 C++（Development Editor / Win64）
- [ ] 创建 `BB_HumanoidEnemy` 黑板资产（Content/Characters/Enemy/Humanoid/）
  - Key: `TargetActor`（Object，Base Class: Actor）
  - Key: `LastKnownPlayerLocation`（Vector）
- [ ] 创建 `BT_HumanoidEnemy` 行为树（同目录）
  ```
  Root
  └── Selector
      ├── Sequence [Decorator: Blackboard(TargetActor != None, Abort: Both)]
      │   ├── Move To (Target: TargetActor, AcceptanceRadius: 200)
      │   └── Wait (0.5s)
      └── Wait (0.1s)
  ```
- [ ] 创建 `BP_HumanoidAIController`（父类 AHumanoidAIController）
  - BehaviorTree = BT_HumanoidEnemy
- [ ] `BP_Phantom` → Pawn → AI Controller Class = BP_HumanoidAIController
- [ ] PIE 全流程验证

---

## Session40 更新：正式启用感知 + 接入射击（FEAT-035）

### C++ 改动

| 文件 | 变更 |
|---|---|
| `HumanoidAIController.cpp::OnTargetPerceptionUpdated` | 解除 TEMP 禁用。命中玩家：`BB.TargetActor=玩家` + `SetFocus(玩家)` + `SetAIState(Aim)`；丢失：`BB.LastKnownPlayerLocation=刺激位置` + `ClearValue(TargetActor)` + `ClearFocus` + `SetAIState(Patrol)`（SearchRush/Scan 留待 FEAT-026）。 |
| `HumanoidAIController.cpp::Tick` | 解除 TEMP（原 `bIsAiming=false`）。Aim 状态下每帧 `Enemy->AimTargetWorld=玩家位置` + `bIsAiming=true`（供 AimIK + 子弹方向）；非 Aim 置 `bIsAiming=false`。include GameplayStatics。 |
| `BTTask_UseCombatSkill.h/.cpp`（新建，`Characters/Enemy/`，**通用非人形怪专属**） | 战斗放招节点：读黑板目标 → `AEnemyBase::UseRandomSkill(Target, Range)`。节点 UPROPERTY `Range`(近/中/远) + `TargetActorKey`(默认 TargetActor)。**不绑定具体技能**——技能由敌人 PhaseSkillSets 数据决定。 |

> 注：原 AIState 枚举 `Combat` 已是 `Aim`（session34 改名）。感知发现玩家 → Aim。

### 技能集系统（放 AEnemyBase 通用，session40 用户两轮迭代定稿）

- 阶段 × 距离二维：`AEnemyBase.PhaseSkillSets`（`TArray<FEnemyPhaseSkillSet>`，index0=阶段1…）；每个 `FEnemyPhaseSkillSet` 内含 `NearAbilities/MidAbilities/FarAbilities` 三组。
- 枚举 `EEnemySkillRange{Near,Mid,Far}`。`CurrentPhase`(默认1，`SetCombatPhase` 切换；距离判定先不做)。
- `AEnemyBase::UseRandomSkill(Target, Range)`：当前阶段技能集 → 对应距离档 → 随机一个 → `AimAtTarget(Target)`(virtual，AHumanoidEnemy 重写写 AimTargetWorld) → `TryActivateAbilityByClass`。
- `BeginPlay` 授予 DefaultAbilities + 所有阶段所有距离档技能（`GrantAbilities` helper）。
- 旧的 `BTTask_EnemyShoot` / `AHumanoidEnemy::FireAbility` 已删除，由本系统取代。

### 行为树最终结构（编辑器，需更新 BT_HumanoidEnemy）

```
Root
└── Selector
    ├── Sequence  [Decorator: Blackboard "TargetActor" Is Set, Observe aborts: Both]
    │   ├── Move To             [Blackboard Key: TargetActor, Acceptance Radius: 交战距离]
    │   ├── BTTask_UseCombatSkill [Range = Near/Mid/Far]   ← 当前先放一个(如 Mid)，距离分支以后再加
    │   └── Wait                [出招间隔, 如 1.0s, Random Deviation 0.3]
    └── Wait 0.1   （无目标 → C++ 巡逻自驱）
```
- 距离分档(近/中/远三支 + 距离 Decorator)**先不做**（用户：先不管射程）；现在战斗序列放一个 BTTask_UseCombatSkill 即可，以后要分档时复制该节点改 Range + 套距离 Decorator。
- Decorator Abort=Both：丢失目标立刻中断回 Selector → Wait 0.1 → C++ 巡逻（SetAIState(Patrol) 已置 bNeedsPatrolResume，BTTask_ResumeNearestPatrol 续巡逻）。

### 待完成（编辑器）

- [ ] 更新 `BT_HumanoidEnemy`：战斗 Sequence 里 MoveTo 后加 `BTTask_UseCombatSkill`(Range 先填一个) + `Wait`。
- [ ] 确认 `BB_HumanoidEnemy` 有 `TargetActor`(Object/Actor) 与 `LastKnownPlayerLocation`(Vector)。
- [ ] `BP_Phantom`：`PhaseSkillSets` 加 1 个元素(阶段1)，其 Near/Mid/Far 任一组填入 `BGA_EnemyShoot`（与 BT 节点的 Range 对应）。
- [ ] `BP_HumanoidAIController` BehaviorTree=BT_HumanoidEnemy；`BP_Phantom` AI Controller Class=BP_HumanoidAIController。
- [ ] 感知 gotcha：若玩家不被察觉，检查玩家是否被 Sight 视为可感知源（默认 auto-register pawns + bDetectNeutrals=true 应可），必要时加 AIPerceptionStimuliSource 或设 Team。
- [ ] PIE：玩家进 1500cm 视野 → 敌人转向+追击+随机放招扣血；离开 1800cm → 回巡逻。

## Bug 日志

（无）
