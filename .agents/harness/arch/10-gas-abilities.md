# GAS 技能系统

**何时读取：** 新增或修改 GAS Gameplay Ability、调试开火流程、新增 Gameplay Tag 时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/GAS/TheManGameplayTags.h` | 全局 Tag 声明：`TAG_Input_Weapon_PrimaryFire` / `TAG_Input_Weapon_SecondaryFire` / `TAG_Input_Character_Interact` / `TAG_Data_Damage`(`Data.Damage`，子弹伤害 SetByCaller) |
| `Source/TheManTest/Private/GAS/TheManGameplayTags.cpp` | Tag 定义（字符串绑定），新增 Tag 在此添加 `UE_DEFINE_GAMEPLAY_TAG` |
| `Source/TheManTest/Public/GAS/Abilities/GA_Shoot.h` | 玩家射击技能；CDO 中 `AbilityTriggers` 监听 `Input.Weapon.PrimaryFire` |
| `Source/TheManTest/Private/GAS/Abilities/GA_Shoot.cpp` | `ActivateAbility()`：从 `AFPSCharacterBase` 取当前 `AFirearm` → 从枪口生成 `Firearm->BulletClass` 子弹(`InitBullet`)，命中由子弹施加 `HitEffectClass`(GE_BulletDamage)；与开火反馈(蒙太奇/音效/后坐力)解耦，空枪也播反馈 |
| `Source/TheManTest/Public/GAS/Abilities/GA_InfiltratorScan.h` | 玩家扫描技能；`bScanActive` 独立控制开关并调用 `UScanEffectComponent`；全息 UI 为可选展示，类为空或生成失败都不能阻断扫描材质 |
| `Source/TheManTest/Public/GAS/Abilities/GA_EnemyShoot.h` | **敌人射击技能基类**；复用子弹管线；配置(BulletClass/Muzzle/蒙太奇/音效)作技能 UPROPERTY(技能=子弹绑定)；`virtual SpawnProjectiles()` 扩展点(散射/连发由子类重写)；**不注册 GameplayEvent 触发器**(由 UseRandomSkill 按类激活) |
| `Source/TheManTest/Private/GAS/Abilities/GA_EnemyShoot.cpp` | `ActivateAbility()`：`Cast<AHumanoidEnemy>` → 武器 Muzzle socket 取枪口 → 朝 `AimTargetWorld` → `SpawnProjectiles` 生成子弹 `InitBullet(Enemy, 敌人ASC)` |

`UGA_EnemyShoot` 公共基类统一处理人形敌人的渐进散射（基础、逐发扩散、上限、移动惩罚、恢复）和枪口 Niagara；三连发/扫射子类只负责节奏。默认人形步枪特效为 `/Game/Effects/_Shared/Muzzle/Systems/NS_HumanoidRifle_Muzzle`，具体技能蓝图可覆盖。
| `GA_EnemyAutomaticFire` | 数据化 `ShotsPerActivation`/`ShotInterval`；同一 C++ 能力配置成三连发或持续扫射；每发消费 `UEnemyMagazineComponent` |
| `GA_EnemyReload` | 仅空匣可激活，延时/动画均可配置，完成后把弹匣补满 |
| `GA_EnemyTakeCover` | 调通用 `AEnemyCoverPoint::FindBestCover`，可选 RollMontage，移动到 StandPoint |
| `GA_EnemyAreaBarrage` | 不消费普通弹匣；在目标范围上方随机生成可替换弹体并向下轰炸 |
| `Source/TheManTest/Public/Characters/Enemy/BTTask_UseCombatSkill.h` | **通用敌人战斗放招 BT 节点**；UPROPERTY `Range`(近/中/远)+`TargetActorKey`；读黑板目标 → `AEnemyBase::UseRandomSkill`；不绑定具体技能 |

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
