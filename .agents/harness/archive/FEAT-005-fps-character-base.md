# [FEAT-005] FPS 角色基类重构

**创建日期：** 2026-06-07
**状态：** done
**Archive 文件：** `archive/FEAT-005-fps-character-base.md`

---

## 功能概述

新建 `AFPSCharacterBase` 平行基类，采用纯第一人称架构（SpringArm→Camera→ArmsMesh），以及三个具体角色的 FPS 版本。原有 `ATheManCharacterBase` 及其三个子类暂时弃用，代码保留不删。

---

## 背景决策

- **原方案（双骨骼）搁置**：`ATheManCharacterBase` 同时维护全身骨骼（投影用）和手臂骨骼（渲染用），相机挂在手臂骨骼的 `head` 插槽上，复杂度高，暂不开发。
- **新方案（纯 FPS）**：只用手臂骨骼，相机通过 SpringArm 驱动，手臂 Mesh 跟随相机俯仰，骨骼资产在蓝图中配置。

---

## 架构说明

```
CapsuleComponent（根，继承自 ACharacter）
  └── CameraArm（USpringArmComponent）
        TargetArmLength = 0
        bUsePawnControlRotation = true
        bInheritPitch/Yaw = true, Roll = false
        bDoCollisionTest = false
        Z 偏移 = BaseEyeHeight（默认 64）
        └── HeadCamera（UCameraComponent）
              bUsePawnControlRotation = false
              └── ArmsMesh（USkeletalMeshComponent）
                    SetOnlyOwnerSee = true
                    CastShadow = false
EquipmentManager（组件，装备系统复用）
GetMesh()（隐藏 + 无碰撞，不使用）
```

---

## 涉及文件

**新建 C++ 文件：**

| 文件 | 说明 |
|---|---|
| `Public/Characters/FPSCharacterBase/FPSCharacterBase.h` | 新基类声明 |
| `Private/Characters/FPSCharacterBase/FPSCharacterBase.cpp` | 新基类实现 |
| `Public/Characters/Infiltrator/FPSInfiltrator.h/.cpp` | 潜行者 FPS 版（空壳） |
| `Public/Characters/MaintenanceWorker/FPSMaintenanceWorker.h/.cpp` | 维修工 FPS 版（空壳） |
| `Public/Characters/TheExecutive/FPSTheExecutive.h/.cpp` | 高管 FPS 版（空壳） |

**未动文件（弃用但保留）：**
- `TheManCharacterBase.h/.cpp`、`Infiltrator.h/.cpp`、`MaintenanceWorker.h/.cpp`、`TheExecutive.h/.cpp`

---

## 完成标准

- [ ] C++ 编译无错误无警告（Development Editor / Win64）
- [ ] 在编辑器中基于 `AFPSMaintenanceWorker` 创建蓝图，配置 `ArmsMesh` 骨骼
- [ ] PIE 测试：相机随 SpringArm 正确俯仰，手臂 Mesh 跟随相机

---

## 已知遗留问题（不在本 FEAT 范围内）

| 问题 | 影响 | 处理计划 |
|---|---|---|
| `PlayerController` Cast 仍指向 `ATheManCharacterBase` | FPS 角色无输入响应 | 后续独立 FEAT |
| `EquipmentBase::Equip()` 取 `GetMesh()` 的 AnimInstance | FPS 角色装备层链接失败 | 后续独立 FEAT |
| `FCharacterType::CharacterClass` 类型锁死旧基类 | DT_CharacterRoster 无法填 FPS 角色 | 后续独立 FEAT |

---

## 实现日志

### 2026-06-07 — C++ 文件创建完成

- 新建 `AFPSCharacterBase`，组件挂载逻辑完成，GAS/装备/输入逻辑从 `ATheManCharacterBase` 移植
- 新建三个 FPS 具体角色类（空壳）
- 文档更新：CLAUDE.md、AGENTS.md、progress.md

---

## 验证证据

| 检查项 | 日期 | 结果 |
|---|---|---|
| C++ 编译 | 2026-06-07 | 通过 |
| 蓝图创建 + ArmsMesh 配置 | 2026-06-07 | 完成 |
| PIE 测试 | 2026-06-07 | 通过（用户确认） |

---

**完成标准全部满足日期：** 2026-06-07
**功能关闭日期：** 2026-06-07
