# 装备系统

**何时读取：** 新增装备类型、修改装备生命周期（Equip / Unequip 行为）、新增插槽或动画层时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Weapons/_Shared/EquipmentBase/EquipmentBase.h` | `Equip()` / `Unequip()` / `PlayEquipMontage()`；StaticMesh / SkeletalMesh / RectLight 组件；插槽名；EquipMontage；EquipmentAnimLayerClass |
| `Source/TheManTest/Private/Weapons/_Shared/EquipmentBase/EquipmentBase.cpp` | 动画层目标同时包含 `ArmsViewMesh` 与 `GetMesh()`；Equip 同步 Link、Unequip 同步 Unlink。活动装备入口现在调用 C++ `PlayEquipEffect()`：对装备全部 Mesh 创建 MID，固定使用 VFXPack 材质参数 `Amount (S)`，在 0.45 秒内由 1 平滑过渡到 -1。`PlayEquipMontage()` 与资产字段暂时保留兼容，但开局及切枪流程均不再调用。 |

装备 Montage 必须包含 `UpperBodySlot` 轨道：主 `TABP_BodyLocomotion` 的中央 `WeaponUpperBody` 会在 `DefaultSlot` 之后从 `spine_01` 覆盖上半身，所以只放 `DefaultSlot` 虽然 Montage 计时正常，动作仍会被最终武器层遮掉。`UpperBodySlot` 位于中央混合之后，适合 Equip/开火/换弹等需要进入最终上半身输出的动作。

Equip Montage 兼容代码仍保留，但 FEAT-074 session178 起不再由开局或切枪流程播放。当前切枪在新层求值一帧后直接用材质溶解显示枪体，并标记一次无位移 Camera Cut 清除 TAA/TSR 的旧枪颜色历史。
| `Source/TheManTest/Public/Weapons/_Shared/WeaponBase/WeaponBase.h` | 武器基类（继承 EquipmentBase，当前为空壳） |
| `Source/TheManTest/Public/Weapons/_Shared/Firearms/Firearm.h` | 射击参数：`bIsHitscan` / `HitscanRange` / `FireRate` / `BulletClass` / `MuzzleSocketName`；弹药：`MagazineCapacity=30` / `CurrentAmmo` / `SpareMagazineCount=3`、Consume/Reload/CanFire/CanReload 与 `OnAmmoChanged`；主/副射击及独立 `ReloadAbilityClass`；**`GrantedASC` 缓存** |
| `Source/TheManTest/Private/Weapons/_Shared/Firearms/Firearm.cpp` | BeginPlay 按蓝图容量初始化满弹；Consume/Reload 广播弹药事件；`Equip()` 写 AimSource 并 GrantAbilities；`Unequip()` 回收技能 |

`AFirearm` 还提供可按具体武器覆盖的 `MuzzleEffect / MuzzleEffectRotation / MuzzleEffectScale`；`UGA_Shoot` 每发在实际 Muzzle Socket 附着一次性 Niagara。RepairGun 当前使用 `/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_SniperScout_Muzzle`（FEAT-072，从外部 Sniper Scout 精确迁入，包含枪口闪光与烟雾）；专属前向烟雾材质/纹理位于同一 RepairGun Muzzle 目录，其余复用依赖位于 `/Game/Core/_Shared/Effects/Muzzle/`。
该 RepairGun 专属 System 的火焰、Glow、Lens Flare、Y 形火焰和火花颜色曲线已灰度化为中性灰；共享依赖仍保持原色，不得为了 RepairGun 外观修改 `/Game/Core/_Shared`。

RepairGun 的成功射击 SoundWave 为 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Fire`，由 `BP_RepairGun.FireSound` 配置；`UGA_Shoot` 在真实枪口世界位置播放。空弹音效应使用独立字段/反馈链，不得复用实弹 `FireSound`。
`AFirearm` 的空弹配置为 `DryFireSound` 及独立 Volume/Pitch Multiplier。RepairGun 使用 `/Game/Weapons/RepairGun/Audio/S_RepairGun_DryFire`；仅在当前弹匣为 0、`ConsumeRound()` 失败时播放。

`AFirearm::FireCameraShake/FireCameraShakeScale` 是每把枪独立的纯视觉冲击配置，不得与改变 Controller Rotation 的真实 `AddRecoil` 合并。RepairGun 使用 `/Game/Weapons/RepairGun/Effects/Camera/CS_RepairGun_Fire`，当前为 0.14 秒高频短冲击、Scale=1.0。
RepairGun 当前为单独验收震屏而在武器蓝图覆盖 `RecoilPitch/RecoilYawMin/RecoilYawMax=0/0/0`；这是武器数据配置，不代表公共后坐力系统已删除。

FEAT-074 起，玩家枪口统一由 `AFirearm::GetMuzzleWorldTransform()` 解析：优先 SkeletalMesh 的命名 Socket，其次 StaticMesh 的命名 Socket，最后使用 `MuzzleLocalTransform * ActorTransform`。因此纯静态枪模也必须在武器 BP 配置正确的 `MuzzleLocalTransform`，不得退回相机位置伪造枪口。

FEAT-078 起，`UEquipmentManagerComponent::OnCurrentEquipmentChanged` 在首次装备和切枪后广播；Combat HUD 依靠它切换 Firearm 数据源。切角色销毁旧 Pawn 前必须解绑 Equipment/Ammo 委托，禁止 UI 每帧轮询。

FEAT-078 session251 起，玩家枪械可独立配置 `ReloadAbilityClass`；`GrantAbilities/RevokeAbilities` 与主/副射击一样按 Handle 授予回收。RepairGun 配置共享 `BGA_Reload`，由 `Input.Weapon.Reload` Gameplay Event 触发；当前换弹即时完成并沿用 `OnAmmoChanged` 更新 HUD。

`AFirearm` 提供可选 `StaticMeshOverlay`，附着主 StaticMesh、无碰撞且不投影，用于 VFXPack Rifle 的反法线 Outline 壳。`BP_RepairGun` 的实体枪仍为 `SM_RepairGun_Rifle`，描边壳为 `SM_RepairGun_Rifle_Outline`；不得单独拿 Outline 壳替代实体枪。
| `Source/TheManTest/Public/Weapons/_Shared/Firearms/Bullets/BulletBase.h` | CollisionSphere(QueryOnly) + BulletMesh + ProjectileMovement；`Damage`(SetByCaller 传入 HitEffectClass) / `HitEffectClass` / `bDestroyOnHit`；`InitBullet(发射者, SourceASC)`(忽略发射者防自撞) / `ProcessHit()` BlueprintNativeEvent。FEAT-073 起，玩家/非敌方弹体有效命中 `AEnemyBase` 时统一调用 `ReactToProjectileHit`，穿透判定优先。 |
| `Source/TheManTest/Public/Weapons/RepairGun/Bullets/RepairGunBullet.h` | 环境命中保持指数膨胀（e^(Rate×t)）与危险区压制；敌人命中施加 `SlowPercent`/`SlowDuration`（默认40%/2.5秒）后立即销毁。连续命中刷新时长、不叠加强度。 |

抛射体的根 `CollisionSphere` 必须保持 `Movable`；`ProjectileMovementComponent` 移动的是根碰撞组件，仅把子级 `BulletMesh` 设为 Movable 不足以让 Actor 飞行。若根球体为 Static，PIE 会报告 `CollisionSphere has to be 'Movable'`，子弹将停在生成点，直到其他物体碰到它才触发命中逻辑。

## 武器资产目录约定（FEAT-052）

- 多种武器复用的资源放在 `/Game/Weapons/_Shared/`，并按 `Mesh`、`Material`、`Textures`、`Animations`、`GAS` 等类型继续分层。
- 某把武器专属的资源放在 `/Game/Weapons/<WeaponName>/`，按 `Blueprint`、`Mesh`、`Material`、`Textures`、`Animation` 分层；不要把材质或贴图放进 `Mesh`。
- 通用弹体：`/Game/Weapons/_Shared/Meshes/SM_Shared_Bullet`，材质为 `/Game/Weapons/_Shared/Materials/M_Shared_Bullet` 与 `M_Shared_Bullet_Accent`。
- RepairGun 科技泡沫弹体：`/Game/Weapons/RepairGun/Meshes/SM_RepairGun_Bullet`，专属材质为 `/Game/Weapons/RepairGun/Materials/M_RepairGun_Bullet`。

## 武器动画层字段

`Equip()` 只用武器 BP 上的 `EquipmentAnimLayerClass` 操作角色 Mesh 的现有 AnimInstance：

FEAT-077 起，FPS 玩家只对 `ArmsViewMesh` Link/Unlink 武器层和播放装备 Montage。`CharacterMesh0` 不再重复切换武器层，而是在完整身体 locomotion 之后从 `ArmsViewMesh` Copy 最终上半身 Pose；非 FPS `ACharacter` 仍操作 `GetMesh()`。

- **`EquipmentAnimLayerClass`**（`LinkAnimClassLayers`，链接武器层）：必须与宿主使用同一 Skeleton，并实现 `ALI_WeaponAnim`。
- `EquipmentAnimClass` 与 `SetAnimInstanceClass` 路径已删除；武器不得整体替换角色基础 AnimBP。需要专属跑跳的特殊武器应把状态机放进自己的 Linked Anim Layer。

## FEAT-042 上半身武器层（session70）

当前项目使用 `AFPSCharacterBase + 角色专属 AnimBP + ArmsViewMesh` 路线：

- `EquipmentAnimLayerClass` 保持原职责：由 `AEquipmentBase::Equip()` 链接到 `AFPSCharacterBase::GetArmsMesh()`，也就是独立 FP viewmodel `ArmsViewMesh`。
- session70 最终方案：`EquipmentAnimLayerClass` 会同时链接到 FPS 角色的 `ArmsViewMesh` 和 `GetMesh()`。两个 mesh 可以使用同一个主 ABP（如 `ABP_BodyLocomotion`），各自拥有独立 AnimInstance，并各自执行武器层/AimIK。
- `AFirearm::Equip()` 在基类完成 Link 后，给 `ArmsViewMesh` 和 `GetMesh()` 上已链接的 `UFirearmAnimInstance` 都写入 `MuzzleLocalTransform`。
- `ShadowBodyMesh` / `LegsMesh` 仍跟随 `GetMesh()`，因此身体武器层和 AimIK 会自然进入影子/腿共享姿势。
- 不使用 Copy Pose From Mesh；武器切换仍靠同一个 `EquipmentAnimLayerClass`。

## 2026-08-04 装备显现与统一动画层

- VFXPack 装备显现由 `AEquipmentBase` 固定实现：MID 参数 `Amount (S)`，0.5 秒 cubic Hermite 1→0，首切线 -5.434987。
- FPS 角色装备/卸下时，武器 Linked Anim Layer 链接到角色所有 SkeletalMesh AnimInstance；Shadow/Legs 即使是 Leader follower 也保持同一最终 AnimClass 架构。FEAT-075 最终由隐藏的完整 `CharacterMesh0` 直接 CastHiddenShadow；重复 ShadowBody/ShadowUpperBody 为空。shadow-only 静态枪体附着 `CharacterMesh0` 的 `GripPoint`，不可附着已弃用 ShadowBody。
