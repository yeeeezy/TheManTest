# 当前进度

## Active Feature

- FEAT-080，in_progress。2026-09-05最新一轮共享全身爆炸受击、移动附着/死亡清理、普通物理爆炸已实现并自验，整体保留用户手感/观感验收。
- 前序准星仰射、身体附着、Enemy Air007/缩放、条件子弹时间均已完成，细节见archive/FEAT-080-three-weapon-setup.md。

## 当前行为和入口

- 人形基类自动接`Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction`与`CR_Humanoid_HitReaction`。ABP无TargetSkeleton，Rig无Phantom资产引用；Phantom模型原PostProcess槽已清空，通过基类组件Override接入。
- ExplosionHitReaction组件：MaxAngleDegrees=38、AttackDuration=.055、RecoveryDuration=.85、FollowDelay=.045、LegCompression=7cm，BoneMapping可配。胸腹受力、头肩滞后跟随、髋部/膝盖缓冲，脚保持输入动画位置，胶囊不移动。不同骨骼层级仍须映射和Rig兼容适配，不是自动重定向。正式触发仍为爆炸范围伤害，普通直接子弹未额外触发此Rig。
- BP_ExplosionGunBullet → Bullet|Explosion|Physics：PhysicsImpulseRadius400cm、PhysicsImpulseStrength800，速度变化型线性衰减冲量，检查墙体遮挡；排除Enemy和Chaos，普通物体击飞不触发子弹时间。
- 爆炸弹仍5点首次伤害、2秒Fuse、20点/400cm延迟伤害。身体附着物随骨骼移动；Enemy结束时销毁待爆弹与身体血痕，自身爆炸造成击杀时允许Detonate完成。地面血迹保留独立寿命。
- 子弹时间只因本次爆炸击杀或真实Chaos Break触发。Chaos采用组件/位置/0.2游戏秒窗口匹配，不能严格区分同窗口其他力量造成的破碎。用户时间.05/.01/1/.01保留。
- Phantom → Phantom|Testing → Stationary Hit Test仍默认开启。取消后重新PIE可恢复原逻辑；正式AI移动还需恢复先前测试树/零速度设置。移动验收只在测试实例中开启行走。
- 用户Explosion Cue里的Enemy VFX开关、EnemyScale/声音/震屏设置未覆盖。前序准星修正保留；AttachmentOffset4cm和PhysicsAsset近似、极近大仰角身体遮挡仍是精度边界。

## 验证

- Development Editor Win64最终构建成功，无新增C++警告。
- SharedReactionFinal.log五项5/5 Success：EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。
- 四方向头部位移约21cm、髋部下降4.8cm、膝部位移10cm，脚位/胶囊和恢复通过；已查看TMT_EnemyRig_Directions.png。
- 非Phantom原生AHumanoidEnemy自动接入共享ABP，实际行走425cm、转45度并受击；弹体/身体Decal骨骼附着正确，倒计时死亡与爆炸击杀两条清理路径通过。
- 普通物理近/远速度约599/300cm/s；墙后、范围外静止，无子弹时间。原Chaos/条件结果九场景通过。
- SharedReactionColdVerified.log：共享ABP只依赖共享Rig，Rig无/Game依赖，Phantom导入源/预览/骨架引用清除，旧路径及Redirector均无；实际调参38/.85/.045/7冷读通过。

## 会话交接

- 写前选择性检查点8406b19保存上轮仰射和用户当时的Phantom ABP状态；本轮最终结果未提交/push。
- 新入口Scripts/VFX/install_shared_humanoid_reaction.py与validate_shared_humanoid_reaction.py；旧Audio配置脚本已移除Phantom Rig创建逻辑，避免还原旧路径。
- 原生测试最初遇TObjectPtr推导编译错误已修；移动测试最初MOVE_None已修为真实Walking；冷验发现Rig骨架导入源引用已清，所有最终验证通过。
- 地图、TestMap ExternalActor、用户Explosion Cue参数及其他未归属配置变化不纳入本轮；不得全量提交或撤销。无重定向、无关机操作，测试编辑器已退出。
- 本轮无未完成实现步骤，等待用户全身受击观感验收；后续按具体反馈调参。
