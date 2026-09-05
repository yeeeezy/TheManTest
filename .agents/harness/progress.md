# 当前进度

## Active Feature

- 最新：直接Chaos命中条件和致命枪击击飞已完成。C++构建和相关6项PIE回归通过；本轮不改资产，待用户试力度。此前动画接入继续保留。

- FEAT-080，in_progress。爆炸受击已默认切到5条新动画，原共享Rig分支保留但默认绕过；模式可切回。编译、配置冷读/再编译、运行时方向/混合和7项回归均通过。待用户实战观感反馈。死亡布娃娃用户认可，Relax握枪修复已完成。
- 前序准星仰射、身体附着、Enemy Air007/缩放、条件子弹时间均已完成，细节见archive/FEAT-080-three-weapon-setup.md。

## 当前行为和入口

- 当前入口：BP_Phantom组件ExplosionHitReaction → Reaction Mode，默认Animation；改ControlRig可恢复原Rig链路。Enabled总开关仍有效。五条Front/Back/Left/Right/HeavyFrontAnimation可配，HeavyFrontMinStrength=.9、AnimationBlendIn=.06/BlendOut=.18、PlayRate=1。按Actor局部爆炸来源选动作，强正面选重击；动作期间继续结算后续伤害，但不重启动作。普通直接枪击没有新增存活动画触发。
- 共享后处理图中InputPose缓存→新SequenceEvaluator混合/旧ControlRig两条分支由模式选择。静止全身混合；速度>=10cm/s或下落时只混spine_01以上，腿/骨盆保持主AnimBP。游戏时间采样，随子弹时间放慢；结束自动回输入动画。原主AnimBP/AI/胶囊不替换，死亡照旧禁用PostProcess并转布娃娃。AHumanoidEnemy现在显式UPROPERTY持有原同名组件，解决具体BP参数保存丢失。

- 新成品样片：`Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_RifleHit_{Front,Left,HeavyTwist,Right,Back}`。绑定现役Rifle_01的70骨Skeleton；Wraith四方向+RifleAnimsetPro重击改编，握枪/腿部修正、首尾回Relax，时长1/.8667/1.6333/.8667/1秒。是当前具体骨架的Sequence样片，不是任意人形通用动画；共享Rig仍骨架无关，未增加Phantom依赖。
- 外部工作位于`D:/Blender Projects/HumanoidHitReactions`，主工程`Humanoid_RifleHit_Reactions.blend`、预览`Humanoid_RifleHit_Preview.gif`、静态`Reactions_Selected.png`。TMIIR成品在`/Game/ReactionPrep/Final`，TheManTest只导入已完成FBX，没有重定向工作资源。动态图是同步相位慢放比较，不表示各条时长相同。右侧较轻，HeavyTwist幅度大，主观效果待选。

- BP_Phantom.WeaponAttachSocket=hand_r_wepSocket，WeaponMesh.RelativeScale3D=(1,1,1)。源TMIIR Overview的5把示范枪都采用该配置，Socket挂在带动画轨道的hand_r_wep；此前固定hand_rSocket_Aim/.9枪械缩放错误。不能按Aim/Relax切两个静态手部Socket。只改BP_Phantom，没有修改动画轨道或C++。

- 人形基类自动接`Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction`，内部原`CR_Humanoid_HitReaction`仅ControlRig模式启用。ABP无TargetSkeleton，Rig及共享图无Phantom资产引用；具体Enemy自行配置同Skeleton兼容动画。Phantom模型原PostProcess槽已清空，通过基类组件Override接入。
- ExplosionHitReaction组件：MaxAngleDegrees=38、AttackDuration=.055、RecoveryDuration=.85、FollowDelay=.045、LegCompression=7cm，BoneMapping可配。胸腹受力、头肩滞后跟随、髋部/膝盖缓冲，脚保持输入动画位置，胶囊不移动。不同骨骼层级仍须映射和Rig兼容适配，不是自动重定向。正式触发仍为爆炸范围伤害，普通直接子弹未额外触发此Rig。
- Enemy → Enemy|Death：CorpseLifetime默认5游戏秒（最小.1），ProjectileHitImpulse默认5000 kg cm/s（0关闭枪击冲量）。死亡停止AI/技能/移动/Actor Tick、关闭胶囊/后处理动画，PhysicsAsset接管全身。人形手持武器死亡后无碰撞，避免反推尸体；Phantom取消隐身。没有PhysicsAsset的模型仍延时消失但无法布娃娃。
- 所有ABulletBase子类在伤害前确定骨骼局部命中点，伤害后对模拟身体施加点冲量，致命一枪/尸体再中枪（含0伤害）共用入口。尸体不再扣血，也不刷新寿命；肉体声/血痕保留，不新触发痛呼。
- BP_ExplosionGunBullet → Bullet|Explosion|Physics：PhysicsImpulseRadius400cm、PhysicsImpulseStrength800，线性衰减速度冲量，检查墙体遮挡。移除Enemy类型排除，非模拟身体由IsSimulatingPhysics过滤，Chaos仍独立处理。先伤害后物理，刚被炸死的敌人及已有尸体均可击飞；推动尸体/普通物体本身不触发子弹时间。
- 爆炸弹仍5点首次伤害、2秒Fuse、20点/400cm延迟伤害。身体附着物随布娃娃骨骼移动，Fuse继续可在尸体上爆炸；CorpseLifetime到期真正EndPlay时销毁待爆弹与身体血痕。地面血迹保留独立寿命。
- 子弹时间只因本次爆炸击杀，或爆炸弹直接命中GeometryCollection而在Fuse结束触发。直接命中无需实际破碎，范围波及破碎不触发；旧Chaos结果监听类保留但无调用。用户时间参数保留。
- Enemy|Death新增ProjectileKillKnockbackSpeed=250cm/s、ProjectileKillUpwardSpeed=120cm/s；直接致命枪击添加沿弹道/世界上方全身速度，原点冲量保留。两值设0关闭；旧尸体再次中枪不重复击飞，范围爆炸继续原径向冲量。
- Phantom → Phantom|Testing → Stationary Hit Test仍默认开启。取消后重新PIE可恢复原逻辑；正式AI移动还需恢复先前测试树/零速度设置。移动验收只在测试实例中开启行走。
- 用户Explosion Cue里的Enemy VFX开关、EnemyScale/声音/震屏设置未覆盖。前序准星修正保留；AttachmentOffset4cm和PhysicsAsset近似、极近大仰角身体遮挡仍是精度边界。

## 验证

- AnimationReactionRuntime3：四方向、强正面重击、旋转Actor方向、Enabled关闭、重复命中不重启、恢复输入和切回旧Rig全部通过；实际头位移18.41/40.52/28.38/9.19/33.42cm，枪挂点误差0。等速物理移动对照骨盆/腿/双脚误差<.5cm，主AnimInstance类保留。AnimationReactionCold重新加载并编译后五条引用仍存在，Shared ABP/Rig无Phantom依赖。
- AnimationReactionRegression.log：EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionRadialDamage、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood共7/7 Success/exit0。最终Development Editor Win64构建成功，无新增C++警告。

- 最终Development Editor Win64构建成功，无新增C++警告。EnemyRagdollFinal.log六项6/6 Success：EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。
- 三枪致命冲量、实际飞行0伤害弹推尸体、爆炸致死击飞通过；2秒/.8秒尸体寿命和骨骼挂弹/血痕到期清理通过。三枪总X动量约4780~4815，尸体再中枪骨盆X速度约103~116cm/s，爆炸致死约487cm/s。正式默认保留5秒，测试未改资产值。
- SharedReactionFinal.log五项5/5 Success：EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。
- 四方向头部位移约21cm、髋部下降4.8cm、膝部位移10cm，脚位/胶囊和恢复通过；已查看TMT_EnemyRig_Directions.png。
- 非Phantom原生AHumanoidEnemy自动接入共享ABP，实际行走425cm、转45度并受击；弹体/身体Decal骨骼附着正确，倒计时死亡与爆炸击杀两条清理路径通过。
- 普通物理近/远速度约599/300cm/s；墙后、范围外静止，无子弹时间。原Chaos/条件结果九场景通过。
- SharedReactionColdVerified.log：共享ABP只依赖共享Rig，Rig无/Game依赖，Phantom导入源/预览/骨架引用清除，旧路径及Redirector均无；实际调参38/.85/.045/7冷读通过。

## 会话交接

- 最新选择性检查点cdf510c保存此前动画接入源码/脚本/harness；二进制资产和用户地图/ExternalActor/音效/血纹理/电击弹配置不纳入。本轮仅代码/测试/文档修改，结果未最终提交/push。源工程/动画/Rig配置未改。
- 直接Chaos命中规则已替代范围Break监听；旧ExplosionOutcomeSubsystem保留无消费者。Enemy全身击飞可在Enemy|Death调两个速度，设0关闭。默认值已由实际BP_Phantom实例验证。
- DirectChaosLethalLaunch中5项Success；布娃娃直接击杀骨盆前向速度约303~454cm/s、向上81~118cm/s。旧尸体中枪向上约-16~2cm/s，没有重复120cm/s上抛。零参数回原点冲量。12项子弹时间条件验证通过，旁边Chaos真实破碎也不触发。
- 用户电击弹当前Damage=30，旧Sticky测试零伤害假设失效；只改测试实例为0，不改蓝图。重新构建成功，DirectChaosStickyFinal单项Success，相关6项均已通过。测试编辑器已退出、未保存地图。
- 前轮动画切换运行时与7项回归、Relax挂枪及样片制作已完成，详细记录见archive。动画默认Animation，旧Rig可切回；独立源动画/Blender工作仍在外部目录。地图、音效、VFX设置保持用户版本。
