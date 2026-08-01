# 敌人 AI 与战斗系统

**何时读取：** 修改敌人巡逻/转身/感知/战斗状态、行为树、敌人技能集（阶段×近中远）、敌人 GAS 或敌人动画驱动时。

> 相关：敌人动画状态机细节见 `06-animation.md`；敌人 GAS 技能/技能集见 `10-gas-abilities.md`；属性/死亡见 `04-gas-attributes.md`。本文聚焦 AI 行为与系统关系。

---

## 类层级

```
AEnemyBase（Public/Enemy/）  ← 所有敌人基类，ASC+属性挂自身（无 PlayerState）
  ├── AHumanoidEnemy（.../Enemy/Humanoid/）  ← 人形怪：巡逻/转身/战斗/武器/AI
  │     └── APhantom（.../Humanoid/Phantom/）  ← 幻影（空壳，差异化在蓝图）
  └── ANightmareEnemy（.../Enemy/Nightmare/）  ← 梦魇（直接继承基类，空壳）
```

---

## 文件清单

### 基类 / 属性 / 技能集

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/EnemyBase.h` | ASC + `UEnemyAttributeSetBase` 挂自身；`InitGEClass`；`DefaultAbilities`（常驻技能）；**技能集系统**：`PhaseSkillSets`(阶段数组)/`CurrentPhase`/`SetCombatPhase()`/`UseRandomSkill(Target,Range)`/`GrantAbilities()`/virtual `AimAtTarget()`；`CurrentStrength`；`OnDeath()`。`Tick` 默认关闭 |
| `Private/Enemy/EnemyBase.cpp` | BeginPlay：InitAbilityActorInfo(self,self) + 应用 InitGE + 授予 DefaultAbilities & 所有阶段技能 + 强度初始化（绑 `OnMidRoundStrengthIncrease`）；`UseRandomSkill`（当前阶段+距离档随机→AimAtTarget→TryActivateAbilityByClass）；`OnDeath` 默认 Destroy |
| `Public/Enemy/EnemyAttributeSetBase.h` | 继承 `UTheManAttributeSetBase`，怪物专属属性在此扩展（当前为空） |
| `EEnemySkillRange`（EnemyBase.h 内） | 交战距离档枚举：`Near` / `Mid` / `Far` |
| `FEnemyPhaseSkillSet`（EnemyBase.h 内） | 一个阶段的技能集：`NearAbilities` / `MidAbilities` / `FarAbilities`（各 `TArray<TSubclassOf<UGameplayAbility>>`） |

### 人形怪

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/Humanoid/HumanoidEnemyTypes.h` | `EHumanoidEnemyAIState`：`Patrol` / `Aim` / `SearchRush` / `SearchScan` / `Dead` |
| `Public/Enemy/Humanoid/HumanoidEnemy.h` | 重开 `Tick`；`WeaponMesh`(StaticMesh，挂 `WeaponAttachSocket`=hand_r)；`AimTargetWorld`/`bIsAiming`(public，AIController 写)；`AIState`/`SetAIState`；巡逻路点 `PatrolPoints`；速度参数(PatrolWalkSpeed/CombatWalkSpeed/TurnWalkSpeed)；转身/减速参数；重写 `AimAtTarget` |
| `Private/Enemy/Humanoid/HumanoidEnemy.cpp` | C++ 巡逻：`MoveToNextPatrolPoint`/`OnPatrolMoveCompleted`/`TryTurnOrMove`/`RequestTurn`/`OnTurnComplete`/`ResumeNearestPatrol`；`SetAIState`(Aim:停巡逻+Focus 朝向+加速；回 Patrol:置 bNeedsPatrolResume)；`Tick`(转身旋转 + 接近路点线性减速)；`AimAtTarget`(写 AimTargetWorld) |
| `Public/Enemy/Humanoid/Phantom/Phantom.h` | `APhantom : AHumanoidEnemy`；二阶段透明材质、弹体通道穿透、`ShouldProjectilePassThrough` |
| `Public/Enemy/Components/EnemyMagazineComponent.h` | 通用 20 发弹匣；仅普通自动射击消费，支持空匣判断、Reload 与 AmmoChanged |
| `Public/Enemy/Cover/EnemyCoverPoint.h` | 通用掩体 Actor；StandPoint + 距离/威胁背向/Visibility 遮挡评分选择 |
| `Public/Enemy/Nightmare/NightmareEnemy.h` | `ANightmareEnemy : AEnemyBase`，空壳（非人形，无巡逻/武器逻辑） |

### AI 控制器 / 行为树

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/Humanoid/HumanoidAIController.h` | `UAIPerceptionComponent`；黑板 key static const `BB_TargetActor`("TargetActor") / `BB_LastKnownPlayerLocation`("LastKnownPlayerLocation")；`BehaviorTree`(蓝图子类指定) |
| `Private/Enemy/Humanoid/HumanoidAIController.cpp` | Sight 感知；发现玩家进入 Aim；丢失时写 LastKnown、清 Target/Focus 并调用公共 `StartLostTargetSearch` |
| `Public/Enemy/BTTask_UseCombatSkill.h` | **通用战斗放招节点**：`Range`(近/中/远) + `TargetActorKey`(默认 TargetActor)；读黑板目标→`AEnemyBase::UseRandomSkill`；不绑定具体技能 |
| `Public/Enemy/Humanoid/BTTask_ResumeNearestPatrol.h` | 丢失目标回巡逻：调 `AHumanoidEnemy::ResumeNearestPatrol`（找最近路点续巡逻） |

### 动画（详见 06-animation.md）

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/Humanoid/HumanoidEnemyAnimInstance.h` | 继承 `UBaseLocomotionAnimInstance`，缓存 `AHumanoidEnemy`；输出 `AIState`/`bIsTurning`/`TurnAngle`/`TurnAnimIndex`(0-6)/`bIsDead`/`bIsPatrolScanning`/`PatrolScanAnimIndex`；停步虚拟减速 `StopDecelerationRate`；左手 Two-Bone IK(`grip_l`)；BBBAimIK 变量(`AimAlpha`/`AimAxis`/`AimSourceLocalTransform`/`bIsAiming`…) |
| `Public/Enemy/Humanoid/AnimNotify_TurnComplete.h` | 挂转身动画末尾 → 回调 `AHumanoidEnemy::OnTurnComplete()` 清 bPendingTurn |

---

## AI 状态机（EHumanoidEnemyAIState）

- `Patrol` 巡逻（未发现玩家）——**移动由 C++ 自驱**（MoveToActor 循环 + 转身），BT 只 Wait。
- `Aim` 锁定追击（发现玩家）——停 C++ 巡逻；`SetFocus` 锁朝向（bUseControllerRotationYaw）；CombatWalkSpeed；**移动+放招由 BT 驱动**。
- `SearchRush` 丢失后以 `SearchRushSpeed` 冲向 LastKnownLocation；到达转 `SearchScan`。
- `SearchScan` 复用 Relaxed Fgt 随机环视，`SearchScanDuration` 后找最近巡逻点恢复；无 Nav/Move 失败时安全回 Patrol。
- `Dead` 死亡。

状态切换入口：`AHumanoidEnemy::SetAIState()`（负责停计时器/StopMovement/改朝向模式/置 bNeedsPatrolResume）。

---

## 巡逻系统（C++ 驱动，FEAT-028/024/029）

- `PatrolPoints`（EditInstanceOnly，关卡实例填 `APatrolPoint`）。`PossessedBy` 绑 `OnRequestFinished`，延迟启动首次 `MoveToNextPatrolPoint`。
- 到点：`OnPatrolMoveCompleted` → 等待(WaitTime)/扫视 → `TryTurnOrMove`（偏角 > 阈值先 `RequestTurn` 转身，`OnTurnComplete` 再续走）。
- Walk→Stop：接近路点 Tick 线性降速 + AnimInstance 虚拟减速，blend space 平滑过渡（FEAT-029）。
- 所有巡逻函数开头 `if (AIState != Patrol) return` 守卫——战斗时不被巡逻干扰。

---

## 感知 → 战斗流（FEAT-032，session40 启用）

```
Sight 感知玩家(1500cm/60°)
  → OnTargetPerceptionUpdated(成功) → BB.TargetActor=玩家 + SetFocus + SetAIState(Aim)
  → AIController::Tick 每帧 AimTargetWorld=玩家位置, bIsAiming=true（供 AimIK + 子弹方向）
  → BT 战斗 Sequence [Decorator: TargetActor IsSet, Abort Both]:
       MoveTo(TargetActor, AcceptanceRadius=交战距离)
       → BTTask_UseCombatSkill(Range) → UseRandomSkill(当前阶段对应档随机一个技能)
       → Wait(出招间隔)
丢失玩家(1800cm)
  → BB.LastKnownPlayerLocation + 清 TargetActor + ClearFocus
  → StartLostTargetSearch → SearchRush(MoveTo LastKnown) → SearchScan(Relaxed 随机环视)
  → SetAIState(Patrol) + ResumeNearestPatrol
```

### 行为树结构（BT_HumanoidEnemy，编辑器）

```
Root → Selector
  ├── Sequence [Decorator: Blackboard TargetActor Is Set, Observe aborts: Both]
  │   ├── Move To              (Key: TargetActor, Acceptance Radius: 交战距离)
  │   ├── BTTask_UseCombatSkill (Range: 近/中/远)
  │   └── Wait                 (出招间隔)
  └── Wait 0.1                 (无目标 → C++ 巡逻自驱)
```

> 距离分近/中/远三支（距离 Decorator）当前**未做**；战斗序列先放一个 BTTask_UseCombatSkill。

---

## 技能集系统（阶段 × 近/中/远，FEAT-035，放 AEnemyBase 通用）

- 二维：`PhaseSkillSets[阶段]` → `FEnemyPhaseSkillSet{Near/Mid/FarAbilities}`。`CurrentPhase` 默认 1，`SetCombatPhase()` 切换（第二阶段换整组）。
- `BeginPlay` 经 `GrantAbilities` 授予所有阶段所有档技能。
- `UseRandomSkill(Target, Range)`：从随机起点轮询当前阶段/距离档能力；空匣等激活条件失败时继续尝试同档其他能力。
- 技能 = `UGA_EnemyShoot` 蓝图子类（技能与子弹绑定，复用子弹管线；不同开火逻辑重写 `SpawnProjectiles`）。详见 `10-gas-abilities.md`。
- 触发不走 GameplayEvent，按类激活（`BTTask_UseCombatSkill` → `UseRandomSkill`），故一敌可多技能各自独立。

---

## 当前状态 / 注意

- ✅ 巡逻(FEAT-028)、转身(FEAT-024)、Walk→Stop(FEAT-029)、左手 IK(FEAT-030) 已验证。
- 🔶 FEAT-032（感知+BT 战斗）：session40 已启用 C++ 感知/Tick/BTTask，**编辑器 BT 战斗序列与 PIE 验证待完成**。
- ✅ FEAT-031（BBBAimIK 脊柱瞄准）：session40 恢复并验证通过，敌人 Aim 时上半身跟随玩家。（aim 取点当前用 GetActorLocation，如需更高改 eyes）
- 🔶 FEAT-035（敌人射击）：C++ 完成，待编辑器配 BGA_EnemyShoot + 武器 Muzzle socket + BP_Phantom PhaseSkillSets。
- ✅ FEAT-058～063：搜索链、通用掩体、弹匣/三连发/扫射/换弹、Phantom 找掩体与二阶段已实现；Phantom 通过 `PhaseSkillSets` 数据注入能力，公共 BT 不依赖 Phantom 类型。
- ✅ FEAT-058 session126：Patrol 与 SearchRush 均优先使用 NavMesh；请求立即/异步失败时改用 CharacterMovement 直移。到达后仍复用等待、Relaxed 随机环视、下一路点、SearchScan 与最近巡逻点恢复；`SetPatrolPoints` 支持运行时生成敌人。TestMap 无 RecastNavMesh 的两条 PIE 路径均验证真实移动与扫描。
- ✅ FEAT-064：Aim 状态移动由 `AHumanoidAIController` 的公共距离环带接管。BT 的 Actor `MoveTo(TargetActor)` 在 Aim 时只作为技能序列门槛并返回 AlreadyAtGoal；真实移动目标为 NavMesh 投影后的战术 Location。默认保持 700±150 cm：近距后撤+侧移、远距收拢+侧移、环带内切向绕行；无 NavMesh 时回退 CharacterMovement 直接移动。实际局部速度继续由 `UBaseLocomotionAnimInstance` 计算 Direction，驱动子 AnimBP 二维 Aim BlendSpace。
