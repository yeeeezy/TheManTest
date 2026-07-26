# 装备系统

**何时读取：** 新增装备类型、修改装备生命周期（Equip / Unequip 行为）、新增插槽或动画层时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Equipment/EquipmentBase/EquipmentBase.h` | `Equip()` / `Unequip()` / `PlayEquipMontage()`；StaticMesh / SkeletalMesh / RectLight 组件；插槽名；EquipMontage；EquipmentAnimLayerClass；EquipmentAnimClass |
| `Source/TheManTest/Private/Equipment/EquipmentBase/EquipmentBase.cpp` | 组件构造（默认 `EquipSocketName="Grip_Point"`；武器 mesh `CastShadow=true`/`bCastDynamicShadow=true`——FEAT-042 让地上影子手里有枪，配合 FEAT-038 ShadowBodyMesh）；FPS 角色的动画层目标包含独立 `ArmsViewMesh`，并按 session70 方案同时链接 `ArmsViewMesh` 与 `GetMesh()`；**Equip 只做 SetAnimInstanceClass → LinkAnimClassLayers，不播蒙太奇**；蒙太奇拆到 `PlayEquipMontage()` 由调用方决定时机（切角色初次装备推迟到姿势就绪的下一帧播，避免起始位置错乱/音效误触发） |
| `Source/TheManTest/Public/Equipment/WeaponBase/WeaponBase.h` | 武器基类（继承 EquipmentBase，当前为空壳） |
| `Source/TheManTest/Public/Equipment/Firearms/Firearm.h` | 射击参数：`bIsHitscan` / `HitscanRange` / `FireRate` / `BulletClass` / `MuzzleSocketName`；开火反馈：`FireMontage` / `FireSound` / 后坐力；技能：`PrimaryFireAbilityClass` / `SecondaryFireAbilityClass`；`GrantAbilities()` / `RevokeAbilities()`；**`GrantedASC`(TWeakObjectPtr 缓存，切角色回收技能用)** |
| `Source/TheManTest/Private/Equipment/Firearms/Firearm.cpp` | `Equip()`：链接 ArmsMesh AnimLayer + GrantAbilities(缓存 GrantedASC)；`Unequip()`：RevokeAbilities(ASC 为 null 时回退 GrantedASC) + 解链 AnimLayer |
| `Source/TheManTest/Public/Equipment/Firearms/Bullets/BulletBase.h` | CollisionSphere(QueryOnly) + BulletMesh + ProjectileMovement；`Damage`(SetByCaller 传入 HitEffectClass) / `HitEffectClass` / `bDestroyOnHit`；`InitBullet(发射者, SourceASC)`(忽略发射者防自撞) / `ProcessHit()` BlueprintNativeEvent |
| `Source/TheManTest/Public/Equipment/Firearms/Bullets/RepairGunBullet.h` | 指数膨胀（e^(Rate×t)）；膨胀到 MaxExpansionScale 后锁定，LifetimeAfterExpansion 秒后销毁 |

## 武器资产目录约定（FEAT-052）

- 多种武器复用的资源放在 `/Game/Weapons/_Shared/`，并按 `Mesh`、`Material`、`Textures`、`Animations`、`GAS` 等类型继续分层。
- 某把武器专属的资源放在 `/Game/Weapons/<WeaponName>/`，按 `Blueprint`、`Mesh`、`Material`、`Textures`、`Animation` 分层；不要把材质或贴图放进 `Mesh`。
- 通用弹体：`/Game/Weapons/_Shared/Mesh/SM_Shared_Bullet`，材质为 `/Game/Weapons/_Shared/Material/M_Shared_Bullet` 与 `M_Shared_Bullet_Accent`。
- RepairGun 科技泡沫弹体：`/Game/Weapons/RepairGun/Mesh/SM_RepairGun_Bullet`，专属材质为 `/Game/Weapons/RepairGun/Material/M_RepairGun_Bullet`。

## ⚠️ 武器动画两字段对 MM 角色（FEAT-042）的影响

`Equip()` 用武器 BP 上两个 `Equipment|Animation` 字段操作 Leader mesh(`GetMesh()`) 的 AnimInstance：

- **`EquipmentAnimClass`**（`SetAnimInstanceClass`，整体替换 ABP）：⚠️ **MM 角色上必须留 None**——否则会把 `SandboxCharacter_CMC_ABP`（MM 主 ABP）整个顶掉，MM 直接废。
- **`EquipmentAnimLayerClass`**（`LinkAnimClassLayers`，链接武器层）：必须是与宿主**同骨架（Manny）**且实现 `ALI_WeaponAnim` 的 ABP。旧 `ABP_FirearmBase`（旧手臂骨架）链不上 MM 宿主。FEAT-042 接装备阶段两字段都留 None（持枪 pose 由宿主 `WeaponAimOffset` 默认层兜底），等阶段 3-b 出 Manny 版武器层 ABP 再填 `EquipmentAnimLayerClass`。

## FEAT-042 上半身武器层（session70）

当前项目已回到原生 `AFPSCharacterBase + ABP_BodyLocomotion + ArmsViewMesh` 路线：

- `EquipmentAnimLayerClass` 保持原职责：由 `AEquipmentBase::Equip()` 链接到 `AFPSCharacterBase::GetArmsMesh()`，也就是独立 FP viewmodel `ArmsViewMesh`。
- session70 最终方案：`EquipmentAnimLayerClass` 会同时链接到 FPS 角色的 `ArmsViewMesh` 和 `GetMesh()`。两个 mesh 可以使用同一个主 ABP（如 `ABP_BodyLocomotion`），各自拥有独立 AnimInstance，并各自执行武器层/AimIK。
- `AFirearm::Equip()` 不再重复 Link 层，只在原链路完成后给 `ArmsViewMesh` 和 `GetMesh()` 上已链接的 `UFirearmAnimInstance` 都写入 `MuzzleLocalTransform`。
- `ShadowBodyMesh` / `LegsMesh` 仍跟随 `GetMesh()`，因此身体武器层和 AimIK 会自然进入影子/腿共享姿势。
- 不使用 Copy Pose From Mesh；武器切换仍靠同一个 `EquipmentAnimLayerClass`。
