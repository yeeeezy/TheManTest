# [FEAT-006] FPS 动画架构

**创建日期：** 2026-06-07
**状态：** in_progress
**Archive 文件：** `archive/FEAT-006-fps-anim-architecture.md`

---

## 功能概述

为纯第一人称手臂骨骼（ArmsMesh）建立完整动画架构。核心思路：

- **Base AnimBP**（`ABP_FPSArms`）：处理所有武器共用的逻辑（移动速度、跳跃、落地），由 C++ 每帧驱动变量
- **Linked Anim Layer**：每种武器有独立的 AnimBP 实现 `ALI_WeaponAnim` 接口，装备时注入、卸下时移除
- **不需要上下半身混合**：手臂骨骼即全部，无需分层

---

## 架构图

```
ArmsMesh
  └── EquipmentAnimClass（每把武器的基础 ABP，装备时整体替换）
        ├── 该武器专属的状态机 / 姿态 / Locomotion
        ├── 变量：Speed / bIsFalling / AimPitch（UFPSArmsAnimInstance 子类驱动）
        └── WeaponLayer 插槽（ALI_WeaponAnim 接口）
              └── EquipmentAnimLayerClass（可选的武器专属 Linked Layer，叠加 AimIK 等）
```

**装备时顺序（EquipmentBase::Equip）：**
1. `AnimMesh->SetAnimInstanceClass(EquipmentAnimClass)` — 整体替换基础 ABP，旧实例销毁
2. `AnimInst->LinkAnimClassLayers(EquipmentAnimLayerClass)` — 叠加武器专属层（可选）
3. `AnimInst->Montage_Play(EquipMontage)` — 播放拔枪蒙太奇（可选）

**切枪时：** 新武器的 `SetAnimInstanceClass` 自动销毁旧实例及其所有链接层，无需手动 Unlink。`Unequip()` 里的 `UnlinkAnimClassLayers` 调用保留但已冗余。

---

## 范围

### C++ 新建
- `Source/TheManTest/Public/Characters/FPSCharacterBase/Animation/FPSArmsAnimInstance.h`
- `Source/TheManTest/Private/Characters/FPSCharacterBase/Animation/FPSArmsAnimInstance.cpp`
  - 继承 `UAnimInstance`
  - 缓存 `AFPSCharacterBase*`
  - 每帧计算：`Speed` / `bIsFalling` / `AimPitch`

### C++ 修改
- `Source/TheManTest/Private/Equipment/EquipmentBase/EquipmentBase.cpp`
  - `Equip()`：`CharacterOwner->GetMesh()` → 从 `AFPSCharacterBase` 取 `ArmsMesh`
  - `Unequip()`：同上
- `Source/TheManTest/Private/Equipment/Firearms/Firearm.cpp`
  - `Equip()`：同上，取 `ArmsMesh` 的 AnimInstance 后链接层、写入 AimSourceLocalTransform

### 蓝图操作（编辑器）
- 新建 `ABP_FPSArms`，父类 `UFPSArmsAnimInstance`，骨骼 = ArmsMesh 使用的骨骼
- 配置 `ALI_WeaponAnim` 武器层插槽
- 为 RepairGun 新建或调整对应的 Linked Layer AnimBP
- 各武器蓝图 `EquipmentAnimLayerClass` 指向对应 Linked Layer

---

## 关键决策

| 决策 | 原因 |
|---|---|
| 不用上下半身混合 | 纯手臂骨骼，无下半身，无需分层 |
| 用 Linked Anim Layer 而非整体换 AnimBP | 切换无缝、无状态机重置、新武器只需新建层 |
| C++ 驱动基础变量 | 保持蓝图干净，变量来源明确 |
| EquipmentBase 改从 ArmsMesh 取 AnimInstance | AFPSCharacterBase 的 GetMesh() 已隐藏无 AnimInstance |

---

## 待确认

- [ ] `ALI_WeaponAnim` 绑定的骨骼与 ArmsMesh 骨骼是否一致（需在编辑器确认）
- [ ] `ABP_FirearmBase`（FEAT-003 产物）是否复用或重建

---

## 完成标准

- [ ] `UFPSArmsAnimInstance` C++ 编译无错误
- [ ] `ABP_FPSArms` 编辑器创建并编译通过
- [ ] `AFirearm::Equip()` 从 ArmsMesh 正确取到 AnimInstance（调试打印验证）
- [ ] PIE：移动/跳跃动画正常，装备武器后武器层覆盖，卸下后恢复
- [ ] `feature_list.json` 和本 archive 更新为 done

---

## 实现日志

### 2026-06-07 — 功能规划

- 确认架构：Base AnimBP + Linked Layer，不做上下半身混合
- 前置问题：ALI_WeaponAnim 骨骼兼容性待确认

### 2026-06-07 — C++ 实现

- 新建 `UFPSArmsAnimInstance`（h + cpp），编译通过
  - 缓存 `AFPSCharacterBase*` 和 `UCharacterMovementComponent*`
  - 每帧计算：`Speed` / `Velocity_Z` / `bIsFalling` / `Direction` / `AimPitch`
  - `AimPitch` 归一化到 [-1, 1]，处理 UE 控制器旋转 >180° wraparound
- 未复用 `UTheManAnimInstanceBase`：旧类缓存 `ATheManCharacterBase*`，FPS 基类不同

### 2026-06-07 — 蓝图操作（用户完成）

- 编辑器中新建 `ABP_FPS_Arm_MainCharacter`，父类 `UFPSArmsAnimInstance`
- 用户已替换角色蓝图的 ArmsMesh 动画蓝图为 `ABP_FPS_Arm_MainCharacter`
- Locomotion StateMachine：5 个状态（Idle / Run/Walk / Jump_Start / Jump_Loop / Jump_End），使用 `SCI_FI_WEAPON_PACK/SCF_Rifle_02` 动画资产
- 动画蓝图 ALI_WeaponAnim 接口已添加到 Class Settings

### 2026-06-07 — AnimGraph 最终结构确定

完整链路（含蒙太奇 Slot）：
```
UseCachedPose(Cache_Locomotion)
    → DefaultSlot
    → WeaponAimOffset (LinkedAnimLayer)
    → UpperBodySlot
    → WeaponUpperBody (LinkedAnimLayer)
    → FullBodySlot
    → Output Pose
```

Slot 名称已在 ArmsMesh 骨骼的 Anim Slot Manager 中注册：`DefaultSlot` / `UpperBodySlot` / `FullBodySlot`

**武器扩展策略决策：**
- 普通武器：共享基础 Locomotion，只实现 Layer 里的持枪姿态和瞄准 IK
- 特殊武器（需专属跑跳）：在 `WeaponAimOffset` 层内自建 StateMachine，忽略输入 Pose
- 原则：共享基础 + 按需覆盖

### 2026-06-07 — 未完成

- `AFirearm::Equip()` 仍从 `GetMesh()` 取 AnimInstance，未改为 ArmsMesh（下一步）

### 2026-06-07 — Session5 架构升级

- `EquipmentBase.h`：新增 `EquipmentAnimClass`（`TSubclassOf<UAnimInstance>`，`Equipment|Animation`）
  - 装备时整体替换 ArmsMesh 的基础 ABP，每把武器携带自己的完整动画蓝图
- `EquipmentBase.cpp`：`Equip()` 重写
  - 先 `SetAnimInstanceClass(EquipmentAnimClass)` 替换基础 ABP
  - 再 `LinkAnimClassLayers(EquipmentAnimLayerClass)` 叠加武器层
  - 再 `Montage_Play(EquipMontage)` 播放拔枪动画
  - 三步共用同一个 AnimMesh 局部变量，逻辑收拢
- `EquipmentBase.h` Getter 补充：`GetEquipMontage()`

---

## Bug 记录

### BUG-006-001：ArmsMesh 俯仰轴心错误（已修复）

**现象：** Tick() 中 `ArmsMesh->SetRelativeRotation(FRotator(Pitch, 0, 0))` 绕 root 骨骼（地面附近）旋转，导致手臂 Mesh 整体旋转插入地下。

**修复方案：** 将 `bUseControllerRotationPitch = true`，让胶囊体直接跟控制器 pitch 旋转，ArmsMesh 挂在胶囊上自动跟随，删除 Tick() 中的手动旋转代码。`HeadCamera->bUsePawnControlRotation = true` 保证相机旋转精确匹配控制器，PlayerCameraManager 的 ViewPitchMin/Max 限制仍然生效。

**状态：** 已修复，PIE 验证通过（2026-06-07）。

---

## 验证证据

| 检查项 | 日期 | 结果 |
|---|---|---|
| UFPSArmsAnimInstance 编译 | 2026-06-07 | 通过 |
| ABP_FPSArms 编辑器编译 | 2026-06-07 | 通过（用户确认） |
| BUG-006-001 俯仰轴心修复 | 2026-06-07 | 通过 |
| ArmsMesh AnimInstance 链接 | — | 待验证 |
| PIE 测试（完整） | — | 待验证（武器层链接后） |

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
