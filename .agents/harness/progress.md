# 当前进度

## Active Feature

- FEAT-080：三枪独立表现，in_progress；本轮HitStop、范围伤害、Enemy分支和痛呼已实现并验收，整体功能不关闭。
- 历史配置、变更与验证见 archive/FEAT-080-three-weapon-setup.md。

## 最新完成：HitStop、范围伤害与痛呼

- 最终要求：Enemy/环境都附着倒计时，结束均触发Chaos、声音、震屏、HitStop和范围伤害，只有爆炸Niagara不同。中途“Enemy立即销毁/只倒计时销毁”的方案均已撤销。
- BP_ExplosionGunBullet新增ExplosionDamage=20、ExplosionDamageRadius=400cm、ExplosionDamageEffectClass=GE_BulletDamage。范围只伤Enemy，候选去重，Visibility墙体遮挡，先判断遮挡再执行GE/Chaos；玩家不受范围伤害。
- 同一蓝图Bullet|Explosion|Hit Stop：Enabled=true、Duration=.06真实秒、TimeScale=.05、InnerRadius=200、OuterRadius=1500cm、MaxContinuousDuration=.12真实秒。Core/_Shared/Feedback/HitStopSubsystem保存/恢复原TimeDilation，连续上限与50ms恢复窗口，外部改速时让出控制，WorldEndPlay/Deinitialize清理。GC不管理时间/伤害。
- GC_Weapon_ExplosionGun_Explosion新增EnemyExplosionEffect，目前为空，以后填入后用实际爆点；环境ExplosionEffect继续Ground投影。Data.Explosion.EnemyImpact快照保存原命中类型。声音3倍、震屏8不变。
- GC_Character_Enemy_Hit叠加SCue_Enemy_Pain（下载424116 Wizard Pain），PainVolumeMultiplier=1、PainCooldown=.6真实秒。EnemyHitAudioComponent按每个敌人保存状态，附着/1条并发/播放中不重叠/销毁停止；原肉体声5倍率不变，痛呼有轻微随机和3D衰减。
- Development Editor Win64成功；HitStopPainDamageRegression六项Success。验证100→95→75、环境范围伤害/墙后和玩家免伤、Enemy Chaos及空VFX、时停恢复原速度/独立于GC和子弹销毁、两敌人痛呼独立/冷却/销毁停止。

## 保留：音效统一资产配置

- 12个现用玩法音效已封装owner-local SCue_*，Modulator轻微随机音高/音量（音量0.95~1），未来多素材支持Random无放回。扫描启停识别音保留稳定；未新增原本不存在的切枪/换弹声音。
- 开火/机械/环境命中/肉体/爆炸按用途配置衰减及并发，具体参数见arch/14-audio-policy.md。打角色时武器Impact不创建声音，角色自己的Hit负责；原粒子/贴花/伤害不变。
- 新增音效强制规范已写AGENTS.md；编辑器作者工具TheManAudioAssetLibrary与Scripts/Audio/configure_audio_cues.py提供可重复接入/冷验证。
- Development Editor Win64成功，ConfigureAudioCues3完成12Cue及9消费者Blueprint打开/编译/保存；ValidateAudioCues冷回读12/12通过，原素材依赖/消费者/无Redirector和音量5/3、震屏8检查通过。AudioPolicyRegression六项Success，实际录音左右/远近断言通过；AudioPolicyHitRouting补充三枪实际OnExecute打人不创建武器声断言Success。

## 保留：随机血迹与空间命中声

- 血迹与武器弹痕随机尺寸、旋转、长宽和MID图案；身体血迹修正为Mesh表面法线与骨骼附着，补Trace失败不悬空生成。血迹12秒/喷溅0.55秒不变。
- 四项Hit Cue接SA_ProjectileImpact：180cm内全量、再衰减2200cm，左右空间定位；并发12/8。Enemy肉体音量保留用户5，武器打人撞击层现在完全禁声；独立肉体Submix限幅减少峰值削波。
- Development Editor Win64成功；HitFeedbackFinal六项回归Success，HitSpatialFocused独立录音验证肉体/电击声音左右和远近均生效。测试临时取消后台静音并恢复，未改全局配置。新增三项材质Shader错误数0。
- 具体资产/字段/录音RMS见FEAT-080 archive与arch/10。视觉已查看孤立血迹截图，最终手感待用户反馈。

## 保留：Chaos Cube 与地面爆炸

- 可复用蓝图：/Game/Actors/DestructibleCube/Blueprint/BP_ChaosDestructibleCube，原生AChaosDestructibleCube。现为42块不规则Voronoi，大小混合、0.8cm断面凹凸；不再是27块规则网格。FractureAsset/Toughness可逐实例配置，大小使用Actor Scale；初碰不自动碎。
- VFXTestMap新增3个Cube，标签VFXTest_ChaosCube_1~3。其他场景直接拖入同一蓝图。
- ExplosionGunBullet两种命中都触发Chaos：ChaosRadius=400cm、ChaosStrain=500000、ChaosImpulse=1200速度变化、ChaosAngularSpeed=5rad/s随机翻滚；Strain后0.05游戏秒弱引用径向冲量/范围内逐粒子角速度。
- GroundSearchDistance=2000cm、GroundMaxSlope=45度。Actor Tag ExplosionGround不是Gameplay Tag/Component Tag；Object Multi Trace穿过Cube，拒绝Enemy/子弹/GeometryCollection。无合格地面时跳过Ground Niagara/内含decal。
- 地面标记：GASPTest的Plane；VFXTestMap的VFXTest_Floor；TestMap的Landscape与64个StreamingProxy（共65外部Actor包）。未标记墙、天花板、Cube或LobbyMap。
- Cue参数：Location为真实爆点，用于声音/震屏；EffectContext.HitResult携带地面落点，Niagara在地面+1cm播放。物理以真实爆点为中心，不在Cue里执行。

## 必须保留的前置配置

- BP_ExplosionGunBullet：ExplosionDelay=2秒，AttachmentOffset=4cm。首次5点伤害与原PhysicalImpact不变，新增上述延时范围伤害。
- 正式Explosion Cue：VolumeMultiplier=3、CameraShakeScale=8；Alien Cannon指定音频不改样本，0.45秒/200~1800cm方位衰减。Enemy Hit指定FleshHit音频倍率5（用户覆盖）。
- Ground Niagara：ExplosionGun/Effects/Explosion/Systems/NS_ExplosionGun_Detonation，来自TMIIR N_ExplosionGround_006，116包owner-local依赖，EffectLifeSpan=8秒兜底清理。
- ElectricGun Damage=0，有效首次Hit仍触发血花/受击声；正伤害、穿透、重复命中行为不变。血迹12秒、喷溅0.55秒。
- 三枪无描边；电击枪材质不变，爆炸枪保留橙红能量材质。MuzzleEffectScale两新枪XYZ=2，RepairGun保留0.85。
- 共用装备显现代码在Weapons/_Shared/EquipmentBase/Effects，仅从原材质创建MID，不换灰模；动画各装备独立配置。
- VFXTestMap静止Phantom、血条及临时关闭视角后坐配置保留。

## 最新验证

- 本轮IrregularCubeFinal.log：IrregularCubeGeometry、ExplosionChaosGround、StickyExplosionAndBlood、ExplosionDirectionalShake、ThreeWeaponBaseline 5/5 Success；Development Editor Win64编译成功。冷读取42块、体积284.6~72946.6cm³、总量1000000.1cm³，非轴对齐断面成立。
- RebuildIrregularCube.log生成/保存同路径正式GC，Cube Blueprint打开/编译/保存；没有加载保存或重新布置地图。已查看最新有照明的TMT_ChaosCube_Detonated.png，可见大小错落与斜面碎片。

- Development Editor Win64编译成功（包含全部本轮C++和测试）。
- ExplosionChaosFinal.log：ExplosionChaosGround、StickyExplosionAndBlood、ExplosionDirectionalShake、ThreeWeaponBaseline 4/4 Success。
- 覆盖真实Projectile Sweep附着、倒计时前不碎/结束RootBroken、半径外不碎、碎块扩散、穿过Cube找地面、坡度/标签/距离筛选、缺地面不生成Niagara、Enemy不引爆附近Cube、原伤害/血花/清理及声震配置。
- ValidateChaosAssets.log：冷回读两地板、65地形、3Cube与GC引用；三项Blueprint编译保存；音量3/震屏8、无Redirector通过。
- 已查看TMT_ChaosCube_Detonated.png：可见碎块扩散与地面效果。Intact截图由于测试区远离关卡灯光较暗，不作为外观验收证据。
- 初版物理数据缺失已修复：UpdateGeometryDependentProperties后生成SimulationData。早期NullRHI摆放Actor导致引擎除零，已切D3D重新作业并冷验证，无未保存结果被当作完成。

## 会话交接

- 本轮范围完成：爆炸子弹蓝图调HitStop/范围伤害，爆炸Cue填EnemyExplosionEffect，Enemy Hit Cue调Pain参数。HitStop是单机世界减速，短暂放慢游戏时间定时器/物理/动画，声音不变速。
- 最新安全检查点258bf70保存统一Sound Cue前置状态；本轮结果未最终Git提交，未push。地图/用户新增外部Actor保持原样。Scripts/Audio/configure_enemy_pain.py支持-AudioValidateOnly冷回读。
- TestMap的65个外部Actor包改动均为这次地形标记，不要误当无关编辑撤销。
- 可复用编辑器资产创建命令CreateTestCubeAsset(bRebuild=false)不在运行时/Construction调用；仅显式true重建不规则资产。PlanarCut插件与PlanarCut/Voronoi依赖限Editor。最新生成脚本Saved/Codex/rebuild_irregular_cube.py，不重存地图；旧install_chaos_cube.py用于首次布场，不要为重建碎块重新执行。
- 既有M_UE4Man_Body材质缺纹理及AimIK警告未在本轮处理。
