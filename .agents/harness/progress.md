# 当前进度

## Active Feature

- FEAT-080：三枪独立表现，in_progress。最新完成平滑子弹时间、同GC分支声/VFX、旋转余震；整体观感待用户确认。历史见archive/FEAT-080-three-weapon-setup.md。

## 最新实现（2026-09-05）

- HitStopSubsystem已删除，替换为Core/_Shared/Feedback/BulletTimeSubsystem。BP_ExplosionGunBullet → Bullet|Explosion|Bullet Time：TimeScale=.2、SlowInDuration=.05、HoldDuration=.08、RecoveryDuration=.25真实秒，Inner/OuterRadius=200/1500cm。Smoothstep减速/恢复原速度，不超速；活动期不重启或延长，结束100ms恢复间隔，外部改速让出控制，世界结束清理。
- 同一个GC_Weapon_ExplosionGun_Explosion：环境ExplosionEffect/ExplosionSound；EnemyExplosionEffect/EnemyExplosionSound独立，无回退。EnemyEffectScale=1、EnemyVolumeMultiplier=3。Enemy资源未指定，目前声/VFX两槽为空，其他Gameplay仍执行。
- Explosion|Camera：ShakeDuration=.75真实秒、ShakeFrequency=12Hz、ShakeRotationDegrees=1.5；保留用户最新CameraShakeScale=4（非旧8），运行时最大8，200/1800cm平方衰减。冲击加多次衰减余震，零相机位移，同类不叠加。去掉原5cm×4视点平移风险，未改手臂/武器构图，不能宣称解决所有贴墙穿透。

## 保留的Gameplay与资产

- Enemy/环境均附着倒计时2秒；首次Damage5和Impact不变；爆炸20伤害/400cm，只伤Enemy、去重、Visibility墙体遮挡、玩家免伤；伤害/半径可调。
- 两分支均触发Chaos：Radius400、Strain500000、Impulse1200、AngularSpeed5rad/s；Strain后.05游戏秒弱引用冲量。BP_ChaosDestructibleCube仍42块不规则Voronoi，FractureAsset/Toughness逐实例可调。
- GroundSearchDistance2000、GroundMaxSlope45，仅Actor Tag ExplosionGround。环境Niagara用投影GroundHit；声音/震屏/物理用真实爆点，缺地面不播大decal。GASPTest/VFXTestMap地板、TestMap Landscape+64Proxy标签及三测试Cube不变。
- 环境爆炸SCue_ExplosionGun_Detonation（512565）音量3；Enemy肉体SCue_Enemy_FleshHit（166553）音量5；痛呼SCue_Enemy_Pain（424116）音量1，每敌人.6真实秒冷却、播放不叠加、销毁停止。
- 音效按arch14使用owner-local Sound Cue一次随机，按用途衰减/并发；打角色仅角色Hit发声。ElectricGun Damage0有效命中仍血花；血迹12秒、喷溅.55秒，身体骨骼附着、随机图案和空间化不变。
- 两新枪MuzzleEffectScale XYZ=2，RepairGun=.85。三枪无描边，枪体材质、共用装备原材质溶解不变；VFXTestMap静止Phantom/血条/临时关闭视角后坐保留。

## 验证

- Development Editor Win64编译成功。ConfigureExplosionFeedback.log：两个Blueprint打开/编译/保存，新默认与原伤害/音量3/用户震屏4回读通过。
- BulletTimeFeedbackRegression.log六项6/6 Success，进程exit0：BulletTimeAndPain、ExplosionDirectionalShake、ExplosionRadialDamage、StickyExplosionAndBlood、ExplosionChaosGround、ThreeWeaponBaseline。
- 覆盖渐入/保持/渐出曲线、不超速、重入不延长、恢复原.5而非1、外部改速不覆盖、无GC/弹体销毁后仍恢复；痛呼独立/冷却/销毁；震屏多次过零且零平移/方位/距离；100→95→75、环境20范围伤害、墙后与玩家免伤、Chaos。
- 冷回读日志Saved/Logs/ValidateExplosionFeedbackCold.log。Scripts/Audio/validate_explosion_feedback.py支持-FeedbackValidateOnly，只读验证无Redirector及当前配置。

## 会话交接

- 安全检查点4f6f676保存前置实现和用户BP覆盖；本轮未最终Git提交/未push。旧HitStop源码/测试已由BulletTime替代，可从检查点恢复；未删在用资产。
- 地图/ExternalActor未纳入这轮写入；TestMap65包为前轮地形标记，用户新增3/YJ、6/26、B/CV、B/ED、C/ZF目录保留，禁止撤销或全量提交。
- 下一步用户选择Enemy专用爆炸音效/VFX（两槽目前空），确认余震/子弹时间手感。如果仍穿模，需新画面区分贴墙武器、手臂构图或其他Shake。
- arch09/10/14记录新入口；configure_enemy_pain.py更新BulletTime验证；configure_audio_cues.py不再钉死旧震屏8。
- 既有M_UE4Man_Body缺纹理和AimIK警告未处理。Chaos重建只用显式CreateTestCubeAsset(true)，不重跑初始布场脚本覆盖地图。
