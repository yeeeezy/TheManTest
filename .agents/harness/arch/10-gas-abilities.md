# GAS 技能系统

## 2026-09-05 爆炸表现当前入口（覆盖下文历史值）

- 同一GC_Weapon_ExplosionGun_Explosion：环境ExplosionEffect/ExplosionSound/EffectScale/VolumeMultiplier；EnemyExplosionEffect/EnemyExplosionSound/EnemyEffectScale/EnemyVolumeMultiplier独立，无回退。目前Enemy声/VFX两槽为空，等待用户资源，伤害/Chaos/震屏/子弹时间仍执行。
- Explosion|Camera：保留用户CameraShakeScale=4（运行时上限8）、ShakeInner/OuterRadius=200/1800；新增ShakeDuration=.75真实秒、ShakeFrequency=12Hz、ShakeRotationDegrees=1.5。多次衰减余震，零相机位移；Cue初始化Pattern，每相机同类爆炸仅一条，防止无限叠加。方向仍由爆点→视点决定；未改ControllerRotation、手臂挂载或普通开火震屏。
- 环境爆炸音量3，Enemy肉体5，痛呼1。HitStop已删除，改为弹体向Core/_Shared/Feedback/BulletTimeSubsystem请求平滑子弹时间；GC只负责表现，详见arch09。

## 随机命中与空间音频（当前配置）

- Enemy Hit额外播放PainSound=`Enemy/_Shared/Audio/SCue_Enemy_Pain`（下载424116 Wizard Pain），独立PainVolumeMultiplier=1、PainCooldown=.6真实秒，原肉体ImpactSound/倍率5不变。懒创建UEnemyHitAudioComponent保存每敌人冷却与弱AudioComponent，播放期间不重叠，声音附着目标，目标结束时停止；静态GC自身不持有运行时冷却。
- 爆炸GC只负责Niagara/声音/震屏，不处理HitStop或二次伤害。Data.Explosion.EnemyImpact元数据选EnemyExplosionEffect（默认空），使用真实爆点；无该标记时原ExplosionEffect仍使用Ground投影。Enemy分支空VFX不代表跳过声音、震屏、Chaos或伤害。新Gameplay二次伤害在ExplosionGunBullet中处理，详见arch09。

- `UGCN_ImpactFeedbackBase::SpawnImpactSound`统一在命中点创建自动销毁AudioComponent，传入ImpactAttenuation/ImpactConcurrency。随机已移到Sound Cue，原PitchVariation/CharacterSoundMultiplier删除；新增音效强制遵守arch/14-audio-policy.md。
- 当前EnemyHit.VolumeMultiplier为用户设置的5，覆盖下文历史值1。打人时武器Impact完全不出声，由角色自己的Hit Cue发声；ShouldPlayImpactSound由EnemyHit重写拥有自身声音。四项命中Cue共用`/Game/Core/_Shared/Audio/SA_ProjectileImpact`：180cm内全量，外加2200cm线性衰减，Spatialize开启、StereoSpread=0。并发分别SC_ProjectileImpact=12与Enemy专属SC_EnemyFleshHit=8，StopQuietest。
- FleshHit源音频不改样本，输出到Enemy/_Shared/Audio/SMX_EnemyFleshHit，经SFX_EnemyFleshLimiter限制过大峰值；并非保证所有主输出绝不削波。
- 贴花随机旋转、尺度/长宽，并用独立MID的DecalPatternOffset改变Opacity噪声。两枪新增owner-local M_*_ImpactDecalVaried母材质，原MI参数保留；血迹原M_Enemy_BloodStain扩展同样参数。DecalSizeVariation/BloodSizeVariation可调。
- 身体血迹先Trace Mesh表面，失败向最近骨骼补Trace；使用实际表面法线/骨骼附着，无法命中Mesh则跳过，避免贴在胶囊外悬空。BodyStainProjectionDepth默认12cm；附着随骨骼刚性移动，不等于蒙皮纹理绘制。环境血迹和喷溅卡片同样随机，寿命12秒/0.55秒不变。

## 爆炸延时 Cue 与 Enemy 血迹（2026-09-04）

- Explosion Cue参数契约：Params.Location/Normal为真实附着爆点，音效与方位震屏继续使用此位置；Params.EffectContext.HitResult为弹体查询的ExplosionGround地面，仅Niagara使用ImpactPoint+Normal(1cm)。Context没有地面Hit时不生成Ground Niagara/内含decal，不回退Cube表面；Chaos物理不在Cue内。

- 最新用户覆盖：正式Explosion Cue的VolumeMultiplier=3（Alien Cannon素材），CameraShakeScale实际回读8且保留。EnemyHit.VolumeMultiplier=1。下文原生默认/初次接入配置不覆盖用户当前蓝图值。

- 零伤害弹命中：ABulletBase仅在Damage==0的有效非穿透首次Hit显式调用Enemy.ExecuteHitReactionCue(Context,0,true)。新可选bAllowZeroDamageHit默认false，原Health回调仍只接受正伤害；显式零伤害使用即时Invoke，不修改Damage，不重复播放。电击弹Damage保持0也可播放默认血花。

- 爆炸Cue使用用户提供的Alien Cannon音频 `Weapons/ExplosionGun/Audio/S_ExplosionGun_AlienDetonation`，近满幅素材保持VolumeMultiplier=1。`UExplosionCameraShake` / `UExplosionCameraShakePattern`（Weapons/ExplosionGun/Effects）提供0.45秒方位冲击，CameraShakeScale=3。Cue按爆炸点→本地相机方向传入UserDefined播放空间，200cm内全强、1800cm外不震、中间平方衰减；与开火震屏和ControllerRotation独立。默认Enemy Hit的ImpactSound为 `Enemy/_Shared/Audio/S_Enemy_FleshHit`，VolumeMultiplier=1；旧合成爆炸音频已无引用删除。

- `GameplayCue.Weapon.ExplosionGun.Explosion` → `/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion` → `UGCN_ExplosionGunExplosion`。由附着弹Timer触发，不合并原ImpactCue；ExplosionEffect/ExplosionSound/EffectScale/VolumeMultiplier独立配置，Niagara按+Z对齐法线。EffectLifeSpan默认8秒，通过弱绑定Timer兜底清理源效果的长尾焰，防止连续射击累积。无伤害GE。
- 默认 `GameplayCue.Character.Enemy.Hit` → `UGCN_EnemyHit` 现在配置 `BloodSprayMaterial/BloodStainMaterial/BloodScale/BloodStainLifeSpan`，默认12秒血迹，0.55秒喷溅。仍由真实Health扣减后的敌人ASC调用，不在某把枪内添加敌人分支。
- `Enemy/_Shared/Effects/EnemyBloodSpray.h/.cpp` 是默认EnemyHit的瞬态表现Actor（9个无碰撞/无阴影卡片、短弹道、朝向相机、MID Fade、自动销毁）。共享默认Cue由Phantom与其他Enemy继承；差异化敌人可改自身HitReactionCueTag。
- 源PNG、Texture与喷溅/贴花材质归 `/Game/Enemy/_Shared/Effects/Hit`；附着到敌人Mesh/bone的血迹与近处墙/地面血迹由敌人Cue负责，武器ImpactFeedbackBase的环境贴花规则保持原样。

**何时读取：** 新增或修改 GAS Gameplay Ability、调试开火流程、新增 Gameplay Tag 时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Core/_Shared/GAS/TheManGameplayTags.h` | 全局 Tag 声明；定义在对应 Private 路径 |
| `Source/TheManTest/Public/Weapons/_Shared/GAS/Abilities/GA_Shoot.h` | 多枪械共用的玩家射击技能；实现位于对应 Private 路径 |
| `Source/TheManTest/Public/Weapons/_Shared/GAS/Abilities/GA_Reload.h` | 多枪械共用的玩家换弹技能；监听 `Input.Weapon.Reload`，校验当前 Firearm 的 `CanReload()` 后调用 `ReloadMagazine()` |
| `Source/TheManTest/Public/Characters/Infiltrator/GAS/Abilities/GA_InfiltratorScan.h` | Infiltrator 专属扫描技能；实现位于对应 Private 路径 |
| `Source/TheManTest/Public/Enemy/_Shared/GAS/Abilities/` | 多种 Enemy 可复用的 `GA_EnemyShoot`、自动射击、换弹和掩体技能 |
| `Source/TheManTest/Public/Enemy/Humanoid/Phantom/GAS/Abilities/GA_EnemyAreaBarrage.h` | Phantom 专属二阶段区域轰炸技能 |

`UGA_EnemyShoot` 公共基类统一处理人形敌人的渐进散射（基础、逐发扩散、上限、移动惩罚、恢复）和枪口 Niagara；三连发/扫射子类只负责节奏。默认人形步枪特效为 `/Game/Enemy/Humanoid/Phantom/Effects/Muzzle/Systems/NS_HumanoidRifle_Muzzle`，具体技能蓝图可覆盖；跨系统共享依赖位于 `/Game/Core/_Shared/Effects/Muzzle/`。

FEAT-080 起，玩家 `UGA_Shoot` 取得当前 `AFirearm` 后首先调用 `ConsumeRound()`。空弹仅播放当前枪械独立的 `DryFireSound` 后结束 Ability，不生成弹体，也不播放实弹蒙太奇/音效、Niagara、震屏或后坐力；成功扣弹会同步广播 `OnAmmoChanged` 更新 Combat HUD。
命中反馈采用独立所有权的两层 Cue：`ABulletBase::ImpactCueTag` 由具体武器/弹体选择武器命中表现；敌人 Health 确认扣减后，由目标 ASC 按 `AEnemyBase::HitReactionCueTag` 选择敌人受击表现。共享 `GE_BulletDamage` 只负责伤害，不绑定表现 Cue，也不要为 Cue 单独创建空壳 Gameplay Effect。`ABulletBase` 已将该 GE 设为 C++ 默认 HitEffectClass，新子弹 Blueprint 只需配置 Damage；需要治疗或其他效果的特殊子弹可覆盖。
武器命中 Cue 不区分环境与 Character：`UGCN_ImpactFeedbackBase` 始终播放同一 `ImpactEffect`，贴花只落环境。不同敌人的血花、受击闪光或受击声必须放在各自 `HitReactionCueTag` 对应的 Cue；`GameplayCue.Character.Enemy.Hit` 是默认兜底，同 Tag 不会自动区分敌人类型，需要差异时为敌人定义专属 Tag 并在其 Blueprint 覆盖。
FEAT-080 新增原生 Tag `GameplayCue.Weapon.ElectricGun.Impact` 与 `GameplayCue.Weapon.ExplosionGun.Impact`；对应 Cue 扫描路径在 `DefaultGame.ini` 的 `AbilitySystemGlobals.GameplayCueNotifyPaths` 中逐武器注册。Tag 必须继续由 `TheManGameplayTags.h/.cpp` 声明/定义，具体 Bullet 与 Cue Blueprint 只选择正式 Tag。
成功射击的震屏由 `UGA_Shoot` 读取当前枪械 `FireCameraShake/Scale` 并通过本地 `PlayerCameraManager` 播放，只负责短促打击感；随后 `AddRecoil` 才负责实际控制视角上抬。两者不得互相替代。
2026-09-04 临时测试配置：`UGA_Shoot` 保持 Camera Shake 播放，但以 `AFirearm::bEnableViewRecoil` 包住 `AddRecoil`；公共默认值当前为 `false`，因此三把枪不会推动 Controller Rotation，原后坐力参数未删除。
session251 起，玩家 `UGA_Reload` 由 `IA_Reload(R)` 经 Character 发送 `Input.Weapon.Reload` Gameplay Event 激活。Ability 属于当前枪械，使用独立 `ReloadAbilityClass/Handle` 随 Equip/Unequip 授予回收；当前为即时换弹，满弹或无备用弹夹时拒绝激活。
Phantom 的四个具体射击 Ability（Shoot1/Shoot2/Burst/Suppressive，FEAT-071）统一覆盖为 BaseSpread=3°、PerShot=0.8°、Max=9°、Recovery=2°/s、MovingPenalty=2°；其两种子弹 Damage=6。公共 C++ 默认值保持不变，避免无依据影响未来其他 Enemy。
| `GA_EnemyAutomaticFire` | 数据化 `ShotsPerActivation`/`ShotInterval`；同一 C++ 能力配置成三连发或持续扫射；每发消费 `UEnemyMagazineComponent` |
| `GA_EnemyReload` | 仅空匣可激活，延时/动画均可配置，完成后把弹匣补满 |
| `GA_EnemyTakeCover` | 调通用 `AEnemyCoverPoint::FindBestCover`，可选 RollMontage，移动到 StandPoint |
| `GA_EnemyAreaBarrage` | 不消费普通弹匣；在目标范围上方随机生成可替换弹体并向下轰炸 |
| `Source/TheManTest/Public/Enemy/BTTask_UseCombatSkill.h` | **通用敌人战斗放招 BT 节点**；UPROPERTY `Range`(近/中/远)+`TargetActorKey`；读黑板目标 → `AEnemyBase::UseRandomSkill`；不绑定具体技能 |

> **玩家技能授予时序（武器持有，ASC 在 PlayerState）：**
> - 角色 `BeginPlay` 时 ASC 尚未就绪，`Equip()` 中的 `GrantAbilities()` 会因 ASC 为空而跳过。
> - `PossessedBy()` 初始化 ASC 后，在末尾调用 `Firearm->GrantAbilities(ASC)` 补授。
> - 运行时切枪时，`Unequip()` 回收旧技能，`Equip()` 授予新技能（此时 ASC 已就绪）。
> - **切换角色的技能回收（重要）**：旧角色被 `Destroy` 前已被 `Possess` 转走 PlayerState，`GetAbilitySystemComponent()` 返回 null。`AFirearm` 在 `GrantAbilities` 时缓存 `GrantedASC`(`TWeakObjectPtr`)，`RevokeAbilities` 传入 null 时回退到它，确保技能一定回收，否则 `GA_Shoot` 规格泄漏累积 → 一次开火多颗子弹炸膛。

> **敌人技能系统（ASC 在敌人自身；技能集 = 阶段 × 近/中/远）：**
> - `AEnemyBase`：`PhaseSkillSets`（`TArray<FEnemyPhaseSkillSet>`，[0]=阶段1…；每项含 `Near/Mid/FarAbilities`）+ `CurrentPhase`(默认1) + `SetCombatPhase()`。
> - `BeginPlay` 经 `GrantAbilities()` 授予 `DefaultAbilities` + 所有阶段所有距离档技能。
> - `UseRandomSkill(Target, EEnemySkillRange)`：从随机起点轮询距离档能力；某能力因空匣等条件拒绝激活时继续尝试其他候选。
> - Phantom 阶段1：TakeCover/Burst/SuppressiveFire/Reload；阶段2保留全部并新增 AreaBarrage。用 `PhaseSkillSets` 数据注入，无需 Phantom 专属行为树子树。
> - 触发链：感知发现玩家→`SetAIState(Aim)` → BT 战斗序列 `BTTask_UseCombatSkill`(配 Range) → `UseRandomSkill`。详见 FEAT-032 archive。

> **新增 GAS 技能的标准流程：**
> - 玩家技能：声明触发 Tag（`Input.角色.技能名`）→ 继承 `UGameplayAbility` 用 `AbilityTriggers` 监听 → 角色 `DefaultAbilityClasses` 或武器 `PrimaryFireAbilityClass` 引用 → 编辑器建蓝图子类赋值。
> - 敌人射击技能：继承 `UGA_EnemyShoot`（仅数据不同建蓝图子类；逻辑不同重写 `SpawnProjectiles`）→ 放进敌人 `PhaseSkillSets` 的某距离档 → BT 用 `BTTask_UseCombatSkill`(Range 对应)触发。**不需要触发 Tag**（按类激活）。
