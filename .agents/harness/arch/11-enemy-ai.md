# 敌人 AI 与战斗系统

- FEAT-081 第三批主 BT：`ACoreMorphAIController` 真正 Possess 唯一头领；`BT_CoreMorphBoss`／`BB_CoreMorphBoss` 位于头领 AI 目录。Selector 内蝠鲼分支 Flight GA→Reassemble GA；蝎子分支按范围／冷却选择 Face→当前阶段近距技能、Approach 或 Idle。`BTTask_CoreMorphAction` 等待并取消具体活动能力句柄；攻击三个动作阶段属于 GA，不拆成三个 BT 技能。`BTService_CoreMorphTarget` 选择检查目标／LastThreat／玩家并更新形态和距离，形态与阶段独立。
- 八足运动沿用源的地面接触、身体扫掠和局部扇形避障，不是 NavMesh 全局寻路；尚未实现绕过任意复杂障碍的路径规划。ScorpionCombat.bEnabled 默认 false，检查地图 V/M 开启；旧飞行／重组检查地图保持原入口。取消、目标销毁、死亡和退出必须同时清 GA／GE 与运动状态；完整三枪适配仍属第四批。

- 当前OnDeath不立即Destroy：先取消ASC技能、停止AI Brain/移动并解除Controller，清Actor计时器/波次订阅，关闭角色Tick与胶囊，进入布娃娃；CorpseLifetime默认5游戏秒后Destroy。Humanoid AIState置Dead，武器网格无碰撞并跟手部骨骼；Phantom取消隐身。UseRandomSkill拒绝死人。

**何时读取：** 修改敌人巡逻/转身/感知/战斗状态、行为树、敌人技能集（阶段×近中远）、敌人 GAS 或敌人动画驱动时。

> 相关：敌人动画状态机细节见 `06-animation.md`；敌人 GAS 技能/技能集见 `10-gas-abilities.md`；属性/死亡见 `04-gas-attributes.md`。本文聚焦 AI 行为与系统关系。

---

## 类层级

- 2026-09-05：Humanoid基类自动接共享骨架无关后处理ABP，不再依赖Phantom资产槽。ExplosionHitReaction可配置38度幅度、.055秒攻击、.85秒恢复、.045秒头肩跟随延迟、7cm腿部缓冲及骨骼映射。Rig降低pelvis并解析求解双腿，使脚保持当前输入动画位置而非固定世界位置，允许移动时受击。共享Rig不含Phantom模型/骨架导入引用。

```
AEnemyBase（Public/Enemy/）  ← 所有敌人基类，ASC+属性挂自身（无 PlayerState）
  ├── AHumanoidEnemy（.../Enemy/Humanoid/）  ← 人形怪：巡逻/转身/战斗/武器/AI
  │     └── APhantom（.../Humanoid/Phantom/）  ← 幻影（空壳，差异化在蓝图）
  ├── ANightmareEnemy（.../Enemy/Nightmare/）  ← 梦魇（直接继承基类，空壳）
  └── ABossEnemyBase（.../Enemy/Boss/） ← 头领公共语义层，不放具体变形
        └── ACoreMorphBoss（.../Boss/CoreMorph/） ← 蝠鲼／蝎子，FEAT-081 分批迁移中
```

---

## 文件清单

### CoreMorph 头领（FEAT-081，第一批）

- 第二批开始：用户已验收飞行观感，授权迁入变形重组。具体头领持有 ReassemblyComponent；飞行结束后的自动变形衔接只在新 L_CoreMorphReassembly Review 地图启用，正式决策仍等待第三批专属 BT。GA／GE／Cue 管理变形事务和表现，完成后停在静态蝎子，不启动源独立移动 Actor。

- `Enemy/Boss/BossEnemyBase.h` 为公共语义层；`CoreMorph/CoreMorphBoss.h/.cpp` 持有具体形态和飞行组件，继承一份 ASC／Health。没有人形骨骼、击退、布娃娃或血肉受击 Cue。
- `CoreMorph/Movement/CoreMorphFlightPath` 只保留源 FEAT058 参考路线的位置数学；`CoreMorphFlightComponent` 管理 154 个同 Owner 静态分件。Actor／Capsule 在编辑态直接位于身体主体，分件用相对变换跟随摆放；用逆起飞偏移乘 Actor Transform 推导参考路线坐标，BeginPlay 固定该坐标并将分件切为绝对世界姿态，实际 Pawn 根位置跟随飞行。CharacterMovement 在飞行预览中关闭。检查地图根位置为 (-16000,0,2500)，对应原路线原点 (0,0,900)，静态装配位置不变。
- `CoreMorph/GAS/Abilities/GA_CoreMorphFlight` 管理飞行生命周期；`GAS/Effects/GE_CoreMorphManta` 持有形态 Tag。阶段仍走 EnemyBase 的 `PhaseSkillSets/SetCombatPhase`，预览复位不重置阶段、Health 或技能授予。
- 第一批全身反馈：`FCoreMorphFlightMotion` 只接收世界位置／DeltaTime，从速度、升降、加速和转向计算身体朝向／侧倾及翅膀动作；尾巴按距离追随有界三维运动历史并叠加错相摆动。种子控制平滑随机节奏、波幅、左右差异，刚性核心仍与躯体统一运动。旧 TailMotion 已合并删除，SourcePose／DivePose 固定动作编排已移除。暂停／取消／死亡冻结，复位清空状态与轨迹。
- 可选 `ACoreMorphFlightRoute` 提供可在编辑器修改的 Spline，FlightComponent.FlightRoute 选择路线、RouteSpeed 控制速度；从首点开始，开放路线末端结束、闭合路线循环，无固定表现时点。空引用仍走源参考路径。MotionRandomness 默认 .18、0 关闭，MotionSeed 默认 0（复位选新种子）、非零可复现。当前是路线跟随与实时表现，正式战斗 AI／避障仍属于后续工作。
- 正式资产 `/Game/Enemy/Boss/CoreMorph/{Blueprint,Data,Meshes,Materials}`。`BP_CoreMorphBoss` 引用 `DA_CoreMorphVisualLayout` 和已有 `GE_EnemyBase_Init`。
- 本批尚未接战斗 BT，检查地图直接请求飞行 GA；默认源参考路线在 13.4 秒粒子释放前冻结，自定义 Spline 按实际末端／循环条件结束。主 BT／形态子树、变形、蝎子运动和攻击按后续批次接入；不得将当前预览误认为完整战斗。
- 检查入口 `/Game/Maps/CoreMorph/L_CoreMorphFlight`，V 播放、R 复位、P 暂停、F 切换相机；控件只属于地图专属 `CoreMorph/Review/CoreMorphFlightReview`。
- 多路线入口 `/Game/Maps/CoreMorph/L_CoreMorphRoutes`，同一 Boss／ASC／GA 配三个 Spline；Review.Routes 保存路线引用、英文 Label 与速度。PIE 自动开始 Gentle Climb（约 20 秒），1／2／3 分别重播 Gentle Climb／S-Turns（约 22 秒）／Orbit and Dive（约 27 秒）。切换先取消旧 GA，再复位并启动新路线；原地图 Routes 为空，不新增键绑定或自动播放。
- Motion 接收提前加速意图和 PowerStroke；Spline 先挥翼再逐步提速，因此实际时长增加起步发力与加速耗时。慢／快轴向侧滚属于当前飞行 GA 内的表现机动，组件拒绝非飞行／暂停／死亡时请求，随机触发由距离和实时状态决定。阶段、形态及技能授予逻辑不变；编译与实际 PIE 侧滚、取消／死亡／退出检查通过。

### 基类 / 属性 / 技能集

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/EnemyBase.h` | ASC + `UEnemyAttributeSetBase` 挂自身；通用屏幕空间 `EnemyHealthBar` 组件绑定 Health/MaxHealth；`InitGEClass`；`DefaultAbilities`（常驻技能）；**技能集系统**：`PhaseSkillSets`(阶段数组)/`CurrentPhase`/`SetCombatPhase()`/`UseRandomSkill(Target,Range)`/`GrantAbilities()`/virtual `AimAtTarget()`；`CurrentStrength`；`OnDeath()`。FEAT-073 新增 `ReactToProjectileHit` 与可刷新限时 `ApplyMovementSlow`；`SetDesiredMaxWalkSpeed` 将状态基础速度和临时倍率分离。`HitReactionCueTag` 由每类敌人配置，Health 实际扣减后由目标 ASC 执行受击 Cue。默认 `GameplayCue.Character.Enemy.Hit` 对应 Registry 可发现资产 `/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit`，作为所有敌人的公共兜底；同 Tag 不会自动区分角色，需要独立表现时应新增如 `GameplayCue.Character.Phantom.Hit` 并在具体敌人 BP 覆盖。`Tick` 默认关闭。 |
| `Private/Enemy/EnemyBase.cpp` | BeginPlay：InitAbilityActorInfo(self,self) + 应用 InitGE + 初始化/刷新头顶血条 + 授予 DefaultAbilities & 所有阶段技能 + 强度初始化（绑 `OnMidRoundStrengthIncrease`）；`UseRandomSkill`（当前阶段+距离档随机→AimAtTarget→TryActivateAbilityByClass）；`OnDeath` 隐藏血条后默认 Destroy |
| `Public/Enemy/UI/EnemyHealthBarWidgetBase.h` | `AEnemyBase` 共用的轻量原生血条 Widget；黑色边框/暗红底/亮红当前血量，无需敌人 Blueprint 资产 |
| `Public/Enemy/EnemyAttributeSetBase.h` | 继承 `UTheManAttributeSetBase`，怪物专属属性在此扩展（当前为空） |
| `EEnemySkillRange`（EnemyBase.h 内） | 交战距离档枚举：`Near` / `Mid` / `Far` |
| `FEnemyPhaseSkillSet`（EnemyBase.h 内） | 一个阶段的技能集：`NearAbilities` / `MidAbilities` / `FarAbilities`（各 `TArray<TSubclassOf<UGameplayAbility>>`） |

### 人形怪

| 文件 | 关键内容 |
|---|---|
| `Public/Enemy/Humanoid/HumanoidEnemyTypes.h` | `EHumanoidEnemyAIState`：`Patrol` / `Aim` / `SearchRush` / `SearchScan` / `Dead` |
| `Public/Enemy/Humanoid/HumanoidEnemy.h` | 重开 `Tick`；`WeaponMesh`(StaticMesh，挂 `WeaponAttachSocket`=hand_r)；`AimTargetWorld`/`bIsAiming`(public，AIController 写)；`AIState`/`SetAIState`；巡逻路点 `PatrolPoints`；速度参数(PatrolWalkSpeed/CombatWalkSpeed/TurnWalkSpeed)；转身/减速参数；重写 `AimAtTarget` |
| `Private/Enemy/Humanoid/HumanoidEnemy.cpp` | C++ 巡逻：`MoveToNextPatrolPoint`/`OnPatrolMoveCompleted`/`TryTurnOrMove`/`RequestTurn`/`OnTurnComplete`/`ResumeNearestPatrol`；`SetAIState`(Aim:停巡逻+Focus 朝向+加速；回 Patrol:置 bNeedsPatrolResume)；`Tick`(转身旋转 + 接近路点线性减速)；`AimAtTarget`(写 AimTargetWorld)。FEAT-073 起，玩家子弹命中会立即面向玩家、进入 Aim，并写 AI Focus/Blackboard TargetActor；所有状态速度均通过减速倍率入口。 |
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

### FEAT-080 临时 Phantom 静止测试配置

- 2026-09-05：APhantom新增默认开启的`Phantom|Testing → Stationary Hit Test`。BeginPlay解除并销毁自身AI控制器、清角色计时器、关闭角色Tick与Movement；ReactToProjectileHit跳过父类转向/Focus，Mesh与受击组件仍更新。适用于固定方向受击测试。取消勾选并重启PIE恢复原生AI路径；以下测试树/零速度配置仍需恢复才回正式战斗。

- `/Game/Enemy/Humanoid/Phantom/AI/BT_Phantom_TestIdle`：`Root -> Sequence -> Wait(86400s)`，无移动和攻击节点。
- `/Game/Enemy/Humanoid/Phantom/AI/BP_Phantom_TestIdleAIController`：公共 `BP_HumanoidAIController` 的 Phantom 专用测试副本，运行静止树。
- `BP_Phantom` 当前临时指向该测试 Controller，四项状态速度与 CharacterMovement `MaxWalkSpeed` 均为 0，用作爆炸弹命中/范围逻辑的静止目标。
- 恢复正式 AI 时改回共享 `BP_HumanoidAIController_C`，并恢复 `Patrol/Combat/Turn/SearchRush = 150/300/50/600`、`MaxWalkSpeed=600`。

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
- ✅ FEAT-071：Phantom 子弹 Damage=6；射击散布扩大为3°基础/0.8°逐发/9°上限，移动惩罚2°、恢复2°/s；每波伤害成长10%、最大倍率1.5。只覆盖 Phantom 蓝图，不修改通用 Enemy C++ 默认。
- ✅ FEAT-058 session126：Patrol 与 SearchRush 均优先使用 NavMesh；请求立即/异步失败时改用 CharacterMovement 直移。到达后仍复用等待、Relaxed 随机环视、下一路点、SearchScan 与最近巡逻点恢复；`SetPatrolPoints` 支持运行时生成敌人。TestMap 无 RecastNavMesh 的两条 PIE 路径均验证真实移动与扫描。
- ✅ FEAT-064：Aim 状态移动由 `AHumanoidAIController` 的公共距离环带接管。BT 的 Actor `MoveTo(TargetActor)` 在 Aim 时只作为技能序列门槛并返回 AlreadyAtGoal；真实移动目标为 NavMesh 投影后的战术 Location。默认保持 700±150 cm：近距后撤+侧移、远距收拢+侧移、环带内切向绕行；无 NavMesh 时回退 CharacterMovement 直接移动。实际局部速度继续由 `UBaseLocomotionAnimInstance` 计算 Direction，驱动子 AnimBP 二维 Aim BlendSpace。

### CoreMorph 蓄力地面雷爆更新

尾刺仍由主 BT 请求当前阶段 NearAbilities 中的 `GA_CoreMorphTailStrike`，战斗阶段与形态不耦合。此技能现在锁定目标脚下可用静态地面：2 秒蓄力红圈预警 → 尾尖击地 → GA 结算一次球形 GE 范围伤害 → 收回。目标移动不会拖动本次红圈／伤害中心；刺出途中墙体阻挡不产生雷爆。视效由两个独立 Gameplay Cue 管理，不参与命中判定。全部求解仍位于这只具体头领的 ScorpionCombat／Movement，公共 Boss 层未增加专属逻辑。

### CoreMorph 空中远程决策（2026-09-13）

正式 BT_CoreMorphBoss 的 Manta 分支改为 SimpleParallel：主任务 Flight GA，背景 Selector 按 CanBombard 请求当前阶段 FarPhaseSkill，否则短暂 Wait；WaitForBackground 使飞行结束后等待当前背景技能，再 Reassemble。CanBombard 由原 Target Service 检查具体组件、Attacking／MissileCooldown Tag；远程分支使用 LowerPriority 中止，避免攻击 Tag 中止自身。手动 M 明确取消剩余导弹。
Near／FarPhaseSkill 任务比较启动前后的 Active Spec，保存自己新启动的技能句柄；任务中止仅取消自身技能，避免误取消并发飞行。Scorpion 分支保留。第一阶段 Near=TailStrike、Far=MissileBarrage；默认 Flight／Reassemble，共4唯一技能。闭合路线持续飞行；MantaCombat 地图手动T施放，Scorpion地图V走正式BT自动远程→变形→近战。
