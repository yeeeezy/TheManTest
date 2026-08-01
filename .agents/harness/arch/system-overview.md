# 系统关系总览

```
ATheManPlayerController
  ├── 持有增强输入绑定（IMC / IA）
  ├── 路由输入 → AFPSCharacterBase（Move / Look / Jump / SwitchEquipment）
  ├── 路由开火输入 → AFPSCharacterBase::PrimaryFire() / SecondaryFire()
  └── 驱动角色切换 → SwitchCharacter(FName) → DT_CharacterRoster

ATheManPlayerState
  ├── 拥有 UAbilitySystemComponent（ASC）← GAS 唯一所有者
  └── 拥有 UTheManAttributeSetBase（Health / MaxHealth）

AFPSCharacterBase（当前活跃基类，纯第一人称）
  ├── 从 PlayerState 获取 ASC（PossessedBy 时初始化）
  ├── PossessedBy 末尾：为当前装备的 AFirearm 补授开火技能
  ├── PrimaryFire() / SecondaryFire()：向 ASC 发送 GameplayEvent Tag
  ├── HeadCamera（挂在 ArmsMesh head 骨骼）
  ├── ArmsMesh（Leader+动画宿主，只渲染手臂材质段，跟 capsule 俯仰；蓝图配置资产）
  ├── BodyRoot（SceneComponent，绝对旋转，Tick 只取 Yaw → 直立）  ← FEAT-038
  │     ├── ShadowBodyMesh（Follower，全身，OwnerNoSee+bCastHiddenShadow，只投影）
  │     └── LegsMesh（Follower，只渲染腿材质段，OnlyOwnerSee，无影）
  │     （三 mesh 同一 Skeleton，SetLeaderPoseComponent(ArmsMesh) 共享姿势；几何分离用材质段）
  ├── UEquipmentManagerComponent
  │     ├── AttachTargetMesh = ArmsMesh（BeginPlay 赋值）
  │     └── TArray<AEquipmentBase*> Inventory（运行时背包）
  └── UTheManCharacterDataAssetBase → InitGEClass（初始属性 GE）

装备继承链：
AEquipmentBase → AWeaponBase → AFirearm
  AEquipmentBase：Equip() 只设 AnimInstanceClass + 链接 AnimLayer（不播蒙太奇）；
                  PlayEquipMontage() 单独播拔枪动画（切角色初次装备推迟到下一帧）
  AFirearm 持有：
    ├── 射击配置：bIsHitscan / HitscanRange / BulletClass / MuzzleSocketName / FireRate
    ├── 开火反馈：FireMontage / FireSound / 后坐力参数
    ├── 技能配置：PrimaryFireAbilityClass / SecondaryFireAbilityClass + GrantedASC(切角色回收技能用缓存)
    └── Equip()：链接 ArmsMesh AnimLayer + GrantAbilities(缓存 GrantedASC)
        Unequip()：解链 AnimLayer + RevokeAbilities(ASC 为 null 时回退 GrantedASC)

GAS 开火流（FEAT-009/010/034）：
LMB → Character::PrimaryFire() → ASC->HandleGameplayEvent(Input.Weapon.PrimaryFire)
  → UGA_Shoot::ActivateAbility()（已授予，监听该 Tag）
    → 从枪口 Muzzle 生成 Firearm->BulletClass 子弹(InitBullet) + 播放蒙太奇/音效/后坐力
      → 子弹飞行命中 → ProcessHit 对目标 ASC 施加 HitEffectClass(GE_BulletDamage，SetByCaller 传 Damage)

具体角色（差异化在蓝图中配置）：
AFPSInfiltrator / AFPSMaintenanceWorker / AFPSTheExecutive
  └── 均继承自 AFPSCharacterBase（空壳）

动画实例：
UFPSCharacterAnimInstance（玩家 ABP 父类；FEAT-041 由 UFPSArmsAnimInstance 改名）
  ├── 继承 UBaseLocomotionAnimInstance，当前为空子类（仅 Locomotion 变量）
  ├── 挂 ArmsMesh(Leader)，驱动手臂/影子/腿三件套共享的全身姿势
  └── 每帧输出（来自基类）：Speed / Velocity_Z / bIsFalling / AimPitch / Direction

敌人继承链：
AEnemyBase（Public/Enemy/）
  ├── AHumanoidEnemy（Public/Enemy/Humanoid/）← 人形怪基类
  │     └── APhantom（Public/Enemy/Humanoid/Phantom/）← 幻影
  └── ANightmareEnemy（Public/Enemy/Nightmare/）← 梦魇基类
  - ASC + UEnemyAttributeSetBase 挂在自身（无 PlayerState）
  - UEnemyAttributeSetBase 继承 UTheManAttributeSetBase，怪物专属属性在此扩展
  - InitGEClass = GE_EnemyBase_Init（复制自 GE_CharacterBaseBase_Init，独立维护）
  - InitialMaxHealth / InitialHealth 直接暴露在蓝图（不用 DataAsset）
  - CurrentStrength = BaseStrength + RoundNumber（BeginPlay 计算）
  - OnDeath() 默认 Destroy()，蓝图子类可覆写

敌人战斗/AI（FEAT-031/032/035）：
AEnemyBase 技能集系统（通用）：
  ├── PhaseSkillSets：阶段数组，每阶段含 Near/Mid/FarAbilities（EEnemySkillRange）
  ├── CurrentPhase(默认1) / SetCombatPhase()；BeginPlay 经 GrantAbilities 全部授予
  ├── UseRandomSkill(Target, Range)：当前阶段对应距离档随机放招 → AimAtTarget(virtual) → TryActivateAbilityByClass
  └── 技能 = UGA_EnemyShoot 蓝图子类（技能与子弹绑定，复用子弹管线；逻辑不同重写 SpawnProjectiles）
AHumanoidEnemy：重写 AimAtTarget 写 AimTargetWorld（子弹方向/AimIK）；WeaponMesh 挂 hand_r；C++ 巡逻
AHumanoidAIController：Sight 感知(1500发现/1800丢失) → 发现玩家:BB.TargetActor+SetFocus+SetAIState(Aim)；
                      丢失:回 Patrol；Tick 在 Aim 下每帧更新 AimTargetWorld/bIsAiming
战斗流：感知发现 → SetAIState(Aim) → BT 战斗序列 MoveTo(TargetActor) → BTTask_UseCombatSkill(Range) → Wait
        BTTask_UseCombatSkill 不绑定具体技能，调 UseRandomSkill；丢失目标 Decorator 中断 → C++ 巡逻自驱

全局系统 / 游戏流程（详见 arch/13-game-flow.md）：
ATheManGameStateBase（Public/Core/）— 回合驱动
  - Tick → AdvanceRound()：倒计时 / 强度波(每 StrengthIncreaseInterval 默认150s) / 半场二阶段(OnCombatPhaseChanged)
  - 默认：BaseCountdownDuration=600 / StrengthIncreaseInterval=150 / Phase2TriggerRemainingFraction=0.5
  - ElapsedStrengthWaves / OnMidRoundStrengthIncrease / OnCombatPhaseChanged（敌人订阅）
  - BeginPlay 从 GameInstance 读 CarriedRoundNumber 衔接；DebugSkipTime() 调试快进
  - StartNewRound / OnRoundStarted / OnCountdownExpired(→玩家死亡)

死亡 → 大厅 → 选角色 → 重开（FEAT-037）：
  死亡(被打死 OnDeath / 时间到 OnCountdownExpired) → UTheManGameInstance::HandlePlayerDeath
    → 记 CarriedRoundNumber + OpenLevel(LobbyMap)
  LobbyMap(ATheManLobbyGameMode：建 WBP_CharacterSelect UI) → 按钮 SelectCharacter(ID)
    → GI.SelectCharacterAndStart → OpenLevel(TestMap)
  TestMap(ATheManGameModeBase.GetDefaultPawnClassForController 据 SelectedCharacterID 生成角色)
    + GameState 读回合数 +1（敌人初始强度逐回合 +1）
  跨关卡持久仅靠 GameInstance：SelectedCharacterID + CarriedRoundNumber
  注：输入模式跨关卡持久，PlayerController.BeginPlay 重置 GameOnly（否则动不了，BUG-037-001）

武器：
ATestGun（Public/Equipment/Firearms/）← 继承 AFirearm，空壳测试武器

动画基类（FEAT-021）：
UBaseLocomotionAnimInstance（Public/Characters/_Shared/Animation/）← 骨骼无关 C++ 基类
ABP_BaseLocomotion（Content/Characters/CharacterBase/Animations/Logic/）← Template ABP，不绑骨骼

✅ 已删除（FEAT-041，session43）：旧 ATheManCharacterBase / AInfiltrator / AMaintenanceWorker /
   ATheManExecutive / UTheManAnimInstanceBase 全部删除（备份 scratchpad/deprecated-char-backup-session43）。
   对应旧蓝图 BP_TheManCharacterBase / BP_Infiltrator / BP_MaintenanceWorker / BP_TheExecutive /
   ABP_MainCharacter / ABP_FirstPerson_MainCharacter 需在编辑器一并删除。
```
