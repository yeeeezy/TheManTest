# 装备系统

## 爆炸枪附着弹（2026-09-04）

- `Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h/.cpp`：`AExplosionGunBullet : ABulletBase`，`BP_ExplosionGunBullet` 的原生父类。基类先处理首次伤害、原 Impact Cue 与 Phantom 穿透；有效命中后停止碰撞/移动，附着组件或骨骼并计时。基类仅增加protected只读 `HasProcessedHit()`，其他枪行为不变。
- 蓝图 `Bullet|Explosion / ExplosionDelay` 默认2秒，可调；`AttachmentOffset` 默认4cm。零秒延后到下一Tick执行，重复碰撞不重复触发，EndPlay取消计时。
- 倒计时结束调用独立 `GameplayCue.Weapon.ExplosionGun.Explosion`，再销毁弹体。本阶段无第二次/范围伤害。
- ExplosionGun/Effects/Explosion/Systems/NS_ExplosionGun_Detonation 来源TMIIR的N_ExplosionGround_006；全部116包依赖owner-local。Cue包与音频分别在本枪GAS/GameplayCues和Audio。原PhysicalImpact与首次5点伤害不变。

**何时读取：** 新增装备类型、修改装备生命周期（Equip / Unequip 行为）、新增插槽或动画层时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Weapons/_Shared/EquipmentBase/EquipmentBase.h` | `Equip()` / `Unequip()` / `PlayEquipMontage()`；StaticMesh / SkeletalMesh / RectLight 组件；插槽名；EquipMontage；EquipmentAnimLayerClass |
| `Source/TheManTest/Private/Weapons/_Shared/EquipmentBase/EquipmentBase.cpp` | FPS 只对 ArmsViewMesh Link/Unlink；PlayEquipEffect 委托共享组件并按 bPlayEquipAnimation 播放本装备 EquipMontage；Unequip 取消效果、恢复材质并停止自己的动画。 |

装备 Montage 必须包含 `UpperBodySlot` 轨道：主 `TABP_BodyLocomotion` 的中央 `WeaponUpperBody` 会在 `DefaultSlot` 之后从 `spine_01` 覆盖上半身，所以只放 `DefaultSlot` 虽然 Montage 计时正常，动作仍会被最终武器层遮掉。`UpperBodySlot` 位于中央混合之后，适合 Equip/开火/换弹等需要进入最终上半身输出的动作。

开局和切换统一由 EquipmentManager.QueueEquipPresentation 等待一帧新姿势后显示。所有装备继承共享显现效果；bPlayEquipAnimation 默认 false，开启后同时播放本装备 EquipMontage，不影响其他装备的动画配置。
| `Source/TheManTest/Public/Weapons/_Shared/WeaponBase/WeaponBase.h` | 武器基类（继承 EquipmentBase，当前为空壳） |
| `Source/TheManTest/Public/Weapons/_Shared/Firearms/Firearm.h` | 射击参数：`bIsHitscan` / `HitscanRange` / `FireRate` / `BulletClass` / `MuzzleSocketName`；弹药：`MagazineCapacity=30` / `CurrentAmmo` / `SpareMagazineCount=3`、Consume/Reload/CanFire/CanReload 与 `OnAmmoChanged`；主/副射击及独立 `ReloadAbilityClass`；**`GrantedASC` 缓存** |
| `Source/TheManTest/Private/Weapons/_Shared/Firearms/Firearm.cpp` | BeginPlay 按蓝图容量初始化满弹；Consume/Reload 广播弹药事件；`Equip()` 写 AimSource 并 GrantAbilities；`Unequip()` 回收技能 |

`AFirearm` 还提供可按具体武器覆盖的 `MuzzleEffect / MuzzleEffectRotation / MuzzleEffectScale`；MuzzleEffectScale 原生默认 `(2,2,2)`，ExplosionGun 同为 2，ElectricGun 保留 2，RepairGun 保留专属 0.85；`UGA_Shoot` 每发在实际 Muzzle Socket 附着一次性 Niagara，并在激活前把 `MuzzleEffectScale` 的最大绝对轴写入 Niagara 暴露参数 `User.MuzzleScale`。需要响应该统一尺寸参数的 Niagara System 必须暴露同名 float；`NS_ElectricGun_LaserMuzzle` 的16个 emitter 与 `NS_ExplosionGun_PhysicalMuzzle` 的9个 emitter 均在 Particle Spawn 阶段通过 Uniform ScaleSpriteSize 模块读取该参数。建议武器蓝图把 XYZ 设为相同值，避免 Niagara 的统一倍率与组件非均匀 Transform 产生难以预测的组合。RepairGun 当前使用 `/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_SniperScout_Muzzle`（FEAT-072，从外部 Sniper Scout 精确迁入，包含枪口闪光与烟雾）；专属前向烟雾材质/纹理位于同一 RepairGun Muzzle 目录，其余复用依赖位于 `/Game/Core/_Shared/Effects/Muzzle/`。
该 RepairGun 专属 System 的火焰、Glow、Lens Flare、Y 形火焰和火花颜色曲线已灰度化为中性灰；共享依赖仍保持原色，不得为了 RepairGun 外观修改 `/Game/Core/_Shared`。

`AFirearm` 同时提供默认关闭的 `MuzzleFlashLight` PointLight，以及 `bEnableMuzzleFlashLight / Color / Intensity / AttenuationRadius / SourceRadius / LocalOffset / Duration` 配置。组件固定使用 Unitless、Inverse Square、Cast Shadows 与 Affect Translucent Lighting；`UGA_Shoot` 只在成功消耗实弹后的开火反馈链调用，连续开火重置淡出计时，`Unequip` 立即清理。灯光尺寸与 Niagara 共用 `MuzzleEffectScale`：`AttenuationRadius` 和 `SourceRadius` 乘以最大绝对轴，Intensity、Color、Duration 与 LocalOffset 不变。当前仅 `BP_ElectricGun` 启用：Laser 枪口倍率 1.0，精确线性颜色 `(0.075319,1,0.652928)`、UE 5.7 视觉补偿强度 1800、半径 200、SourceRadius 60、0.1 秒线性淡出；灯位通过枪口局部偏移 `(1.480382,-6.734961,-15.577805)` 对齐源枪体的持枪侧。RepairGun 保持关闭。ExplosionGun 已按 Physical 1 源蓝图启用：线性颜色 `(1,0.551385,0.147041)`、强度 300、半径 87.370407、SourceRadius 60、持续 0.1 秒、局部偏移 `(8.851011,-17.618745,2.96144)`，枪口倍率 2。ElectricGun 后续用户将倍率调为 2，保持该覆盖。`VFXTestMap` 的 Bloom=4、Threshold=0.5 仅用于对照源项目 MainScene/参考视频的临时 VFX 审核环境，不是枪械效果的替代实现。

FEAT-080 起，MaintenanceWorker 的三把初始枪均为 AFirearm Blueprint：BP_RepairGun、BP_ElectricGun、BP_ExplosionGun。ElectricGun 的 SM_ElectricGun 为 Ballistics Rifle 02，使用 LaserMuzzle/LaserImpact；ExplosionGun 的 SM_ExplosionGun_Rifle 为 Ballistics Rifle 01，使用 PhysicalMuzzle/PhysicalImpact。三枪无描边组件、描边壳或描边材质。ElectricGun/Materials/M_ElectricGun_Surface 与 ExplosionGun/Materials/M_ExplosionGun_Surface 为专属科幻 PBR 表面，分别由 MI_ElectricGun、MI_ExplosionGun_Rifle 引用；只有显现函数和噪声共享。枪体主模型、握持偏移与 MuzzleLocalTransform 保持对应。动画、音频、CameraShake、Bullet、Cue、弹体及武器 VFX 仍分别属于各自武器，不能重新引用 RepairGun 专属资源。

RepairGun 的成功射击 SoundWave 为 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Fire`，由 `BP_RepairGun.FireSound` 配置；`UGA_Shoot` 在真实枪口世界位置播放。空弹音效应使用独立字段/反馈链，不得复用实弹 `FireSound`。
`AFirearm` 的空弹配置为 `DryFireSound` 及独立 Volume/Pitch Multiplier。RepairGun 使用 `/Game/Weapons/RepairGun/Audio/S_RepairGun_DryFire`；仅在当前弹匣为 0、`ConsumeRound()` 失败时播放。

`AFirearm::FireCameraShake/FireCameraShakeScale` 是每把枪独立的纯视觉冲击配置，不得与改变 Controller Rotation 的真实 `AddRecoil` 合并。RepairGun 使用 `/Game/Weapons/RepairGun/Effects/Camera/CS_RepairGun_Fire`，当前为 0.14 秒高频短冲击、Scale=1.0。
RepairGun 当前为单独验收震屏而在武器蓝图覆盖 `RecoilPitch/RecoilYawMin/RecoilYawMax=0/0/0`；这是武器数据配置，不代表公共后坐力系统已删除。
2026-09-04 爆炸弹调试期间，`AFirearm::bEnableViewRecoil` 默认临时设为 `false`，`UGA_Shoot` 仅在该开关开启时调用 `AddRecoil`；原有 Pitch/Yaw/Damping 数据均保留。`FireCameraShake` 调用链未关闭，恢复实际视角后坐时只需把该开关改回 `true`。

FEAT-074 起，玩家枪口统一由 `AFirearm::GetMuzzleWorldTransform()` 解析：优先 SkeletalMesh 的命名 Socket，其次 StaticMesh 的命名 Socket，最后使用 `MuzzleLocalTransform * ActorTransform`。因此纯静态枪模也必须在武器 BP 配置正确的 `MuzzleLocalTransform`，不得退回相机位置伪造枪口。

FEAT-080 起，`UEquipmentManagerComponent::OnCurrentEquipmentChanged` 在首次装备和切枪后广播；Combat HUD 依靠它切换 Firearm 数据源。切角色销毁旧 Pawn 前必须解绑 Equipment/Ammo 委托，禁止 UI 每帧轮询。

FEAT-080 session251 起，玩家枪械可独立配置 `ReloadAbilityClass`；`GrantAbilities/RevokeAbilities` 与主/副射击一样按 Handle 授予回收。RepairGun 配置共享 `BGA_Reload`，由 `Input.Weapon.Reload` Gameplay Event 触发；当前换弹即时完成并沿用 `OnAmmoChanged` 更新 HUD。

2026-09-04：AFirearm.StaticMeshOverlay 原生组件已删除，三枪 Blueprint 的旧壳引用均已清空并重新编译。三个 Outline Mesh 与 M_EquipmentOutline 均无外部引用后删除。
| `Source/TheManTest/Public/Weapons/_Shared/Firearms/Bullets/BulletBase.h` | CollisionSphere(QueryOnly) + BulletMesh + ProjectileMovement；`Damage`(SetByCaller 传入 HitEffectClass) / `HitEffectClass` / `bDestroyOnHit`；`InitBullet(发射者, SourceASC)`(忽略发射者防自撞) / `ProcessHit()` BlueprintNativeEvent。`ABulletBase` 构造函数默认把 HitEffectClass 设为共享 `/Game/Weapons/_Shared/GAS/Effects/GE_BulletDamage`，新建子弹无需重复配置，特殊子弹仍可覆盖/清空。FEAT-073 起，玩家/非敌方弹体有效命中 `AEnemyBase` 时统一调用 `ReactToProjectileHit`，穿透判定优先。 |
| `Source/TheManTest/Public/Weapons/RepairGun/Bullets/RepairGunBullet.h` | 环境命中保持指数膨胀（e^(Rate×t)）与危险区压制；敌人命中施加 `SlowPercent`/`SlowDuration`（默认40%/2.5秒）后立即销毁。连续命中刷新时长、不叠加强度。 |

`ABulletBase` 提供可配置 `ImpactCueTag`，有效碰撞时由攻击者 ASC 执行，但基类不绑定任何具体枪械表现。RepairGun 子弹默认使用 `GameplayCue.Weapon.RepairGun.Impact`，对应 `/Game/Weapons/RepairGun/GAS/GameplayCues/GC_Weapon_RepairGun_Impact` 与 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Impact`；Gameplay Cue 资产名中的 `Weapon_RepairGun_Impact` 必须完整匹配 Tag 层级，确保 Asset Registry 的 `GameplayCueName` 可被运行时管理器发现。环境与 Enemy 都播放同一 RepairGun 反馈，Phantom 穿透不触发。命中 WAV 已离线压缩/软限幅，平均响度较原文件提高约 6.9dB、峰值为 -1dBFS，Cue `VolumeMultiplier` 保持 1.0，避免把近满幅瞬态用超大倍率直接推入总线限幅。Projectile 和当前 Hitscan 均汇入 `ProcessHit`。由于射击 Ability 为 `LocalOnly` 且 Projectile 可在 Ability 结束后才命中，命中反馈必须用 `InvokeGameplayCueEvent(Executed)` 立即本地执行；不得使用依赖 Authority/Prediction Key 的延迟 `ExecuteGameplayCue` 队列。

`UGCN_ImpactFeedbackBase` 对所有命中目标统一播放 `ImpactEffect`，不再按 Character 切换武器 Niagara；角色专用受击表现由目标自身的 `HitReactionCueTag` 负责。武器贴花仍只生成在环境表面。电击枪 Cue 使用 `GameplayCue.Weapon.ElectricGun.Impact`、`NS_ElectricGun_LaserImpact`（源 `NE_VFX_Projectile_Impact_Laser_2`）与紫色贴花（1.1）；其枪口使用 `NS_ElectricGun_LaserMuzzle`（源 `NE_VFX_Muzzle_Laser_Burst_2`）。Laser 依赖迁移后曾有 11 个材质实例的 `Main_Texture` 变成空引用并露出矩形 Sprite 面片，现已恢复为 `/Game/Weapons/ElectricGun/Effects/Textures` 下各自的 owner-local 贴图；后续整理依赖时必须校验材质实例参数值，不能只检查包是否存在。爆炸枪 Cue 使用 `GameplayCue.Weapon.ExplosionGun.Impact`、Physical Impact 1 与黄色贴花（2.0）。两个 Cue 的资产名与 Asset Registry `GameplayCueName` 必须完整匹配对应 Tag。

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

- 2026-09-04 起，显现移至 `UEquipmentEquipEffectComponent`：MID 参数 `Amount (S)`，0.5 秒 cubic Hermite 1→0，首切线 -5.434987；只在效果期间 Tick，结束/卸下恢复原材质。
- FPS 角色装备/卸下时，武器 Linked Anim Layer 链接到角色所有 SkeletalMesh AnimInstance；Shadow/Legs 即使是 Leader follower 也保持同一最终 AnimClass 架构。FEAT-075 最终由隐藏的完整 `CharacterMesh0` 直接 CastHiddenShadow；重复 ShadowBody/ShadowUpperBody 为空。shadow-only 静态枪体附着 `CharacterMesh0` 的 `GripPoint`，不可附着已弃用 ShadowBody。

## 通用装备特效所有权（2026-09-04）

- 运行时代码：Public/Private 下 `Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h/.cpp`。由 AEquipmentBase 构造默认组件 EquipEffect；适用枪械、近战武器及其他装备。
- 共享资产根：`/Game/Weapons/_Shared/Equipment/Effects/Equip/`，只保留 Materials/Functions/MF_EquipmentEquipDissolve 和 Textures/T_EquipmentEquipNoise。旧共享表面仅剩 RepairGun 使用，已迁回 RepairGun/Materials/M_RepairGun_Rifle；描边材质已删除。
- ElectricGun/ExplosionGun 材质实例引用各自 owner-local 科幻表面主材质，保留共享溶解函数。RepairGun 的 M_SCFR_BaseMat 在原图上接同一函数，不改原纹理与配色。
- 自定义装备材质必须接共享函数和 Amount (S) 才参与显现。组件只从原材质创建 MID，显现期间保留材质全部原参数；未接入的槽保持原样，不再替换为通用灰色表面。原备用路径会将 Outline 壳变为不透明灰壳，现已删除。
- 装备动画仍按各装备 EquipMontage / EquipmentAnimLayerClass 独立配置；bPlayEquipAnimation 可在蓝图开启，共用 VFX 不依赖动画内容。
- 测试：EquipmentEquipEffectTests.cpp / TheManTest.Equipment.SharedEquipReveal 验证三枪 MID 的父材质与原表面完全相同、实际渲染、取消/结束恢复、普通装备接入及可选动画；不兼容材质不得出现灰色备用表面。CombatHUDTests 的切枪测试等待实际显现结束，避免加载卡顿时 wall-clock 等待早于游戏时间完成。
