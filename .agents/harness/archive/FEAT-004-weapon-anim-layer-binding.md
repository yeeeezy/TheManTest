# [FEAT-004] 武器动画层装配

**创建日期：** 2026-06-06
**状态：** in_progress
**Archive 文件：** `archive/FEAT-004-weapon-anim-layer-binding.md`

---

## 功能概述

装备/切换武器时自动链接/解链武器专属 Linked Anim Layer（`ABP_FirearmBase`），并在装备时将 `MuzzleLocalTransform` 写入 `UFirearmAnimInstance.AimSourceLocalTransform`，使 AimIK 知道枪口的精确位置。

---

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/Public/Equipment/Firearms/FirearmAnimInstance.h` — 新增 `SetAimSourceLocalTransform()` 公开 setter
- `Source/TheManTest/Public/Equipment/Firearms/Firearm.h` — 新增 `MuzzleLocalTransform` 字段 + `Equip()` 重写声明
- `Source/TheManTest/Private/Equipment/Firearms/Firearm.cpp` — 实现 `Equip()`：Super 链接层，再写入 FirearmAnimInstance；含调试打印
- `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h/.cpp` — 新增 `EndPlay` 销毁 Inventory，修复切换角色时装备 Actor 泄漏

**涉及蓝图（编辑器操作）：**
- RepairGun 蓝图 → `EquipmentAnimLayerClass` 已设为 `ABP_FirearmBase` ✓
- RepairGun 蓝图 → `MuzzleLocalTransform` 待配置

---

## 完成标准

- [x] C++ 编译无错误无警告
- [x] `AFirearm::Equip()` 正确取到 `UFirearmAnimInstance` 并写入 transform（调试打印验证）
- [x] 武器蓝图 `EquipmentAnimLayerClass` 设为 `ABP_FirearmBase`
- [ ] 武器蓝图 `MuzzleLocalTransform` 填入枪口偏移值
- [ ] PIE 测试：装备武器后 AimIK 偏移修正，枪口精确跟随视线

---

## 实现日志

### 2026-06-06 — 调查与 C++ 实现

**调查发现：** `EquipmentBase::Equip()` 已实现 `LinkAnimClassLayers` / `UnlinkAnimClassLayers`，动画层切换骨架已就位。

**调用链：**
```
AFirearm::Equip(NewOwner)
  └→ Super::Equip()  [LinkAnimClassLayers]
  └→ GetMesh()->GetAnimInstance()
       └→ GetLinkedAnimLayerInstanceByClass(EquipmentAnimLayerClass)
            └→ Cast<UFirearmAnimInstance>
                 └→ SetAimSourceLocalTransform(MuzzleLocalTransform)
                 └→ 调试打印（绿/红）
```

**同步修复：** `EquipmentManagerComponent::EndPlay` 销毁 Inventory，修复角色切换时装备 Actor 不自动销毁的泄漏问题。

### 2026-06-06 — PIE 验证

- 调试打印显示**绿色**：FirearmAnimInstance 成功获取，层链接正常
- `MuzzleLocalTransform` 未配置，`AimSourceLocalTransform` = Identity，AimIK 有偏差（见 FEAT-003 调查记录）

### 待做

`MuzzleLocalTransform` 配置方案待定：
- **方案 A**：手动在蓝图里填枪口相对 hand_r 的偏移值（快速测试）
- **方案 B**：C++ 里从枪骨骼网格的 Muzzle socket 自动计算（需先在骨骼编辑器加 socket）

---

## Bug 记录

**BUG-001（已修复）：** 切换角色时旧角色的装备 Actor 不销毁，泄漏在场景里。
- **根因：** `SpawnActor` 生成的装备只是 Attach 到角色，不是子组件，角色 `Destroy()` 时不会自动销毁装备
- **修复：** `EquipmentManagerComponent::EndPlay` 遍历 Inventory 逐一 `Destroy()`

---

## 验证证据

| 检查项 | 日期 | 结果 |
|---|---|---|
| C++ 编译 | 2026-06-06 | 通过 |
| RepairGun EquipmentAnimLayerClass 配置 | 2026-06-06 | 完成 |
| PIE 调试打印——FirearmAnimInstance 获取 | 2026-06-06 | 通过（绿色） |
| MuzzleLocalTransform 配置 | — | 待做 |
| PIE 测试——AimIK 偏移修正 | — | 待验证 |

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
