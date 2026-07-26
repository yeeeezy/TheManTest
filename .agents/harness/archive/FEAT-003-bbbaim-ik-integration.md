# [FEAT-003] BBBAimIK 插件集成

**创建日期：** 2026-06-06
**状态：** in_progress
**Archive 文件：** `archive/FEAT-003-bbbaim-ik-integration.md`

---

## 功能概述

集成项目内已有的 BBBAimIK 插件（`Plugins/BBBUEAimIk-main/`），实现角色上半身脊柱骨骼链跟随瞄准目标的 CCD AimIK 效果。

**最终架构：** AimIK 放在 `ABP_FirearmBase` 的 `WeaponAimOffset` 层内，由 `UFirearmAnimInstance` 驱动，主角色 `ABP_MainCharacter` / `TheManAnimInstanceBase` 完全不感知 AimIK。

---

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/TheManTest.Build.cs` — 新增 `BBBAimIK` 模块依赖
- `Source/TheManTest/Public/Equipment/Firearms/FirearmAnimInstance.h` — 四个 AimIK 变量 + NativeUpdateAnimation 声明 + SetAimSourceLocalTransform setter
- `Source/TheManTest/Private/Equipment/Firearms/FirearmAnimInstance.cpp` — 射线检测驱动逻辑

**涉及蓝图资产（编辑器操作）：**
- `ABP_MainCharacter` — 删除旧 AimIK 节点链，Slot "FullBody" 直连 Output Root（已完成）
- `ABP_FirearmBase` — WeaponAimOffset 层内建 AimIK 链（已完成）
- RepairGun 蓝图 — `MuzzleLocalTransform` 待配置

**完成标准：**
- [x] Build.cs 添加 `BBBAimIK` 模块依赖，C++ 编译无错误
- [x] `UFirearmAnimInstance` 持有四个变量并暴露给蓝图
- [x] `ABP_FirearmBase` WeaponAimOffset 层 AimIK 节点配置完成
- [x] 动画层链接验证通过（FEAT-004 调试打印确认）
- [ ] `MuzzleLocalTransform` 配置，AimIK 偏移修正
- [ ] PIE 测试：枪口精确跟随视线目标

---

## 插件说明（来自 README）

- **算法：** CCD（循环坐标下降），逐骨迭代旋转使枪口朝向目标
- **AimTarget：** 必须是 **Component Space** 坐标，不是世界坐标
- **AimSourceLocalTransform：** 装备时计算**一次**，之后不再更新；Scale 必须为 (1,1,1)，不能为零

### UFirearmAnimInstance 四个变量

| 变量 | 类型 | 更新时机 | 说明 |
|---|---|---|---|
| `AimSourceLocalTransform` | `FTransform` | 装备时一次 | 枪口相对 hand_r 骨骼的局部偏移；Identity 时以 hand_r 原点为瞄准源 |
| `AimTargetComponentSpace` | `FVector` | 每帧 NativeUpdateAnimation | 目标在 Mesh Component Space 的坐标（射线检测转换） |
| `bHasValidAimTarget` | `bool` | 每帧 | 始终 true（无命中时用射线终点） |
| `bIsAiming` | `bool` | 每帧 | 临时硬编码 true，后续接输入动作 |

---

## 实现日志

### 2026-06-06 — 功能规划 → C++ 初版 → 架构重构

- 最初将变量放在 `TheManAnimInstanceBase`，AimIK 节点在 `ABP_MainCharacter` 主链路
- 重构后：变量迁移到 `UFirearmAnimInstance`，AimIK 移入 `ABP_FirearmBase` WeaponAimOffset 层
- BoneChain：spine_01(0.2) → spine_02(0.4) → spine_03(0.6)，AimSourceBoneName：hand_r

### 2026-06-06 — NativeUpdateAnimation 驱动逻辑

- 从控制器视点发射射线（10000 单位，ECC_Visibility）
- `InverseTransformPosition` 将命中点/终点从世界坐标转为 Mesh Component Space
- `bHasValidAimTarget` 始终 true，`bIsAiming` 硬编码 true

### 2026-06-06 — AimSourceLocalTransform 调查

**问题现象：** AimIK 运行但脊柱旋转有偏差。

**根因：** `MuzzleLocalTransform` 未在武器蓝图里配置，`AimSourceLocalTransform` 传入 Identity。Identity 意味着插件把 `hand_r` 骨骼原点当作瞄准源，而非实际枪口，导致脊柱旋转让手骨对准目标而不是枪管对准目标。

**注意：** Identity Transform ≠ 全零。Scale 必须为 (1,1,1)；若 Scale = (0,0,0) 则变换矩阵不可逆，插件会产生 NaN。

**待解决：** 配置 `MuzzleLocalTransform`，方案待定（手动填值 vs 自动从 Muzzle socket 计算）。

---

## Bug 记录

（无）

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（含 BBBAimIK 模块） | 2026-06-06 | 通过 | |
| ABP_FirearmBase WeaponAimOffset 层编译 | 2026-06-06 | 通过 | |
| 动画层链接（FEAT-004 调试打印） | 2026-06-06 | 通过 | 绿色消息确认 FirearmAnimInstance 获取成功 |
| PIE 测试——枪口精确跟随视线 | — | 待验证 | 依赖 MuzzleLocalTransform 配置 |

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
