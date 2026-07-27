# 装备系统

**何时读取：** 新增装备类型、修改装备生命周期（Equip / Unequip 行为）、新增插槽或动画层时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Equipment/EquipmentBase/EquipmentBase.h` | `Equip()` / `Unequip()` / `PlayEquipMontage()`；StaticMesh / SkeletalMesh / RectLight 组件；插槽名；EquipMontage；EquipmentAnimLayerClass |
| `Source/TheManTest/Private/Equipment/EquipmentBase/EquipmentBase.cpp` | 动画层目标同时包含 `ArmsViewMesh` 与 `GetMesh()`；Equip 同步 Link、Unequip 同步 Unlink，且 Montage 不负责改变动画层。`PlayEquipMontage()` 同步播放在 FP 手臂和隐藏身体宿主，ShadowBodyMesh 经 Leader Pose 继承 |

装备 Montage 必须包含 `UpperBodySlot` 轨道：主 `TABP_BodyLocomotion` 的中央 `WeaponUpperBody` 会在 `DefaultSlot` 之后从 `spine_01` 覆盖上半身，所以只放 `DefaultSlot` 虽然 Montage 计时正常，动作仍会被最终武器层遮掉。`UpperBodySlot` 位于中央混合之后，适合 Equip/开火/换弹等需要进入最终上半身输出的动作。

拔枪 Montage 建议 `Blend In = 0`。初始装备与切枪都先完整链接当前武器层，再于下一帧播放 Montage；结束后保持该层不变。若保留默认 0.25 秒 Blend In，持枪姿势会混入动画下方起始姿势，视觉上变成先放下再拿起。
| `Source/TheManTest/Public/Equipment/WeaponBase/WeaponBase.h` | 武器基类（继承 EquipmentBase，当前为空壳） |
| `Source/TheManTest/Public/Equipment/Firearms/Firearm.h` | 射击参数：`bIsHitscan` / `HitscanRange` / `FireRate` / `BulletClass` / `MuzzleSocketName`；开火反馈：`FireMontage` / `FireSound` / 后坐力；技能：`PrimaryFireAbilityClass` / `SecondaryFireAbilityClass`；`GrantAbilities()` / `RevokeAbilities()`；**`GrantedASC`(TWeakObjectPtr 缓存，切角色回收技能用)** |
| `Source/TheManTest/Private/Equipment/Firearms/Firearm.cpp` | `Equip()` 在基类链接层后写入 Arms/Body AimSource 并 GrantAbilities；`Unequip()` 先 RevokeAbilities 再由基类解链 |
| `Source/TheManTest/Public/Equipment/Firearms/Bullets/BulletBase.h` | CollisionSphere(QueryOnly) + BulletMesh + ProjectileMovement；`Damage`(SetByCaller 传入 HitEffectClass) / `HitEffectClass` / `bDestroyOnHit`；`InitBullet(发射者, SourceASC)`(忽略发射者防自撞) / `ProcessHit()` BlueprintNativeEvent |
| `Source/TheManTest/Public/Equipment/Firearms/Bullets/RepairGunBullet.h` | 指数膨胀（e^(Rate×t)）；膨胀到 MaxExpansionScale 后锁定，LifetimeAfterExpansion 秒后销毁 |

抛射体的根 `CollisionSphere` 必须保持 `Movable`；`ProjectileMovementComponent` 移动的是根碰撞组件，仅把子级 `BulletMesh` 设为 Movable 不足以让 Actor 飞行。若根球体为 Static，PIE 会报告 `CollisionSphere has to be 'Movable'`，子弹将停在生成点，直到其他物体碰到它才触发命中逻辑。

## 武器资产目录约定（FEAT-052）

- 多种武器复用的资源放在 `/Game/Weapons/_Shared/`，并按 `Mesh`、`Material`、`Textures`、`Animations`、`GAS` 等类型继续分层。
- 某把武器专属的资源放在 `/Game/Weapons/<WeaponName>/`，按 `Blueprint`、`Mesh`、`Material`、`Textures`、`Animation` 分层；不要把材质或贴图放进 `Mesh`。
- 通用弹体：`/Game/Weapons/_Shared/Mesh/SM_Shared_Bullet`，材质为 `/Game/Weapons/_Shared/Material/M_Shared_Bullet` 与 `M_Shared_Bullet_Accent`。
- RepairGun 科技泡沫弹体：`/Game/Weapons/RepairGun/Mesh/SM_RepairGun_Bullet`，专属材质为 `/Game/Weapons/RepairGun/Material/M_RepairGun_Bullet`。

## 武器动画层字段

`Equip()` 只用武器 BP 上的 `EquipmentAnimLayerClass` 操作角色 Mesh 的现有 AnimInstance：

- **`EquipmentAnimLayerClass`**（`LinkAnimClassLayers`，链接武器层）：必须与宿主使用同一 Skeleton，并实现 `ALI_WeaponAnim`。
- `EquipmentAnimClass` 与 `SetAnimInstanceClass` 路径已删除；武器不得整体替换角色基础 AnimBP。需要专属跑跳的特殊武器应把状态机放进自己的 Linked Anim Layer。

## FEAT-042 上半身武器层（session70）

当前项目使用 `AFPSCharacterBase + 角色专属 AnimBP + ArmsViewMesh` 路线：

- `EquipmentAnimLayerClass` 保持原职责：由 `AEquipmentBase::Equip()` 链接到 `AFPSCharacterBase::GetArmsMesh()`，也就是独立 FP viewmodel `ArmsViewMesh`。
- session70 最终方案：`EquipmentAnimLayerClass` 会同时链接到 FPS 角色的 `ArmsViewMesh` 和 `GetMesh()`。两个 mesh 可以使用同一个主 ABP（如 `ABP_BodyLocomotion`），各自拥有独立 AnimInstance，并各自执行武器层/AimIK。
- `AFirearm::Equip()` 在基类完成 Link 后，给 `ArmsViewMesh` 和 `GetMesh()` 上已链接的 `UFirearmAnimInstance` 都写入 `MuzzleLocalTransform`。
- `ShadowBodyMesh` / `LegsMesh` 仍跟随 `GetMesh()`，因此身体武器层和 AimIK 会自然进入影子/腿共享姿势。
- 不使用 Copy Pose From Mesh；武器切换仍靠同一个 `EquipmentAnimLayerClass`。
