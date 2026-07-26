# [FEAT-010] 子弹类 + 枪口射线

**创建日期：** 2026-06-07
**状态：** in_progress
**Archive 文件：** `archive/FEAT-010-bullet-class-muzzle-trace.md`

---

## 功能概述

- 建立子弹类继承体系（`ABulletBase` → `ARepairGunBullet`），为后续弹道差异化做基础
- `AFirearm` 增加 `BulletClass` 和 `MuzzleSocketName` 配置变量
- `UGA_Shoot` 的 Hitscan 射线起点从相机改为武器骨骼上的 Muzzle Socket，方向保持跟相机朝向（准星）

---

## 射线策略

```
射线起点 = WeaponMesh->GetSocketLocation("Muzzle")   ← 视觉上从枪口出发
射线方向 = HeadCamera->GetForwardVector()             ← 命中点跟准星一致
射线终点 = 起点 + 方向 × HitscanRange

fallback：如果 Socket 不存在，起点退回相机位置（不崩溃）
```

---

## 新增文件

| 文件 | 内容 |
|---|---|
| `Public/Equipment/Firearms/Bullets/BulletBase.h/.cpp` | 空壳基类，`UCLASS(Abstract)` |
| `Public/Equipment/Firearms/Bullets/RepairGunBullet.h/.cpp` | RepairGun 专属子弹，继承 ABulletBase，空壳 |

## 修改文件

| 文件 | 变更 |
|---|---|
| `Firearm.h` | 新增 `MuzzleSocketName`（默认 "Muzzle"）、`TSubclassOf<ABulletBase> BulletClass`、对应 Getter |
| `GA_Shoot.cpp` | 射线起点改为 Muzzle Socket，Socket 不存在时 fallback 到相机位置 |

---

## 蓝图待办

| 步骤 | 说明 |
|---|---|
| 1 | 在武器骨骼（RepairGun SkeletalMesh）编辑器中添加名为 `Muzzle` 的 Socket，位置对准枪口 |
| 2 | 在 `BP_RepairGun` Details 的 `Weapon|Shooting` 分类下，`Bullet Class` 选 `ARepairGunBullet`（或其蓝图子类） |
| 3 | `Muzzle Socket Name` 默认已填 `Muzzle`，与 Socket 名对应即可 |

---

## 完成标准

- [ ] C++ 编译无错误无警告
- [ ] 武器骨骼添加 Muzzle Socket
- [ ] PIE 测试：调试射线从枪口位置出发，命中点与准星一致
- [ ] `feature_list.json` 更新为 done

---

## 实现日志

### 2026-06-07 — C++ 完成

- 新建 `ABulletBase`（Abstract）和 `ARepairGunBullet`（空壳）
- `Firearm.h` 加 `MuzzleSocketName` / `BulletClass` 及 Getter
- `GA_Shoot.cpp` 射线起点改为枪口 Socket，含 fallback

**下一步：** 编译 → 武器骨骼加 Muzzle Socket → PIE 验证

---

## 验证证据

| 检查项 | 日期 | 结果 |
|---|---|---|
| C++ 编译 | — | 待验证 |
| PIE 射线从枪口出发 | — | 待验证 |

**完成标准全部满足日期：** —
**功能关闭日期：** —
