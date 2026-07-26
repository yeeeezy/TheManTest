# [FEAT-009] GAS 射击系统

**创建日期：** 2026-06-07
**状态：** needs_improvement
**Archive 文件：** `archive/FEAT-009-gas-shooting.md`

---

## 功能概述

通过 GAS Gameplay Event 机制实现枪械开火，满足以下设计目标：

- **输入解耦**：LMB/RMB 只发送通用 Tag，PlayerController 无需知道具体技能
- **技能复用**：`UGA_Shoot` 一个类服务所有枪械，运行时从当前 `AFirearm` 读取射击配置
- **配置集中**：所有武器参数（能力类、射程、伤害 GE 等）挂在 `AFirearm` C++ 上，蓝图 Defaults 配置

---

## 架构图

```
LMB 按下
  └── ATheManPlayerController::HandlePrimaryFire()
        └── AFPSCharacterBase::PrimaryFire()
              └── ASC->HandleGameplayEvent(Input.Weapon.PrimaryFire)
                    └── UGA_Shoot::ActivateAbility()（已授予，监听该 Tag）
                          ├── 从 EquipmentManager 取当前 AFirearm
                          ├── LineTraceSingleByChannel（从 HeadCamera 朝向）
                          └── ApplyGameplayEffectSpecToTarget（若命中有 ASC 的目标）
```

**技能授予时序：**
```
BeginPlay → InitializeEquipment → Equip()    [ASC 未就绪，跳过 GrantAbilities]
PossessedBy → ASC 初始化 → Firearm->GrantAbilities(ASC)   [补授]

运行时切枪：
Unequip() → RevokeAbilities(ASC) → Equip() → GrantAbilities(ASC)
```

---

## 新增文件

| 文件 | 内容 |
|---|---|
| `Public/GAS/TheManGameplayTags.h` | `TAG_Input_Weapon_PrimaryFire` / `TAG_Input_Weapon_SecondaryFire` 声明 |
| `Private/GAS/TheManGameplayTags.cpp` | Tag 定义（字符串 `"Input.Weapon.PrimaryFire"` 等） |
| `Public/GAS/Abilities/GA_Shoot.h` | 射击技能头文件 |
| `Private/GAS/Abilities/GA_Shoot.cpp` | CDO 设置 AbilityTriggers；ActivateAbility Hitscan + 伤害 GE |

---

## 修改文件

| 文件 | 变更 |
|---|---|
| `Firearm.h` | 新增：bIsHitscan/HitscanRange/DamageEffectClass/BaseDamage/FireRate（射击参数）；PrimaryFireAbilityClass/SecondaryFireAbilityClass（技能类）；GrantAbilities/RevokeAbilities 方法；Unequip() 覆写 |
| `Firearm.cpp` | Equip()：修复 ArmsMesh AnimLayer + GrantAbilities；Unequip()：RevokeAbilities + 解链 AnimLayer |
| `EquipmentBase.cpp` | 新增 GetAnimLayerMesh() 静态辅助函数，Equip/Unequip 改用 ArmsMesh（FEAT-006 修正） |
| `FPSCharacterBase.h` | 新增 GetHeadCamera/GetArmsMesh/GetEquipmentManager Getter；PrimaryFire/SecondaryFire 方法 |
| `FPSCharacterBase.cpp` | 新增 PrimaryFire/SecondaryFire（发送 GameplayEvent）；PossessedBy 末尾补授武器技能 |
| `TheManPlayerController.h` | 新增 PrimaryFireAction/SecondaryFireAction UPROPERTY；HandlePrimaryFire/HandleSecondaryFire |
| `TheManPlayerController.cpp` | 绑定两个 Action；HandlePrimaryFire/SecondaryFire 路由到 Character |

---

## 蓝图待办（编辑器操作）

| 步骤 | 说明 |
|---|---|
| 1 | 在 Content/Inputs/Actions/ 新建 `IA_PrimaryFire`（鼠标左键，Digital bool）和 `IA_SecondaryFire`（鼠标右键） |
| 2 | 在 `IMC_Default` 中为两个 IA 添加 Mapping |
| 3 | 在 `BP_TheManPlayerController` Details 面板赋值 `PrimaryFireAction` / `SecondaryFireAction` |
| 4 | 在 Content/GAS/Abilities/ 新建蓝图，父类 `UGA_Shoot`（无需任何蓝图逻辑，CDO 已设好触发 Tag） |
| 5 | 在 `BP_RepairGun`（或其他武器蓝图）的 `PrimaryFireAbilityClass` 指定上一步创建的 BP |

---

## 关键决策

| 决策 | 原因 |
|---|---|
| PlayerController 只路由，Character 发 Tag | 解耦；PlayerController 不知道 GAS，Character 才是 GAS 的 Avatar |
| GA_Shoot CDO 设置 AbilityTriggers | 蓝图无需任何配置，创建子类即可用 |
| 技能在 PossessedBy 后补授 | BeginPlay 先于 PossessedBy，ASC 在 PossessedBy 才就绪 |
| EquipmentBase 加 GetAnimLayerMesh() | 避免 Equip/Unequip 把 anim layer 链接到不可见的 GetMesh() 上 |
| Hitscan（非抛射体）优先实现 | 无需额外 Actor 资产，快速验证完整 GAS 链路 |

---

## 完成标准

- [ ] C++ 编译无错误无警告
- [ ] 蓝图创建：IA_PrimaryFire / GA_Shoot BP / 武器蓝图配置
- [ ] PIE：LMB 有红色调试射线输出
- [ ] PIE：命中有 ASC 的目标时伤害 GE 触发
- [ ] PIE：切枪后旧技能回收、新技能生效
- [ ] feature_list.json 更新为 done

---

## 实现日志

### 2026-06-07 — C++ 全部完成

- 新建 GAS Tags、GA_Shoot、Firearm 配置变量
- 修复 EquipmentBase/Firearm Equip 使用 ArmsMesh
- FPSCharacterBase 加 Getter + PrimaryFire/SecondaryFire
- PlayerController 加 LMB/RMB 绑定

### 2026-06-07 — PIE 验证通过，标记待改进

- 子弹从枪口生成（projectile 模式，bIsHitscan=false）
- OnComponentHit 驱动膨胀，BulletBase 全程 WorldDynamic 通道
- 膨胀完成后可被新子弹命中并二次膨胀
- 碰撞方案：QueryAndPhysics + SetNotifyRigidBodyCollision(true)
- 球面滑动问题：CharacterMovementComponent 在球形碰撞表面正常偏转，已接受
- 待完成蓝图：IA_PrimaryFire、GA_Shoot BP、RepairGun 武器配置、Muzzle Socket

---

## 验证证据

| 检查项 | 日期 | 结果 |
|---|---|---|
| C++ 编译 | — | 待验证 |
| PIE 射线调试 | — | 待验证 |

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
