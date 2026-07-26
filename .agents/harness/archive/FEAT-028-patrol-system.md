---
feature_id: FEAT-028
name: 人形怪巡逻系统
status: in_progress
created: 2026-06-11
closed: ~
---

## 目标

为场景中放置的人形怪提供简易路点巡逻能力：设计师在编辑器中点选若干 APatrolPoint Actor 填入数组，敌人运行时按顺序循环走完所有路点，每个路点可配置等待时间。

## 架构决策

- **APatrolPoint 轻量 Actor**：只有位置信息 + WaitTime，用 ArrowComponent 在编辑器可见。不含逻辑。
- **PatrolPoints 用 EditInstanceOnly**：每个放置在场景的敌人实例独立配置路线，不影响蓝图 CDO。
- **直接使用 AAIController**：不建自定义 AIController，用默认 AAIController + MoveToActor，配合 NavMesh 即可。
- **OnRequestFinished 绑定**：在 PossessedBy 时绑定 PathFollowingComponent 委托，到达路点事件驱动，无需 Tick 轮询。
- **WaitTime 用 TimerHandle**：到达后可选等待，0 = 立即前往下一点。

## 新建文件

| 文件 | 说明 |
|---|---|
| `Public/Actors/PatrolPoint.h` | 路点 Actor 声明 |
| `Private/Actors/PatrolPoint.cpp` | 橙色 Arrow 可视化 |

## 修改文件

| 文件 | 变更 |
|---|---|
| `TheManTest.Build.cs` | 追加 AIModule / NavigationSystem |
| `Public/Characters/Enemy/Humanoid/HumanoidEnemy.h` | 构造函数、PossessedBy、PatrolPoints 数组、内部方法声明 |
| `Private/Characters/Enemy/Humanoid/HumanoidEnemy.cpp` | 完整巡逻实现 |

## 核心流程

```
PossessedBy(AAIController)
  └─ BindDelegate(OnRequestFinished → OnPatrolMoveCompleted)
  └─ MoveToNextPatrolPoint()
       └─ AIC->MoveToActor(PatrolPoints[CurrentIndex], 50cm)
            │ 到达
            ▼
       OnPatrolMoveCompleted()
         ├─ WaitTime == 0 → CurrentIndex = (Index+1) % Num → MoveToNextPatrolPoint()
         └─ WaitTime > 0  → SetTimer(WaitTime) → MoveToNextPatrolPoint()
```

## 编辑器使用说明

1. 在关卡中放若干 `APatrolPoint`（或其蓝图子类），调整位置和朝向
2. 选中敌人实例 → Details → Patrol → PatrolPoints 数组，逐一点选路点 Actor
3. 确保关卡有 NavMeshBoundsVolume 覆盖路线区域并已 Build 导航

## 实现日志

### 2026-06-11（Session19）
- Build.cs 新增 AIModule / NavigationSystem
- 新建 APatrolPoint（SphereComponent 标记位置，WaitTime 属性）
- AHumanoidEnemy 新增构造函数（AutoPossessAI + AIControllerClass）、PossessedBy（绑委托+0.1s 延迟启动）、MoveToNextPatrolPoint（MOVE_Walking 保险检查）、OnPatrolMoveCompleted、OnPatrolWaitFinished
- 修复：MoveToLocation 第5参数 bProjectDestinationToNavigation 改为 true
- 修复：PossessedBy 时 MoveMode=0（物理未就绪），改为 0.1f Timer 延迟启动
- 新增 PatrolScan 功能：MinScanWaitTime 阈值（默认 2s），bIsPatrolScanning 驱动 AnimBP PatrolScan 状态
- UHumanoidEnemyAnimInstance 新增 bIsPatrolScanning，NativeUpdateAnimation 每帧轮询
- ABP_HumanoidEnemy PatrolSM 新增 PatrolScan 状态，Idle ↔ PatrolScan 过渡条件配置完成
- PatrolScan 随机动画：`PatrolScanAnimIndex`（int）+ `PatrolScanAnimCount`（EditDefaultsOnly，子 ABP 各自设置）+ NativeUpdateAnimation 上升沿检测随机；ABP 用 Blend Poses by Int
- Stopping 按速度选动画：`bStoppingFromRun`（bool）+ `RunSpeedThreshold` / `StoppingDetectThreshold`（EditDefaultsOnly）；NativeUpdateAnimation 检测减速起始帧；ABP 用 Blend Poses by Bool
- PIE 验证：巡逻循环走路正常，WaitTime ≥ MinScanWaitTime 时随机播放扫视动画，停步动画按走/跑速区分

### 2026-06-11（Session21）
- 动画方案从原地动画改为 root motion 动画
- 移除原地停步模拟参数（PatrolBrakingDeceleration / PatrolGroundFriction），BeginPlay 仅设 MaxWalkSpeed
- AnimInstance 新增 bIsStopping（完整停步窗口标志）、bStoppingWithLeftFoot（停步触发帧骨骼 Z 采样）、StoppingAnimIndex（0-3，编码 急停×脚步 四种组合）
- ABP Stopping 状态改为 Blend Poses by Int（StoppingAnimIndex），对应 Walk_RU/LU + Run_RU/LU 四个动画
- Root motion retarget 丢失问题：用户已自行解决
- Turn 动画扩展：从 3 个（L90/R90/TurnAround）扩展到 7 个（±45°/±90°/±135°/180°）
- TurnAnimIndex（0-6）编入 AnimInstance，ABP 用单一 Turn 状态 + Blend Poses by Int 选动画
- ABP Turn/Stopping 状态待配置（下次开机继续）

### 2026-06-12（Session22）
**ABP 修复与调试**
- Root Motion Mode 改为 `Root Motion from Everything`（ABP Class Defaults）→ 胶囊体随根骨骼移动，问题解决
- Walk → Stopping 过渡箭头加 Blend Duration=0.15s / Mode=Ease In Out，硬切感消除
- PatrolScan Sequence Player 勾选 Loop Animation，避免动画播完后冻结在末帧
- PatrolWalkSpeed 默认 150，需与走路动画 root motion Average Velocity 匹配；同步调整 StoppingDetectThreshold

**转身系统 C++ 重构（全部已编译）**
- 根本问题：`RequestTurn()` 从未被调用，`bIsTurning` 永远 false
- 新增 `TryTurnOrMove()` 私有方法：计算到下一路点的偏转角，超过 `TurnAngleThreshold`（默认 30°）则调 `RequestTurn`，否则直接 `MoveToNextPatrolPoint`
- 修正执行顺序：到达路点 → 等待/扫描 → `OnPatrolWaitFinished` 调 `TryTurnOrMove` → 转身 → 出发（原来是到达后立刻转身，与扫描并行，顺序错误）
- 新增 Tick 旋转：`bOrientRotationToMovement` 在 `RequestTurn` 时关闭，Tick 内用 `FMath::FixedTurn` 匀速旋转到 `TargetTurnYaw`，`OnTurnComplete` 时精确对齐并恢复 `bOrientRotationToMovement`
- 新增配置项：`TurnAngleThreshold`（30°）/ `TurnRotationSpeed`（270°/s），蓝图 Class Defaults 可调
- 移除 `bPendingMoveAfterTurn`（逻辑简化后不再需要）

**转身调试发现的 Bug（待修复）**
- 现象：进入 Turn 状态，无黄色旋转调试文字，红色 `OnTurnComplete` 立刻出现
- 根因：**AnimNotify_TurnComplete 挂在转身动画最开头**，Turn 状态刚进入就触发，`bPendingTurn` 被立刻重置为 false，Tick 来不及执行旋转
- 修复方法（编辑器操作）：打开每个转身动画资产，将 `AnimNotify_TurnComplete` 拖到动画时间轴**末尾倒数 2-3 帧**处

**遗留调试代码（验证后删除）**
- `HumanoidEnemy.cpp` Tick：`GEngine->AddOnScreenDebugMessage(42, ...)` 黄色旋转日志
- `HumanoidEnemy.cpp` OnTurnComplete：`GEngine->AddOnScreenDebugMessage(43, ...)` 红色完成日志

### 2026-06-12（Session23）

**Tick 禁用 Bug 修复（已编译）**
- 根因：`AEnemyBase` 构造函数 `PrimaryActorTick.bCanEverTick = false`，导致所有子类含 `AHumanoidEnemy` 的 Tick 全部失效
- 修复：`AHumanoidEnemy` 构造函数加 `PrimaryActorTick.bCanEverTick = true`，覆盖父类设置
- 诊断线索："没有黄色调试文字，只有红色" → Tick 从未执行 → 旋转逻辑死代码
- 用户确认：转身旋转正常

**PatrolScan A-Pose Bug 修复（已编译）**
- 根因：UE5 `Blend Poses by Int` 内部将 Index 0 用作混合来源（blend source），选中 Index 0 时从 Reference Pose（A-Pose）混入，首帧出现 A-Pose
- 修复：`HumanoidEnemyAnimInstance.h` 默认值改为 `PatrolScanAnimIndex = 1`；`.cpp` 随机范围改为 `FMath::RandRange(1, PatrolScanAnimCount)` 跳过 Index 0
- 用户确认：A-Pose 消失

**TurnWalkSpeed 属性（已编译，当前未使用）**
- `HumanoidEnemy.h` 新增 `TurnWalkSpeed = 50.f`（EditDefaultsOnly）
- `RequestTurn` 调 `MaxWalkSpeed = TurnWalkSpeed`（转身期间限速为 50，但敌人原地转身，实际无移动）

**转身时前进功能——尝试后撤回**
- 尝试：`RequestTurn` 加 `AIC->MoveToLocation(nextPoint)` + `OnPatrolMoveCompleted` 加 `if (bPendingTurn) return`
- 现象：转身动画完全不播放，敌人瞬间转向
- 根因分析：AnimBP Turn 状态进入条件可能要求 `Speed == 0`；`MoveToLocation` 使 Speed > 0，Turn 状态无法激活
- 已完全撤回：恢复为原地转身（无前进移动）
- 结论：若未来需要转身时前进，需在 AnimBP Walk 状态加 Turn 状态转换，或用 AnimNotify 控制移动启动时机
