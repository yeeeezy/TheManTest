# 当前进度

## Active Feature

- FEAT-080，in_progress。5条当前骨架受击动作样片已完成并导入，冷读/首尾/依赖及实际PIE播放通过，动态图已打开，待用户看动作。尚未替换运行时共享Rig或接爆炸Montage。死亡布娃娃用户认可，Relax握枪修复已完成。
- 前序准星仰射、身体附着、Enemy Air007/缩放、条件子弹时间均已完成，细节见archive/FEAT-080-three-weapon-setup.md。

## 当前行为和入口

- 新成品样片：`Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_RifleHit_{Front,Left,HeavyTwist,Right,Back}`。绑定现役Rifle_01的70骨Skeleton；Wraith四方向+RifleAnimsetPro重击改编，握枪/腿部修正、首尾回Relax，时长1/.8667/1.6333/.8667/1秒。是当前具体骨架的Sequence样片，不是任意人形通用动画；共享Rig仍骨架无关，未增加Phantom依赖。
- 外部工作位于`D:/Blender Projects/HumanoidHitReactions`，主工程`Humanoid_RifleHit_Reactions.blend`、预览`Humanoid_RifleHit_Preview.gif`、静态`Reactions_Selected.png`。TMIIR成品在`/Game/ReactionPrep/Final`，TheManTest只导入已完成FBX，没有重定向工作资源。动态图是同步相位慢放比较，不表示各条时长相同。右侧较轻，HeavyTwist幅度大，主观效果待选。

- BP_Phantom.WeaponAttachSocket=hand_r_wepSocket，WeaponMesh.RelativeScale3D=(1,1,1)。源TMIIR Overview的5把示范枪都采用该配置，Socket挂在带动画轨道的hand_r_wep；此前固定hand_rSocket_Aim/.9枪械缩放错误。不能按Aim/Relax切两个静态手部Socket。只改BP_Phantom，没有修改动画轨道或C++。

- 人形基类自动接`Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction`与`CR_Humanoid_HitReaction`。ABP无TargetSkeleton，Rig无Phantom资产引用；Phantom模型原PostProcess槽已清空，通过基类组件Override接入。
- ExplosionHitReaction组件：MaxAngleDegrees=38、AttackDuration=.055、RecoveryDuration=.85、FollowDelay=.045、LegCompression=7cm，BoneMapping可配。胸腹受力、头肩滞后跟随、髋部/膝盖缓冲，脚保持输入动画位置，胶囊不移动。不同骨骼层级仍须映射和Rig兼容适配，不是自动重定向。正式触发仍为爆炸范围伤害，普通直接子弹未额外触发此Rig。
- Enemy → Enemy|Death：CorpseLifetime默认5游戏秒（最小.1），ProjectileHitImpulse默认5000 kg cm/s（0关闭枪击冲量）。死亡停止AI/技能/移动/Actor Tick、关闭胶囊/后处理动画，PhysicsAsset接管全身。人形手持武器死亡后无碰撞，避免反推尸体；Phantom取消隐身。没有PhysicsAsset的模型仍延时消失但无法布娃娃。
- 所有ABulletBase子类在伤害前确定骨骼局部命中点，伤害后对模拟身体施加点冲量，致命一枪/尸体再中枪（含0伤害）共用入口。尸体不再扣血，也不刷新寿命；肉体声/血痕保留，不新触发痛呼。
- BP_ExplosionGunBullet → Bullet|Explosion|Physics：PhysicsImpulseRadius400cm、PhysicsImpulseStrength800，线性衰减速度冲量，检查墙体遮挡。移除Enemy类型排除，非模拟身体由IsSimulatingPhysics过滤，Chaos仍独立处理。先伤害后物理，刚被炸死的敌人及已有尸体均可击飞；推动尸体/普通物体本身不触发子弹时间。
- 爆炸弹仍5点首次伤害、2秒Fuse、20点/400cm延迟伤害。身体附着物随布娃娃骨骼移动，Fuse继续可在尸体上爆炸；CorpseLifetime到期真正EndPlay时销毁待爆弹与身体血痕。地面血迹保留独立寿命。
- 子弹时间只因本次爆炸击杀或真实Chaos Break触发。Chaos采用组件/位置/0.2游戏秒窗口匹配，不能严格区分同窗口其他力量造成的破碎。用户时间.05/.01/1/.01保留。
- Phantom → Phantom|Testing → Stationary Hit Test仍默认开启。取消后重新PIE可恢复原逻辑；正式AI移动还需恢复先前测试树/零速度设置。移动验收只在测试实例中开启行走。
- 用户Explosion Cue里的Enemy VFX开关、EnemyScale/声音/震屏设置未覆盖。前序准星修正保留；AttachmentOffset4cm和PhysicsAsset近似、极近大仰角身体遮挡仍是精度边界。

## 验证

- 最终Development Editor Win64构建成功，无新增C++警告。EnemyRagdollFinal.log六项6/6 Success：EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。
- 三枪致命冲量、实际飞行0伤害弹推尸体、爆炸致死击飞通过；2秒/.8秒尸体寿命和骨骼挂弹/血痕到期清理通过。三枪总X动量约4780~4815，尸体再中枪骨盆X速度约103~116cm/s，爆炸致死约487cm/s。正式默认保留5秒，测试未改资产值。
- SharedReactionFinal.log五项5/5 Success：EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。
- 四方向头部位移约21cm、髋部下降4.8cm、膝部位移10cm，脚位/胶囊和恢复通过；已查看TMT_EnemyRig_Directions.png。
- 非Phantom原生AHumanoidEnemy自动接入共享ABP，实际行走425cm、转45度并受击；弹体/身体Decal骨骼附着正确，倒计时死亡与爆炸击杀两条清理路径通过。
- 普通物理近/远速度约599/300cm/s；墙后、范围外静止，无子弹时间。原Chaos/条件结果九场景通过。
- SharedReactionColdVerified.log：共享ABP只依赖共享Rig，Rig无/Game依赖，Phantom导入源/预览/骨架引用清除，旧路径及Redirector均无；实际调参38/.85/.045/7冷读通过。

## 会话交接

- 最新选择性检查点195a15c保存此前BP_Phantom握枪修复。本轮新增5条Sequence、import_humanoid_reaction_previews.py（仅导入成品，不做重定向）、validate_humanoid_reaction_assets.py与validate_humanoid_reaction_pie.py。ReactionImportFinal/ReactionImportCold/ReactionAssetsFinal/ReactionPreviewPIEFinal均成功；实际五条动作头部位移32.07/26.84/32.85/7.90/38.44cm，挂枪误差0。首尾70骨与现役Relax匹配，只有现有Skeleton依赖。测试编辑器退出、未写地图；无C++/BP变更。没有修改爆炸Cue/声音/死亡或运行时受击触发，动画接入须待用户看样片后决定。结果未提交/push。外部制作细节和失败迭代已归档。

- 最新选择性检查点a1d338f保存已完成布娃娃工作；本轮BP_Phantom握枪修复未提交。Scripts/VFX/fix_phantom_weapon_mount.py安装/-MountValidateOnly冷验证；PhantomMountInstall/Cold均成功。最终PhantomMountVisualFinal.log为MOUNT_PIE_OK，实际Relax/Aim/ReturnRelax挂点误差0、相对缩放1；已查看截图，Relax左手回到护木。角色Mesh自身.9体型保留，枪世界缩放随之.9。临时场景清理、退出，地图未保存，无剩余修复步骤。

- 写前选择性检查点b2bf304保存上轮共享Rig/物理爆炸状态；本轮最终结果未提交/push。
- 新入口Scripts/VFX/install_shared_humanoid_reaction.py与validate_shared_humanoid_reaction.py；旧Audio配置脚本已移除Phantom Rig创建逻辑，避免还原旧路径。
- 原生测试最初遇TObjectPtr推导编译错误已修；移动测试最初MOVE_None已修为真实Walking；冷验发现Rig骨架导入源引用已清，所有最终验证通过。
- 地图、TestMap ExternalActor、用户Explosion Cue参数及其他未归属配置变化不纳入本轮；不得全量提交或撤销。无重定向、无关机操作，测试编辑器已退出。
- 本轮修复布娃娃初期被手持WeaponMesh碰撞反推的问题。初次C4458局部名已修。最终六项通过，归档与索引已更新，无剩余实现步骤；测试编辑器已退出，结果未最终提交/push。
