# FEAT-033 测试枪（TestGun）装配 + 武器手臂骨架统一

**状态：** done
**创建：** 2026-06-15
**关闭：** 2026-06-17-session38
**最后更新：** 2026-06-17-session38

> 用户确认（session38）：拿枪、滚轮切换、开火动画全部正常。骨架问题用 Compatible Skeletons 解决（同源 ref pose）。期间顺带修 BUG-033-001（滚轮切枪 RoundToInt→0）+ GA_Shoot 开火反馈与子弹解耦。

---

## 目标

把 `ATestGun` 配置成一把可用的武器并挂到角色手上（先看得见、能拿稳；可选再让它能开火）。

---

## 现有资产状态（已确认）

| 资产 | 路径 | 状态 |
|---|---|---|
| `ATestGun` C++ 类 | `Public/Equipment/Firearms/TestGun.h` / `.cpp` | **纯空壳**，仅 `: public AFirearm` + 一行 include，无任何属性/方法 |
| `BP_TestGun` | `Content/Weapons/TestGun/Blueprint/BP_TestGun.uasset` | 已存在，内部字段填写情况未确认 |
| `ABP_TestGun` | `Content/Weapons/TestGun/Animation/Logic/ABP_TestGun.uasset` | 已存在（武器动画层） |

> **结论：C++ 一行都不用加。** ATestGun 从 AFirearm 继承了全部 Mesh/Socket/射击/GAS 字段，装配完全是蓝图配置 + 加入角色清单。

---

## 挂载机制（代码事实）

`AFPSCharacterBase::BeginPlay()`（`FPSCharacterBase.cpp:76-79`）：
```cpp
EquipmentManager->AttachTargetMesh = ArmsMesh;                    // 武器挂到手臂骨骼
EquipmentManager->InitializeEquipment(InitialEquipmentClasses);  // 遍历清单生成
```
`UEquipmentManagerComponent::InitializeEquipment`：遍历 `InitialEquipmentClasses` → SpawnActor → 自动装备索引 0 → `AttachToComponent(ArmsMesh, ..., EquipSocketName)`。

→ **"挂到角色身上" = BP_TestGun 配好 + 把它填进角色蓝图的 `InitialEquipmentClasses`（放索引 0 当默认武器）。**

## 装配检查清单（蓝图，编辑器内做）

**最少能看见枪：**
- [x] BP_TestGun：`SkeletalMesh` 填枪模型（session38 完成）
- [x] BP_TestGun：`EquipSocketName` = **`Grip_Point`**（ArmsMesh 手部插槽名，session38 完成）
- [x] 角色蓝图 `InitialEquipmentClasses` 同时含 BP_RepairGun + BP_TestGun（session38，切换 + 显示验证通过）

> session38 PIE 验证：滚轮在 RepairGun ↔ TestGun 之间正常切换，旧枪隐藏、新枪显示在手上。
> 关键配置：`HolsterSocketName` = None（切走即隐藏），`EquipSocketName` = `Grip_Point`。

**要能开火再补（Weapon|* 分类）：**
- [ ] `PrimaryFireAbilityClass` = GA_Shoot 蓝图
- [ ] `BulletClass` / `MuzzleSocketName`（枪骨骼加 Muzzle 插槽）
- [ ] `FireMontage` / `FireSound` / 后坐力参数

---

## ✅ 阻塞已解除（session38）：Compatible Skeletons 验证通过

**结论：两套 ref pose 确实相同，零 retarget。** 在 SCF_Rifle_02 的 `SK_Mannequin_Arms_Skeleton` → Asset Details → `Compatible Skeletons` 添加 SCFP 骨架后，`ABP_TestGun` 引用 TestGun 的 SCFP 骨架动画，预览**两手位置正常**（用户确认）。证实"两手靠近"是用户原 IK Retargeter 没配好，不是骨架不兼容。
- **`RTG_SK_Arms` 重定向器可删**（不再需要）。
- 后续若有别的 SCFP/同源骨架动画要用，同理走 Compatible Skeletons 即可。

---

## 阻塞历史（已解决，保留备查）

**现象：** 把手枪动画 retarget 到对方骨架后，**两只手明显靠得更近 + 上下高度错位**。

### 已查实的事实（session38，用 strings 读 uasset 导入表确认，非推测）

手臂全部来自 `Content/SCI_FI_WEAPON_PACK`，**每把武器子文件夹各带一份独立的 `SK_Mannequin_Arms_Skeleton`**（同名但是不同 USkeleton）。用户实际用的两套：

| 资产 | 骨架文件 | 备注 |
|---|---|---|
| 手枪 `SCFP` | `SK_Mannequin_Arms_Skeleton` 13856 字节 | 仅 `IK_SK_SCFP_Arms` |
| 步枪 `SCF_Rifle_02` | `SK_Mannequin_Arms_Skeleton` **24695 字节** | `RTG_SK_Arms`(重定向器)是**用户自己建的**（打算把包动画重定向到步枪骨架），不是包作者做的；24695 字节大概率是用户 retarget 元数据撑大的，**不代表 ref pose 不同** |

**骨骼结构比对（session38 strings）：两套骨架骨骼名/数量基本完全一致**——都是标准 UE4 Mannequin 手臂骨架（root→pelvis→spine_01/02/03→clavicle→upperarm→lowerarm→hand→各手指 + ik_*），54 根同名同层级。结合 RTG 是用户自建、SCF_Rifle_02 未被第三方重摆 pose → **两套极可能是同一套 Epic 标准手臂、ref pose 相同**，"两手靠近"很可能只是用户那个 IK Retargeter 没配好（retarget pose 未对齐），而非骨架真不兼容。

**关键骨架绑定（strings 实测）：**
- `Weapons/RepairGun/.../A_HandIdle` → 绑 **SCF_Rifle_02 骨架** ✅（现在能跑通的武器）
- `Weapons/TestGun/Animation/Logic/ABP_TestGun` TargetSkeleton → **SCF_Rifle_02 骨架** ✅
- `Weapons/TestGun/.../A_HandIdle`（及 TestGun 自带全套序列）→ 绑 **SCFP(手枪)骨架** ❌ 外来

→ **真正定因：项目标准骨架早就统一在 SCF_Rifle_02(步枪)**（RepairGun + ArmsMesh + ABP_TestGun 都在它上面）。出问题的是 **TestGun 那一包导入进来的动画序列被绑到了 SCFP 骨架**。`ABP_TestGun`（站 SCF_Rifle_02）去播 SCFP 骨架动画 → 两套不同 ref pose 强行混用 → 两手靠近+错位。

> 所以**不是**"要不要统一骨架"的选择题。骨架已统一，只差把 TestGun 的 SCFP 动画搬到 SCF_Rifle_02。

### 解决方案（按优先级，先试简单的）

**目标骨架 = SCF_Rifle_02（项目标准，RepairGun/ArmsMesh/ABP_TestGun 都在它上面），不变。** 要做的是让 TestGun 那批 SCFP 骨架动画落到 SCF_Rifle_02 上。

**方案 A（首选，先验证）：Compatible Skeletons —— 假设 ref pose 相同，零 retarget**
1. 打开 SCF_Rifle_02 的 `SK_Mannequin_Arms_Skeleton` → Asset Details → **Compatible Skeletons** 添加 SCFP 的 `SK_Mannequin_Arms_Skeleton`。
2. `ABP_TestGun` 直接引用 TestGun 的 SCFP 骨架动画（现在应可直接选）。
3. **决定性验证：** 拿 TestGun `A_HandIdle` 在 SCF_Rifle_02 骨架/ArmsMesh 上预览。两手间距/高度正常 → ref pose 相同，收工，`RTG_SK_Arms` 可删；仍两手靠近 → 走方案 B。

**方案 B（仅当方案 A 验证失败，证明 ref pose 真不同）：修好 IK Retargeter**
- 源 SCFP（`IK_SK_SCFP_Arms`）→ 目标 SCF_Rifle_02（`IK_SK_SCF_Rifle_02_Arms`），**重点检查 retarget pose 是否对齐**（用户原 `RTG_SK_Arms` 两手靠近，八成是这里没设对）。
- 批量重定向 TestGun 全部序列到 SCF_Rifle_02，输出回 TestGun 目录，ABP 引用换成重定向版。

**注意：** 换骨架/重定向后 Socket 会丢，若 TestGun Mesh 上有 Muzzle/grip 插槽需在目标骨架重建。

---

## Bug 日志

### BUG-033-001：滚轮切武器要滚好几下、且武器不更换（session38 已修）
**现象：** 滚轮切枪，常常切不动；偶尔滚猛了才切；切换后旧枪不隐藏、新枪不显示。
**根因（C++ 逻辑 bug，跨装备/输入系统）：**
- `AFPSCharacterBase::SwitchEquipment` 用 `FMath::RoundToInt(Dir)` 把滚轮浮点值取整。滚轮一格输出 <0.5 时 → `RoundToInt`=0，但外层守卫只判 `Dir != 0.f`（0.3≠0 成立）照样进 → 调 `SwitchEquipment(0)`。
- 管理器里 `Direction=0` → 新索引=旧索引 → 把当前枪卸下又装上（Equip 蒙太奇重播但没换枪）。猛滚一下值≥0.5 才真切 → "滚好几下才切"。
**修复：**
1. `FPSCharacterBase.cpp::SwitchEquipment`：改为只取符号 `(Dir>0)?1:-1`，任何非零滚动精确切一格，不再依赖滚轮数值大小。
2. `EquipmentManagerComponent.cpp::SwitchEquipment`：加 `NewIndex==OldIndex` 守卫，no-op，避免对同一把枪卸下再装。
**验证：** 调试打印确认 `Dir=-1 共2把 索引0->1`，索引正确切换；调试打印已删除。两处均函数体内改，Live Coding 热编译。
**范围说明：** 属装备/输入系统（非 FEAT-033 本体），因阻塞 TestGun 切换验证而顺带修复，记于此备查。

### 改动：开火蒙太奇/音效/后坐力与子弹解耦（session38）
**动机：** 原 `UGA_Shoot::ActivateAbility` 在 `!Firearm->GetBulletClass()` 时提前 `EndAbility` 返回，导致未配子弹的武器**连开火蒙太奇/音效/后坐力都不播**。空枪/占位/测试枪应能播放开火反馈。
**改动（`GA_Shoot.cpp`）：**
- 提前返回守卫去掉 `!GetBulletClass()`，只保留 `!Firearm`。
- 子弹生成/命中逻辑（hitscan + projectile 两支）整体包进 `if (Firearm->GetBulletClass())`。
- 蒙太奇/音效/后坐力移到该 if 之外，无条件执行。
- 蒙太奇播放补 `GetAnimInstance()` 判空，防 ArmsMesh 无 AnimInstance 时崩。
**影响范围：** 共享开火能力 GA_Shoot，所有 AFirearm（含 RepairGun）。BulletClass=空 → 只有视觉/音效反馈、无实弹；BulletClass 有值 → 行为同旧。
**编译：** 函数体内改，Live Coding 热编译。
