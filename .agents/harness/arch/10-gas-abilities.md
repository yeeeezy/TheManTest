# GAS 技能系统

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
命中反馈采用独立所有权的两层 Cue：`ABulletBase::ImpactCueTag` 由具体武器/弹体选择武器命中表现；敌人 Health 确认扣减后，由目标 ASC 按 `AEnemyBase::HitReactionCueTag` 选择敌人受击表现。共享 `GE_BulletDamage` 只负责伤害，不绑定表现 Cue，也不要为 Cue 单独创建空壳 Gameplay Effect。
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
