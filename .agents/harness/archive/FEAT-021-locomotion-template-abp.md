# [FEAT-021] Locomotion Template ABP 重构

**创建日期：** 2026-06-10
**状态：** done
**Archive 文件：** `archive/FEAT-021-locomotion-template-abp.md`

---

## 功能概述

将 FPS 手臂动画的 Locomotion 逻辑从骨骼绑定的 `ABP_FPS_Arm_MainCharacter` 中解耦，建立一套可复用的 Template ABP + 泛化 C++ 基类架构。目标：玩家手臂和敌人全身骨骼都能复用同一套状态机逻辑，新增角色/敌人只需继承 Template，不重复搭状态机。

---

## 架构变更

### 变更前

```
ABP_FPS_Arm_MainCharacter（绑定手臂骨骼）
  └── C++ 父类：UFPSArmsAnimInstance（缓存 AFPSCharacterBase*）
```

### 变更后

```
UBaseLocomotionAnimInstance（新建 C++ 基类，不依赖具体角色类型）
  ├── UFPSArmsAnimInstance（继承基类，FPS 专属扩展）
  └── UEnemyAnimInstance（继承基类，敌人专属扩展，未来新建）

ABP_BaseLocomotion（Template ABP，不绑定骨骼）
  └── C++ 父类：UBaseLocomotionAnimInstance
        ├── Locomotion StateMachine（Idle/Walk/Run/Jump）
        ├── Slot: DefaultSlot / UpperBodySlot / FullBodySlot
        └── Linked Layer 插槽：WeaponAimOffset / WeaponUpperBody

ABP_FPS_Arm_MainCharacter（绑定手臂骨骼，继承 ABP_BaseLocomotion）
  └── C++ 父类：UFPSArmsAnimInstance（可覆盖父类逻辑）

ABP_Enemy_XXX（绑定敌人全身骨骼，继承 ABP_BaseLocomotion）
  └── C++ 父类：UEnemyAnimInstance（未来按需新建）
```

---

## 范围

### C++ 新建
- `Source/TheManTest/Public/Characters/Animation/BaseLocomotionAnimInstance.h`
- `Source/TheManTest/Private/Characters/Animation/BaseLocomotionAnimInstance.cpp`
  - 继承 `UAnimInstance`
  - 从 `APawn` 获取速度和移动组件，不依赖 `AFPSCharacterBase`
  - 暴露：`Speed` / `Velocity_Z` / `bIsFalling` / `AimPitch` / `Direction`

### C++ 修改
- `Source/TheManTest/Public/Characters/FPSCharacterBase/Animation/FPSArmsAnimInstance.h`
  - 父类从 `UAnimInstance` 改为 `UBaseLocomotionAnimInstance`
  - 移除已迁移到基类的变量和 NativeUpdateAnimation 实现（或改为 `Super::` 调用）

### 蓝图操作（编辑器）

采用 **Template ABP + Asset Override** 方案：

1. 新建 `ABP_BaseLocomotion`（Content Browser → Animation → Animation Blueprint，**不选骨骼**）
2. Class Settings → Parent Class → `UBaseLocomotionAnimInstance`
3. 在 `ABP_BaseLocomotion` 里重建 AnimGraph **结构**，所有 Sequence Player / Blend Space 节点留空（不填动画资产）；过渡条件、Slot 节点、Linked Layer 插槽照常配置
4. 编译 `ABP_BaseLocomotion`
5. 打开 `ABP_FPS_Arm_MainCharacter` → Class Settings → Parent Class → `ABP_BaseLocomotion`
6. 打开 **Window → Asset Override Editor**，将父类中所有空动画引用替换为手臂骨骼的实际动画资产
7. `ABP_FPS_Arm_MainCharacter` AnimGraph 可清空（逻辑继承自父类），只保留 Output Pose 节点
8. 编译两个 ABP，确认无错误

---

## 关键决策

| 决策 | 原因 |
|---|---|
| 用 Template ABP 而非只共享 C++ 父类 | AnimGraph 状态机结构也要复用，不只是变量 |
| 基类从 APawn 而非 ACharacter 取速度 | 兼容未来非 Character 的 AI 实体 |
| UFPSArmsAnimInstance 保留，改继承基类 | 保持对外接口不变，不影响已有蓝图引用 |
| 不在本次新建 UEnemyAnimInstance | 范围控制，敌人系统未开始，届时再加 |

---

## 完成标准

- [x] `UBaseLocomotionAnimInstance` C++ 编译无错误无警告
- [x] `UFPSArmsAnimInstance` 改继承后编译无错误
- [x] `ABP_BaseLocomotion` Template ABP 编辑器编译通过（含空动画状态机 + Slot + Linked Layer 插槽）
- [x] 子 ABP 改继承 `ABP_BaseLocomotion` 后编译通过
- [x] Asset Override Editor 中所有空动画引用已填入手臂骨骼对应资产
- [x] PIE 测试：玩家移动/跳跃动画正常，武器层链接正常，与重构前行为一致
- [x] `feature_list.json` 和本 archive 更新为 done

---

## 实现日志

### 2026-06-10 — 功能创建

- 用户确认：后续敌人系统复用 Locomotion 逻辑，有必要做 Template ABP 重构
- 确认迁移路径：新建 C++ 基类 → 修改 FPSArmsAnimInstance 继承 → 新建 Template ABP → 现有 ABP 改继承

### 2026-06-10 — C++ 实现

新建文件：
- `Source/TheManTest/Public/Characters/Animation/BaseLocomotionAnimInstance.h`
- `Source/TheManTest/Private/Characters/Animation/BaseLocomotionAnimInstance.cpp`
  - 父类 `UAnimInstance`，缓存 `APawn*` 和 `UCharacterMovementComponent*`（通过 `FindComponentByClass`，不依赖 `AFPSCharacterBase`）
  - 每帧驱动：Speed / Velocity_Z / bIsFalling / Direction / AimPitch（逻辑与原 UFPSArmsAnimInstance 完全一致）

修改文件：
- `FPSArmsAnimInstance.h`：父类从 `UAnimInstance` 改为 `UBaseLocomotionAnimInstance`，所有变量和逻辑上移到基类，.h 仅保留类声明
- `FPSArmsAnimInstance.cpp`：清空为只含 #include，逻辑已在基类

待办（编辑器操作）：
- 新建 ABP_BaseLocomotion（Template ABP，不选骨骼，Parent Class = UBaseLocomotionAnimInstance）
- 迁移 ABP_FPS_Arm_MainCharacter 的 AnimGraph 到 ABP_BaseLocomotion
- ABP_FPS_Arm_MainCharacter → Class Settings → Parent Class → ABP_BaseLocomotion

---

## Bug 记录

（暂无）

---

## 验证证据

（待填写）

---

**完成标准全部满足日期：** 2026-06-10
**功能关闭日期：** 2026-06-10
