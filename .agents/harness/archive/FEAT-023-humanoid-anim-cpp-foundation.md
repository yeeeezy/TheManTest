---
feature_id: FEAT-023
name: 敌人动画 C++ 基础层
status: done
created: 2026-06-11
closed: 2026-06-11
---

## 目标

为人形怪动画系统建立完整的 C++ 驱动基础，供后续 FEAT-024～027 的蓝图状态机使用。

## 架构决策

- **枚举独立头文件**：`HumanoidEnemyTypes.h` 只含 `EHumanoidEnemyAIState`，避免 AnimInstance 和 Enemy 头文件互相包含。
- **AnimInstance 轮询模型**：`UHumanoidEnemyAnimInstance::NativeUpdateAnimation` 每帧从 `AHumanoidEnemy` 读状态，不用 Delegate，简单直接。
- **`IsDead()` 放 `AEnemyBase`**：`bIsDead` 字段已在基类，所有敌人类型通用，加 getter 合理。
- **AI 状态接口只在 `AHumanoidEnemy`**：`SetAIState / RequestTurn / OnTurnComplete` 是人形怪专属，不上移到基类。

## 新建文件

| 文件 | 说明 |
|---|---|
| `Public/Characters/Enemy/Humanoid/HumanoidEnemyTypes.h` | `EHumanoidEnemyAIState` 枚举 |
| `Public/Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.h` | AnimInstance 声明 |
| `Private/Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.cpp` | AnimInstance 实现 |

## 修改文件

| 文件 | 变更 |
|---|---|
| `Public/Characters/Enemy/EnemyBase.h` | 新增 `IsDead()` BlueprintPure getter |
| `Public/Characters/Enemy/Humanoid/HumanoidEnemy.h` | 新增枚举 include + AI 状态 + 转向接口 |
| `Private/Characters/Enemy/Humanoid/HumanoidEnemy.cpp` | 实现 SetAIState / RequestTurn / OnTurnComplete |

## 暴露给蓝图的变量（AnimInstance）

| 变量 | 类型 | 用途 |
|---|---|---|
| `AIState` | `EHumanoidEnemyAIState` | 驱动状态机大层跳转 |
| `bIsTurning` | `bool` | Patrol 层 Turn 状态进入条件 |
| `TurnAngle` | `float` | 选择 Turn_L90 / Turn_R90 / TurnAround |
| `bIsDead` | `bool` | 任意状态跳转 Dead |
| `Speed` | `float` | 继承自基类，驱动混合空间 Y 轴 |
| `Direction` | `float` | 继承自基类，驱动混合空间 X 轴 |

## 实现日志

### 2026-06-11
- 创建 `HumanoidEnemyTypes.h`，定义 5 状态枚举
- `EnemyBase.h` 新增 `IsDead()` getter（bIsDead 原为 private）
- `HumanoidEnemy.h/.cpp` 完整实现 AI 状态 + 转向接口
- `HumanoidEnemyAnimInstance.h/.cpp` 完整实现，继承 `UBaseLocomotionAnimInstance`
- 用户确认编译通过 + 编辑器验证 → 关闭
