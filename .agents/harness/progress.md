# 当前进度

## Active Feature

- FEAT-080：三枪独立表现，in_progress；本轮Chaos Cube与地面爆炸范围已实现并验证，整体功能不关闭。
- 历史配置、变更与验证见 archive/FEAT-080-three-weapon-setup.md。

## 最新完成：Chaos Cube 与地面爆炸

- 可复用蓝图：/Game/Actors/DestructibleCube/Blueprint/BP_ChaosDestructibleCube，原生AChaosDestructibleCube。真实27块聚类GeometryCollection；FractureAsset/Toughness可逐实例配置，大小使用Actor Scale；初碰不自动碎。
- VFXTestMap新增3个Cube，标签VFXTest_ChaosCube_1~3。其他场景直接拖入同一蓝图。
- ExplosionGunBullet内触发非Enemy的Chaos：ChaosRadius=400cm、ChaosStrain=500000、ChaosImpulse=1200速度变化；Strain后0.05秒弱引用径向冲量。Enemy命中仅保留原附着/倒计时/Cue，不触发Chaos，不新增伤害。
- GroundSearchDistance=2000cm、GroundMaxSlope=45度。Actor Tag ExplosionGround不是Gameplay Tag/Component Tag；Object Multi Trace穿过Cube，拒绝Enemy/子弹/GeometryCollection。无合格地面时跳过Ground Niagara/内含decal。
- 地面标记：GASPTest的Plane；VFXTestMap的VFXTest_Floor；TestMap的Landscape与64个StreamingProxy（共65外部Actor包）。未标记墙、天花板、Cube或LobbyMap。
- Cue参数：Location为真实爆点，用于声音/震屏；EffectContext.HitResult携带地面落点，Niagara在地面+1cm播放。物理以真实爆点为中心，不在Cue里执行。

## 必须保留的前置配置

- BP_ExplosionGunBullet：ExplosionDelay=2秒，AttachmentOffset=4cm。首次5点伤害与原PhysicalImpact不变，无延时二次伤害。
- 正式Explosion Cue：VolumeMultiplier=3、CameraShakeScale=8；Alien Cannon指定音频不改样本，0.45秒/200~1800cm方位衰减。Enemy Hit指定FleshHit音频倍率1。
- Ground Niagara：ExplosionGun/Effects/Explosion/Systems/NS_ExplosionGun_Detonation，来自TMIIR N_ExplosionGround_006，116包owner-local依赖，EffectLifeSpan=8秒兜底清理。
- ElectricGun Damage=0，有效首次Hit仍触发血花/受击声；正伤害、穿透、重复命中行为不变。血迹12秒、喷溅0.55秒。
- 三枪无描边；电击枪材质不变，爆炸枪保留橙红能量材质。MuzzleEffectScale两新枪XYZ=2，RepairGun保留0.85。
- 共用装备显现代码在Weapons/_Shared/EquipmentBase/Effects，仅从原材质创建MID，不换灰模；动画各装备独立配置。
- VFXTestMap静止Phantom、血条及临时关闭视角后坐配置保留。

## 最新验证

- Development Editor Win64编译成功（包含全部本轮C++和测试）。
- ExplosionChaosFinal.log：ExplosionChaosGround、StickyExplosionAndBlood、ExplosionDirectionalShake、ThreeWeaponBaseline 4/4 Success。
- 覆盖真实Projectile Sweep附着、倒计时前不碎/结束RootBroken、半径外不碎、碎块扩散、穿过Cube找地面、坡度/标签/距离筛选、缺地面不生成Niagara、Enemy不引爆附近Cube、原伤害/血花/清理及声震配置。
- ValidateChaosAssets.log：冷回读两地板、65地形、3Cube与GC引用；三项Blueprint编译保存；音量3/震屏8、无Redirector通过。
- 已查看TMT_ChaosCube_Detonated.png：可见碎块扩散与地面效果。Intact截图由于测试区远离关卡灯光较暗，不作为外观验收证据。
- 初版物理数据缺失已修复：UpdateGeometryDependentProperties后生成SimulationData。早期NullRHI摆放Actor导致引擎除零，已切D3D重新作业并冷验证，无未保存结果被当作完成。

## 会话交接

- 本轮范围完成，可在VFXTestMap直接试三个Cube；用户可调整弹体Ground/Chaos参数和Cube Toughness/Scale。
- 最新安全检查点f199564；结果未最终Git提交，未push。等待用户明确要求更新Git。
- TestMap的65个外部Actor包改动均为这次地形标记，不要误当无关编辑撤销。
- 可复用编辑器资产创建命令CreateTestCubeAsset不在运行时/Construction调用；工作脚本在Saved/Codex：install_chaos_cube.py、tag_explosion_landscape.py、validate_chaos_assets.py。
- 既有M_UE4Man_Body材质缺纹理及AimIK警告未在本轮处理。
