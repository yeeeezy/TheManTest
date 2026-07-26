# [FEAT-011] 开火动画蒙太奇

**创建日期：** 2026-06-07
**状态：** planned
**Archive 文件：** `archive/FEAT-011-fire-montage.md`

---

## 功能概述

按下 LMB 开火时，在 ArmsMesh 上播放武器专属的开火蒙太奇，提供开火手感反馈。

---

## 架构设计

```
LMB → GA_Shoot::ActivateAbility()
  → 执行发射逻辑（projectile 生成）
  → 取 AFPSCharacterBase::GetArmsMesh()
  → ArmsMesh->GetAnimInstance()->Montage_Play(Firearm->FireMontage)
```

- `FireMontage` 配置在 `AFirearm`（`EditDefaultsOnly`），每种武器独立配置
- 由 `GA_Shoot` 播放，不在 Character 或 Controller 里处理
- 蒙太奇在 ABP_FPSArms 的 DefaultSlot 插槽播放

---

## 需修改文件

| 文件 | 变更 |
|---|---|
| `Firearm.h` | 新增 `UAnimMontage* FireMontage`（EditDefaultsOnly, Category="Weapon|Animation"） |
| `GA_Shoot.cpp` | ActivateAbility 末尾取 ArmsMesh AnimInstance 播放蒙太奇 |

---

## 完成标准

- [ ] AFirearm 新增 FireMontage UPROPERTY
- [ ] GA_Shoot 播放蒙太奇
- [ ] BP_RepairGun FireMontage 指定资产
- [ ] PIE 测试：按 LMB 手臂播放开火动画

---

## 实现日志

（待填写）

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
