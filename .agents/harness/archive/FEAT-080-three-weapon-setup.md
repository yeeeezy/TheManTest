# FEAT-080 RepairGun、电击枪与爆炸枪统一动画和独立 VFX

## 目标

- 为 RepairGun 接通现有第一人称开火蒙太奇。
- 以 RepairGun 为完整配置基线创建电击枪和爆炸枪。
- 两把新枪只替换模型、枪口 VFX、命中 VFX 与贴花；玩法、动画、音频、弹药、GAS、后坐力和子弹行为保持与 RepairGun 一致。
- 外部来源仅迁移最终模型和 VFX 依赖，不迁移源项目角色、武器蓝图或动画。

## 2026-09-04 不规则真实破碎

- 用户不接受规则3x3x3分块，确认改为不规则Voronoi、大小错落、凹凸断面与随机翻滚；伤害、Fuse、Cue、Ground规则不动。选择性检查点db9a156保存前置源码/文档/Cube资产，地图及用户新增外部Actor未纳入。
- CreateTestCubeAsset(bRebuild)改为在临时UGeometryCollection中切割完整100cm Cube：24个分散随机点+18个局部密集点，固定seed92417；原生PlanarCut/Voronoi，断面noise幅度0.8cm、间距5cm，零grout。成功后替换同一GC资产的几何与材质槽，保留使用方路径；常规调用不重建，只有显式true重建。
- Bullet新增ChaosAngularSpeed默认5rad/s；保留径向冲量，延迟释放碎块时对半径内逐粒子随机角速度，仅非Enemy分支。PlanarCut插件仅Editor启用，Build.cs的PlanarCut/Voronoi也仅Editor依赖，运行时不切网格。
- RebuildIrregularCube.log已保存正式GC并打开/编译/保存Cube蓝图；未保存或重新布置地图。新增形状统计和回归验证进行中。
- 最终Development Editor Win64构建成功；IrregularCubeFinal.log中IrregularCubeGeometry、ExplosionChaosGround、ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline 5/5 Success。冷读取统计42个刚体块，体积284.6~72946.6cm³，总体积1000000.1cm³，12085个非轴对齐顶点法线；总量保持100cm实心Cube，没有规则网格碎块。
- PIE真实Sweep附着、延迟RootBroken/飞散、范围外不碎、地面VFX、Enemy分支排除、声震/血花/原伤害回归全部通过。测试区新增临时照明（不写地图），已查看TMT_ChaosCube_Detonated.png，大小错落且斜面碎片清晰可见。
- 仅覆盖既有GC和编译Cube BP，未删除资产；原规则版可从db9a156恢复。地图/外部Actor包保持用户前置工作区状态，结果未最终提交/push，FEAT-080保持in_progress。

## 2026-09-04 Chaos Cube 与爆炸地面投射

- 用户确认：可摆放Chaos Cube类，爆炸弹非Enemy命中时倒计时触发Chaos；Enemy分支不扩展，不增加伤害。实际爆点负责物理/声音/震屏，Ground Niagara只在Actor Tag `ExplosionGround`且坡度合格的表面播放。检查点f199564保存前轮音量覆盖。
- 新增Actors/DestructibleCube/ChaosDestructibleCube，27块闭合Cube网格组成真实聚类GeometryCollection；FractureAsset/Toughness与Actor Scale可调。触发逻辑留在ExplosionGunBullet，半径400cm、Strain500000、速度变化1200；初碰不会自动碎。
- GroundSearchDistance=2000cm，GroundMaxSlope=45度；Object Multi Trace穿过Cube，拒绝Enemy/弹体/GeometryCollection。Params.Location保持实际爆点，EffectContext.HitResult仅携带合格地面；无地面跳过Niagara，保留声/震。
- Development Editor Win64初次构建成功。GASPTest Plane、VFXTestMap VFXTest_Floor已标记；VFXTestMap新增3个不同尺寸Cube。TestMap分区地形标记及完整PIE验证进行中。
- 第一次NullRHI脚本在摆放Actor时触发引擎除零异常，已改用D3D编辑器完成资产/关卡保存；未将崩溃当作保存成功，已重新加载现有资产后续作业。
- 初版GC仅组装网格，缺少凸包/体积等数据，实弹穿透；补充UpdateGeometryDependentProperties后创建SimulationData，实弹附着、RootBroken与碎块位移全部通过。测试临时地板需先Movable再SetStaticMesh；坡度案例用薄平板，避免旋转立方体另一面实际仍合格。
- TestMap的Landscape及64个StreamingProxy通过WorldPartitionBlueprintLibrary加载并写Actor Tag，共65项外部Actor包；GASPTest/VFXTestMap各仅一个真实地板被标记。没有标记天花板、墙或Cube。ValidateChaosAssets.log独立冷回读全部65地形、两地板、3Cube及无Redirector，三项Blueprint编译保存；音量3/震屏8保持。
- ExplosionChaosPhysics.log：ExplosionChaosGround与StickyExplosionAndBlood 2/2 Success，验证真实ProjectileSweep附着、倒计时后RootBroken、半径外不碎、碎块扩散、地面落点与Enemy不引爆附近Cube；原5点伤害/零伤害血花/清理保持。最终增加缺失地面不生成Niagara断言与两张PIE截图，四项回归见ExplosionChaosFinal.log。
- 最终ExplosionChaosFinal.log：ExplosionChaosGround、StickyExplosionAndBlood、ExplosionDirectionalShake、ThreeWeaponBaseline全部4/4 Success。已查看Detonated截图，可见碎块与地面效果；Intact截图因测试区域远离灯光较暗，不作为外观验收。当前未最终提交，整体FEAT-080仍in_progress。

## 2026-09-04 指定爆炸音量再增强

- 用户确认将当前Alien Cannon爆炸声从VolumeMultiplier=1改为3；只改正式Explosion Cue的该值，不修改音频样本、敌人音量或震屏。检查点b0fb3ec保存前轮音频替换。
- 冷回读发现当前CameraShakeScale实际为8，保持不变（与旧文档3不同）；EnemyHit.VolumeMultiplier保持1。BoostAlienExplosionFinal.log的saved=3/实际shake=8为准，末尾旧固定打印shake=3不作为证据。首次验证错误仅为脚本硬编码shake=3的错误假设，未修改震屏。
- Blueprint在UE编译保存成功，独立回读确认爆炸3/敌人1。回归测试不再把可调震屏值钉死为3，改为检查配置值与距离衰减的关系；Development Editor Win64构建成功。测试见AlienVolume3Final.log。
- 单声源理论峰值会超过0dBFS，可能触发混音限制或失真；这次按用户明确确认保留3倍增益，没有宣称无削波或主观响度正好三倍。
- AlienVolume3Final.log独立D3D启动ExplosionDirectionalShake测试Success；当前只改正式Cue音量及对应测试/文档，无最终Git提交。

## 2026-09-04 指定爆炸/肉体命中音频与震屏增强

- 用户指定Downloads/512565-Alien-Game-Explosion-Robot-Cannon-4-Big-Hard-Impact-Glitchy.wav用于爆炸、166553-Bullet-Hit-Body-Flesh_05.wav用于敌人Hit，并确认增强震屏及删除无用资产。检查点 `4a40cd8` 保存前轮零伤害Cue修复。
- 源音频审计：爆炸6.960秒/96kHz/24bit/stereo，峰值-0.50dBFS、RMS-16.79dBFS；肉体0.565秒/48kHz/24bit/mono，峰值-1.34dBFS、RMS-17.53dBFS。保留原WAV样本，不套旧合成音频3倍增益；两Cue音量倍率1，避免单声源超满幅。
- 爆炸音频新路径 `Weapons/ExplosionGun/Audio/S_ExplosionGun_AlienDetonation`，敌人受击 `Enemy/_Shared/Audio/S_Enemy_FleshHit`；源码WAV跟随各自资产放入项目，Downloads原文件不动。分别配置Explosion Cue.ExplosionSound和默认EnemyHit.ImpactSound。所有武器原Impact/Fire音频保留。
- CameraShakeScale从1提高到3（正式Cue及原生默认），200/1800cm距离范围与0.45秒时长不变，仍保持方位偏向和距离平方衰减。
- 通过完整D3D编辑器Python导入/编译保存，避免无音频commandlet解码问题。旧S_ExplosionGun_Detonation经硬引用确认无使用方后用EditorAssetLibrary删除，旧合成WAV同步删除，两项可从4a40cd8恢复；未删除其他在用VFX/音频。
- Development Editor Win64编译成功；ValidateUserAudio.log冷回读音频路径、时长、两Cue倍率、震屏强度、旧音频不存在及无Redirector，0错误0警告。新素材引用/3倍震屏断言与零伤害/爆炸回归见UserAudioShakeFinal.log。
- 最终UserAudioShakeFinal.log：ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline全部3/3 Success/exit0，D3D运行无音频解码错误。核验两段指定声音资产与3倍震屏配置；原零伤害血花、正伤害、附着倒计时和自动清理仍通过。

## 2026-09-04 零伤害有效命中触发敌人 Cue

- 用户明确确认零伤害命中也出血花，不改变Damage。写入前检查点 `dca1224` 保存前轮3倍爆炸音量/方位震屏。
- ABulletBase在原穿透/重复Hit门禁与GE逻辑之后，仅当Damage==0且有效命中Enemy时，携带真实HitResult显式调用目标ExecuteHitReactionCue(Context,0,true)。SourceASC缺失时使用目标ASC创建Context；不伪造伤害，不新增GE。
- AEnemyBase::ExecuteHitReactionCue新增默认false的bAllowZeroDamageHit。普通Health回调对0/治疗仍不触发；显式零伤害Hit即时InvokeGameplayCueEvent，正伤害仍沿原ExecuteGameplayCue流程，避免一次命中两次血花。Phantom穿透和重复命中仍提前返回。
- BP_ElectricGunBullet.Damage保持0，未改蓝图/伤害资产。Development Editor Win64编译成功。StickyExplosionAndBlood增加正式电击弹的零伤害血花、生命不变、重复Hit与穿透断言，并对原正伤害血花数量严格断言1；验证日志ZeroDamageEnemyHit.log。
- 最终D3D PIE：ZeroDamageEnemyHit.log中StickyExplosionAndBlood、ThreeWeaponBaseline、ExplosionDirectionalShake全部3/3 Success/exit0。确认电击弹伤害0、血花+1、重复Hit不增加、Phantom穿透不增加、Health不变；正伤害仍仅一次血花。无资产改动，无最终Git提交。

## 2026-09-04 爆炸三倍音量与方位震屏

- 用户确认爆炸音量至少原来的3倍，并按相对玩家方位加入震屏。写入前本地检查点 `84aa9cd` 保存上一轮附着弹/爆炸Cue/默认血迹；本轮不自动最终提交。
- `UGCN_ExplosionGunExplosion.VolumeMultiplier` 默认从1改为3，正式Cue继承并通过UE编译保存确认。保留原合成SoundWave，不重复合成/叠播，不改变原命中或开火声。
- 新 `UExplosionCameraShake / UExplosionCameraShakePattern` 位于本枪 `Effects/ExplosionCameraShake.h/.cpp`。0.45秒有限衰减冲击，位移主轴沿爆炸点→相机的UserDefined播放空间，含旋转抖动；不改变ControllerRotation，不替代开火震屏。
- 爆炸Cue配置 `CameraShakeClass / CameraShakeScale / ShakeInnerRadius / ShakeOuterRadius`，默认强度1、200cm以内全强、1800cm以外0、中间平方衰减。遍历本地玩家相机，可见方向由爆炸位置而不是表面法线决定。
- 电击枪不出血花根因已确认：`BP_ElectricGunBullet.Damage=0`，HitEffectClass仍为GE_BulletDamage；默认敌人Hit Cue只响应DamageTaken>0。不是材质缺失。已异步询问用户保留零伤害但命中也出血，还是伤害改5；未得到选择前保持原伤害/触发规则，不擅改。
- `ExplosionAudioElectricAudit.log` UE配置/编译保存0错误0警告；Development Editor Win64编译通过。方向/衰减/自动结束单测与D3D爆炸Cue链回归见 `ExplosionDirectionalShake.log`。
- 最终 `ExplosionDirectionalShake.log`：ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline 3/3 Success/exit0。验证正式Cue音量3、左右爆炸相反偏移、距离倍率、0.45秒自动结束、真实弹体爆炸Cue向本地PlayerCameraManager添加震屏；原5点伤害与特效清理回归仍通过。测试编辑器正常退出。

## 2026-09-04 附着倒计时爆炸与默认敌人血迹

- 用户确认增加附着弹与可调倒计时；明确保留首次伤害和全部已有武器命中表现，本轮爆炸仅表现，不追加范围伤害。追加要求默认 Enemy Hit Cue 提供血迹、短促飙血。写入前检查点 `5f029d4`，结果未最终提交。
- `AExplosionGunBullet : ABulletBase` 位于 `Weapons/ExplosionGun/Bullets`；先执行基类有效命中路径，再停止移动/碰撞并附着命中组件（角色优先细化到骨骼 Mesh）。`BP_ExplosionGunBullet` 改父类并设置 `bDestroyOnHit=false`，`ExplosionDelay=2s`。重复 Hit 不重置倒计时；零秒走下一 Tick；EndPlay 清理 Timer。隐身 Phantom 仍穿透。
- 新 `UGCN_ExplosionGunExplosion` 位于本枪 `GAS/GameplayCues`，由弹体倒计时结束调用 `GameplayCue.Weapon.ExplosionGun.Explosion`；Niagara 按表面法线的 +Z 方向生成，声音/EffectScale/VolumeMultiplier 都在 `GC_Weapon_ExplosionGun_Explosion` 配置。Source ASC 失效时通过 CueManager 直接播放。
- TMIIR `/Game/NiagaraExplosion01/Niagaras/Ground/N_ExplosionGround_006` 和递归依赖共116包经 AssetTools 迁入，整理到 `/Game/Weapons/ExplosionGun/Effects/Explosion`；系统名 `NS_ExplosionGun_Detonation`。迁移前保留并恢复56个材质纹理参数。独立合成低频爆破 WAV `Weapons/ExplosionGun/Audio/S_ExplosionGun_Detonation.wav`，1.4秒/48kHz/mono，与原命中/开火声独立。
- 原默认 `GC_Character_Enemy_Hit` 的 ImpactEffect/ImpactSound/ImpactDecalMaterial 均空。现在 `UGCN_EnemyHit` 自身生成敌人附着血迹及近处环境血迹，并生成0.55秒自动销毁的 `AEnemyBloodSpray`（9张无碰撞、无阴影、面向相机的短促飞溅卡片）。默认血迹12秒，最后2秒淡出；不修改通用武器 ImpactFeedbackBase，不制作金属弹孔。
- 血迹源纹理由 imagegen skill 的内置 image_gen 生成，保留真实 Alpha（采样范围0–255）；源 PNG 与 Unreal Texture 位于 `Enemy/_Shared/Effects/Hit/Textures/T_Enemy_BloodSplatter`，独立 `M_Enemy_BloodSpray`/`M_Enemy_BloodStain`。生成提示词见同目录 `BloodSplatter-generation.md`。
- Development Editor Win64 构建成功；`StickyBloodD3D.log`：`StickyExplosionAndBlood` 和 `ThreeWeaponBaseline` 2/2 Success。PIE 验证初次5点伤害、重复Hit不叠伤、附着跟随、停止碰撞/速度、可调倒计时、0秒/缺少SourceASC路径、延时后子弹销毁及Niagara保留、不追加伤害、真实伤害触发血迹/喷溅与喷溅销毁。
- `ValidateStickyBlood.log` 冷加载验证两项 GameplayCueName 精确匹配正式Tag，递归116包均owner-local、材质纹理参数非空、无Redirector；供应商目录在Registry与磁盘均清空。首次无界面导入发生音频解码ensure/Interchange Slate退出，但资产已保存；独立二次安装0错误0警告，D3D真实音频设备测试无相关错误。定向Fixup提示无包，因为AssetTools已移除旧包，随后仅清理确认空的目录。
- 延长PIE至14秒的清理测试发现源Niagara尾焰仍active，因此Explosion Cue新增可调 `EffectLifeSpan=8s`，通过绑定Niagara组件的weak timer兜底销毁，独立于弹体与发射者。血迹12秒到期销毁断言通过。视觉测试保存 `TMT_StickyExplosion.png`、`TMT_EnemyBloodHit_Isolated.png`，最终清理回归见 `StickyBloodFinal.log`。
- 最终 `StickyBloodFinal.log`：StickyExplosionAndBlood + ThreeWeaponBaseline 2/2 Success，包含14秒时爆炸组件active为0、血迹组件已过期为0的新断言。已查看隔离截图确认腿部深红受击飞溅/血迹及地面红色斑点可见、无方形透明底；爆炸截图保留源橙黄大范围能量闪光。`ValidateStickyBloodFinal2.log` 再次编译保存两项Cue、冷回读Tag与116包依赖成功。没有关卡写入、没有最终Git提交，测试编辑器均正常退出。

## 2026-09-04 爆炸枪高能核心材质

- 用户认可电击枪并要求只换爆炸枪材质，确认“深色金属、橙红能量舱、亮黄脉冲核心、槽线流动”方向。检查点 f3c38f0 保存上一轮两枪材质及灰壳/描边/缩放修复。
- 仅修改 ExplosionGun/Materials/M_ExplosionGun_Surface 与 MI_ExplosionGun_Rifle：侧面双能量舱以局部坐标定位，橙红外层、亮黄中心与环状热能纹理；核心0.65Hz平缓脉冲，槽线亮点以独立0.45速度流动。深色金属替换原黄铜配色，提高粗糙度，限制发光在能量舱/槽线，保留共享显现函数。
- 参数：CorePulseRate、EnergyFlowSpeed、PlasmaShellColor、PlasmaCoreColor、PlasmaIntensity。不修改模型、开火Niagara、灯光、默认尺寸或玩法代码；ElectricGun资产无变动。
- ExplosionEnergySurface.log 材质编辑/参数验证成功0错误0警告；ExplosionEnergyD3D.log 的 SharedEquipReveal、ExplosionVisualCapture、ThreeWeaponBaseline 全部3/3 Success/exit0。已查看 TMT_WeaponSurface_2.png 实机截图并另存 TMT_ExplosionEnergyCore.png，橙红舱体和亮黄核心可辨，原表面显现保持正常。无C++改动，无新增资产/删除。

## 2026-09-04 枪体材质、灰壳与爆炸枪尺寸修复

- 用户确认实施：修复切换时灰模、参考 RepairGun 做两枪科幻材质、删除三枪描边及无引用资产、按 ElectricGun 接通 ExplosionGun 的真实枪口尺寸倍率。写入前检查点 `3be752e` 保存前轮共享装备 VFX 工作。
- 排查发现 Outline 材质无 Amount (S)，旧共享组件会把描边壳替换为通用不透明显现表面；移除该备用替换，MID 只继承原材质，未接溶解契约的材质保持原样。三把当前武器表面均已接入共享溶解函数；新装备仍自动继承组件，但自定义材质需接同一函数才能参与溶解。
- 删除 AFirearm.StaticMeshOverlay 原生组件；三 BP 清空旧引用并重新编译保存，按外部引用检查删除三把枪 Outline mesh 与 M_EquipmentOutline（4项），可从检查点恢复。
- 新建各枪专属 M_ElectricGun_Surface / M_ExplosionGun_Surface：参考 RepairGun 的 PBR 分层但不套用不匹配 UV 的维修枪贴图。使用枪体局部坐标生成面板接缝、分区金属、能量嵌条、微法线和粗糙度变化，配合原有 owner-local SurfaceDetail 纹理。电击青蓝，爆炸黑化金属/黄铜/橙；保留共享显现函数，不使用整枪 Fresnel 发光。
- 原共享 M_EquipmentEquipSurface 不再有两个使用方，迁回 RepairGun/Materials/M_RepairGun_Rifle，共享目录只保留显现函数和噪声。两枪独立材质实例改为各自表面主材质。
- 通过一次性 Editor migration 按 ElectricGun 已验证的 Spawn ScaleSpriteSize 模块模式，为 PhysicalMuzzle 全部9发射器追加 Uniform Scale Factor = User.MuzzleScale；默认参数1，枪体BP倍率2。迁移代码及临时 NiagaraEditor 构建依赖已移除，并重新冷编译成功。
- ValidateWeaponSurfacesFinal.log：三枪可见表面、无描边组件、材质图输出、纹理参数、蓝图编译、Redirector 校验成功0警告。检查过程中对 RepairGun 11 个材质产生的非语义重存已精确恢复检查点版本，不改变维修枪自身材质。
- Development Editor/Win64 最终构建成功；WeaponSurfacesD3DFinal.log 五项 SharedEquipReveal、EquipDissolveEvidence、ExplosionVisualCapture、ThreeWeaponBaseline、ThreeWeaponPIESwitch 全部 Success/exit0。新增 MID.Parent 与原表面精确一致断言、兼容普通装备/不兼容槽保留原材质断言。
- 首轮 ThreeWeaponPIESwitch 因 TestMap 加载卡顿导致固定 wall-clock 等待早于游戏时间显现完成，切枪输入被正确锁定拒绝；测试已改为最多15秒等待实际可见且显现结束后发输入，不改玩法锁。最终通过。
- ExplosionScale1D3D.log 的1倍实际射击与显现证据均 Success；1/2倍对照截图 TMT_ExplosionScale_1.png / _2.png 已查看，2倍闪光、Sprite与折射范围明显增大。参数覆盖仅运行时测试对象，不改 BP 的2倍默认。
- 已查看 TMT_WeaponSurface_1.png / _2.png（电击/爆炸）和 TMT_EquipReveal_2.png：金属分区、青蓝/橙能量嵌条可见，显现时保留同一表面，没有灰壳。既有 M_UE4Man_Body 缺图材质警告仍与本轮无关。结果未最终提交，待用户审核观感。

## 2026-09-04 通用装备显现与默认枪口倍率

- 用户要求所有装备共用爆炸枪同款切换 VFX，但可有各自动画，并将默认 MuzzleEffectScale XYZ 改为 2。已将 AFirearm 原生默认改为 `(2,2,2)`，BP_ExplosionGun 同步覆盖；保留 ElectricGun 当前 2 倍与 RepairGun 专属 0.85 倍。
- 定位到 RepairGun 实际显示的 SkeletalMesh 使用 M_SCFR_BaseMat，不含 Amount (S)，旧代码对它写参数无效；ElectricGun 的 Noise 被枪体 SurfaceDetail 贴图覆盖，导致溶解图案与源效果不同。
- 新增 `Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h/.cpp`，由所有 AEquipmentBase 原生创建。组件持有 0.5 秒 Hermite 1→0 时序、临时 MID、原材质恢复及取消逻辑；只在播放期间 Tick。没有溶解契约的未来自定义材质会在显现期间使用共享备用表面，结束/卸下时恢复原材质；接入共享函数的材质全程保留自身外观。
- AEquipmentBase 只保留 PlayEquipEffect 委托、Equip/Unequip 和动画配置；新增独立 `bPlayEquipAnimation`（默认 false）允许同时播放每件装备自己的 EquipMontage，EquipmentAnimLayerClass 仍独立。切换和首次装备统一由 EquipmentManager.QueueEquipPresentation 延迟一帧等待姿势，然后播放 VFX/可选动画并显示；移除 FPSCharacterBase 的 PlayInitialEquipEffect 与重复入口。切换锁定直接读取组件是否仍播放，不再复制固定时长定时器。
- 将爆炸枪工作正常的主材质、描边材质和函数通过 AssetTools 提升到 `/Game/Weapons/_Shared/Equipment/Effects/Equip/Materials/{M_EquipmentEquipSurface,M_EquipmentOutline,Functions/MF_EquipmentEquipDissolve}`。专用噪声 `Textures/T_EquipmentEquipNoise` 为共享副本，保留仍被枪口/命中使用的原 VFX 噪声。
- ElectricGun / ExplosionGun 的独立材质实例及 RepairGun 静态材质引用共享父材质；RepairGun 实际骨骼材质在原颜色、法线、金属度、纹理和发光图后接入同一个函数。ElectricGun 与 RepairGun 静态 Outline 槽使用共享描边，所有当前表面统一使用共享噪声。
- 删除 4 项已无引用的旧 RepairGun/ElectricGun 父材质与各自溶解函数；清理三把枪空的 Materials/Functions 目录。旧内容可从写入前 WIP checkpoint `2dff0c6` 恢复，结果未提交。
- Development Editor / Win64 构建成功；冷启动验证共享父材质依赖闭包 3 项全部位于共享 Equip 目录、三把枪可见材质均接通共享函数/噪声与继承组件，相关 Blueprint 编译保存通过。D3D 首轮 SharedEquipReveal、ThreeWeaponBaseline、ThreeWeaponPIESwitch、ExplosionVisualCapture 均 Success。三张显现截帧为 `Saved/Screenshots/WindowsEditor/TMT_EquipReveal_0.png`、`_1.png`、`_2.png`。旧 EquipDissolveEvidence 初次因 PIE 启动工作错过首帧采样失败，已改为测试开始时明确启动测量播放，最终 D3D12 SharedEquipReveal、EquipDissolveEvidence、ThreeWeaponBaseline、ThreeWeaponPIESwitch 4/4 Success。

## 2026-09-04 爆炸枪按截图替换为 Physical 1

- 用户明确授权替换模型、特效、光源和清理旧资产。截图 `屏幕截图 2026-09-04 192212.png` 中 `BP_Weapon_Rifle_Physical_Child` 在源 VFXPack 内重定向到 `BP_Weapon_Rifle_Physical_01_Child`。
- 新枪体来自 `SM_Weapon_Ballistics_Rifle_01` 及 Outline，正式命名 `SM_ExplosionGun_Rifle` / `SM_ExplosionGun_Rifle_Outline`。主材质保留源配置，正式命名 `MI_ExplosionGun_Rifle` / `M_ExplosionGun_Rifle`；源蓝图组件额外指定的 `MA_Example_Item_HackyOutline` 单独迁入为 `M_ExplosionGun_Outline`，写入 Outline Mesh 材质槽，消除空材质槽警告。
- 枪口来自 `NE_VFX_Muzzle_Physical_Burst_1`，命中来自 `NE_VFX_Projectile_Impact_Physical_1`，正式路径分别为 `/Game/Weapons/ExplosionGun/Effects/Muzzle/Systems/NS_ExplosionGun_PhysicalMuzzle` 与 `/Game/Weapons/ExplosionGun/Effects/Impact/Systems/NS_ExplosionGun_PhysicalImpact`；黄色环境贴花继续 2.0，弹体和玩法配置保持原值。
- 主模型相对位置 `(0,-16.757669,3.554176)`；枪口按源 MuzzleFlashLoc 相对握把变换为 `(0.000174,41.751608,8.507415)` / Yaw 90。点光源使用源射击强度 300（源组件静态默认 500，开火变量覆盖为 300）、线性颜色 `(1,0.551385,0.147041)`、半径 87.370407、SourceRadius 60、0.1 秒淡出；局部偏移 `(8.851011,-17.618745,2.96144)` 与源灯相对枪体位置一致。枪口倍率 1。
- AssetTools 迁移/重命名 41 项依赖；18 项有效纹理参数在迁移时保留并冷启动验证。递归依赖闭包均位于 ExplosionGun；旧模型与两套 Physical 3 系统的依赖图按外部引用保护子树后删除 43 项，3 项仍被现有资产引用的依赖保留。供应商目录、迁移空目录和范围内 Redirector 均清空。
- 用户 ElectricGun 当前尺寸为 `(2,2,2)`，已保留；写入前本地 WIP checkpoint 为 `f5b8fb5`。既有基线测试同步新资产路径、ExplosionGun 开灯与 ElectricGun 用户倍率；新增真实开火截帧验证消耗弹药、点光开启/淡出与渲染画面。
- Development Editor / Win64 构建成功；冷启动 `EXP_VALIDATE|DONE|closure=41`；D3D12 首轮和补齐 Outline 材质后的最终 ThreeWeaponBaseline、ThreeWeaponPIESwitch、ExplosionVisualCapture 均 3/3 Success；爆炸枪空材质槽警告消失。现存 RepairGun Outline 空槽、M_UE4Man_Body 纹理和 AimIK 警告不属本轮范围。截图：`Saved/Screenshots/WindowsEditor/TMT_ExplosionGun_Physical1.png`。当前 FEAT-080 继续 in_progress，后续爆炸弹玩法另行推进；结果未提交。

## 源资产调研

- 外部项目：`D:\Unreal Projects\UE389_MuzzleSource\VFX Pack - Stylized FPS Muzzle and Impacts Effects 5.1\VFXPack`
- 电击枪来源：`BP_Weapon_SMG_02_child`，模型 `SM_Weapon_SubmachineGun_02`，枪口 `NE_VFX_Muzzle_Energy_Burst_3`，环境命中粒子为空，紫色贴花，尺寸倍率 1.1。
- 爆炸枪来源：`BP_Weapon_Rifle_Physical_02_Child`，模型 `SM_Weapon_Ballistics_Rifle_02`，枪口 `NE_VFX_Muzzle_Physical_Burst_3`，命中 `NE_VFX_Projectile_Impact_Physical_3`，黄色贴花，尺寸倍率 2.0。
- 源项目 15 个武器 Blueprint 均不直接引用 FPS 动画；角色统一使用 `FirstPerson_AnimBP`，投射物武器父类统一使用 `FirstPerson_Recoil_Large_Montage`。
- TheManTest 已有 `AS_MaintenanceWorker_FP_Fire` 与引用它的 `AM_MaintenanceWorker_FP_RecoilLarge`；RepairGun 当前未配置 `FireMontage`。

## 资产所有权

- `/Game/Weapons/ElectricGun/...`
- `/Game/Weapons/ExplosionGun/...`
- 即使依赖内容相同，也为每把武器复制并语义化重命名专属版本，避免后续调参互相影响。
- 不保留供应商目录；移动和重命名通过 Unreal AssetTools 完成。

## 实施记录

- 2026-09-03：用户确认开始实施。FEAT-079 的实际应用验收暂缓并转 `needs_improvement`；已通过自动化的实现以 WIP checkpoint `5a39440` 封存。
- 2026-09-03：通过 Unreal AssetTools 迁移两把枪所需模型、轮廓模型、Niagara、材质、纹理及依赖，并按所有权重命名到 `/Game/Weapons/ElectricGun`（45 个资产）和 `/Game/Weapons/ExplosionGun`（60 个资产）；未迁移源项目角色、武器蓝图或动画。
- 2026-09-03：RepairGun 原第一人称开火序列/蒙太奇和装备蒙太奇归档并重命名为 `AS_RepairGun_FP_Fire`、`AM_RepairGun_FP_Fire`、`AM_RepairGun_FP_Equip`；`BP_RepairGun` 已接通开火蒙太奇。
- 2026-09-03：创建 `BP_ElectricGun`、`BP_ExplosionGun` 及各自的 Bullet、GameplayCue、AnimBP、开火/装备动画、音频、CameraShake 和子弹表现副本。两把枪的开火蒙太奇内部已改为引用各自的开火序列，不再依赖 RepairGun 开火序列。
- 2026-09-03：通用命中 Cue 增加可配置贴花材质、尺寸倍率和生命周期。电击枪使用 Energy Burst 3 枪口、无粒子命中、紫色贴花 1.1；爆炸枪使用 Physical Burst 3 枪口、Physical Impact 3 命中和黄色贴花 2.0。
- 2026-09-03：新增两个原生 GameplayCue Tag 和扫描路径；`BP_MaintenanceWorker.InitialEquipmentClasses` 按 RepairGun、电击枪、爆炸枪顺序配置三把枪。
- 2026-09-03：定向依赖检查发现复制后的两个弹体 StaticMesh 仍引用 RepairGun 弹体材质；已通过 StaticMesh 正式材质接口改为各自 `M_<WeaponName>_Bullet` 并冷回读确认，对 RepairGun 的依赖降为 0。迁移脚本曾强制重存 RepairGun 整个目录，收尾时已精确还原无语义变化的材质、纹理和 Niagara 文件，只保留动画重命名所需引用及 `BP_RepairGun` 配置改动。

## 验证结果

- Development Editor / Win64 构建成功，无新增编译警告。
- RepairGun 与两把新枪的 Weapon/Bullet/Cue/AnimBP/CameraShake Blueprint 均在 Unreal 冷启动命令会话内编译保存并重新加载通过。
- `TheManTest.Player.Weapons.ThreeWeaponBaseline`：Success；核对玩法基线、模型、枪口/命中 VFX、Cue Tag、贴花与独立资产引用。
- `TheManTest.Player.Weapons.ThreeWeaponPIESwitch`：Success；PIE 中按 RepairGun → ElectricGun → ExplosionGun 切换，每把枪均可见，且独立开火蒙太奇可在实时 Arms AnimInstance 启动。
- `TheManTest.Player.CombatHUD.AmmoLifecycle`：Success；既有 RepairGun、GAS 与 HUD 行为未回归。
- 冷启动 Asset Registry 的两个 `GameplayCueName` 与正式 Tag 精确匹配；两把新枪对 RepairGun/供应商目录的定向依赖为 0；范围内 Redirector 为 0；供应商路径和旧角色 Actions 路径在 Asset Registry 及磁盘均不存在。

## 剩余验收

- 自动化使用 NullRHI，尚未验证最终渲染观感。由用户在带渲染窗口的 PIE 中确认三把枪模型握持位置、枪口 VFX、爆炸枪命中粒子以及两种贴花尺寸；确认后可将 FEAT-080 归档为 done。

## 2026-09-03 子弹、材质与命中特效完善

- 用户最新要求覆盖了早先的“子弹无 Mesh”方案：电击枪和爆炸枪均改为拥有独立可见弹体。
- 通过 BlenderMCP 在外部专用工程 `D:\Blender Projects\TheManTestWeaponProjectiles\TheManTestWeaponProjectiles.blend` 制作两个低模弹体，并导出 `SM_ElectricGun_Projectile.fbx`（约 842 面，26.65×8.55×8.55 cm）与 `SM_ExplosionGun_Projectile.fbx`（约 1156 面，21.8×10.33×10.33 cm）。TheManTest 只接收最终 FBX，不包含 Blender 工作文件。
- 为两类弹体生成独立无缝表面纹理，并在各武器目录创建参数化主材质与三组材质实例。电击弹使用深蓝金属、青色导体和紫青发光核心；爆破弹使用黑化金属、黄铜结构和橙色发光核心。枪体材质同步使用相同色彩语言调校，仍沿用原枪体主材质和溶解能力。
- `BP_ElectricGunBullet`、`BP_ExplosionGunBullet` 从 `ARepairGunBullet` 改为直接继承 `ABulletBase`，保留通用伤害、命中 Cue 与销毁流程，但不再误继承 RepairGun 专属泡泡膨胀、减速和危险区抑制行为。旧复制弹体 Mesh/Material 已经通过 Unreal Editor 删除。
- 重新核对 VFXPack 实现后接入电击枪 Energy Impact 3 与爆炸枪 Physical Impact 3。2026-09-04 用户调整反馈分层：武器 Cue 无论命中环境或角色均播放各自同一个 `ImpactEffect`；敌人额外受击表现只由目标自己的 Character Hit Cue 负责。
- 保持现有架构：`UGA_Shoot → AFirearm.MuzzleEffect` 负责枪口 Niagara；`ABulletBase → GameplayCue → UGCN_ImpactFeedbackBase` 负责武器命中表现。`CharacterImpactEffect` 与角色分支已删除，武器贴花仍只生成在环境表面。
- 验证结果：Development Editor / Win64 构建成功；冷启动资产验证输出 `CODEX_FEAT080_VALIDATE|DONE`；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 为 Success；定向依赖扫描输出 `CODEX_FEAT080_DEP|DONE`，未发现 RepairGun 或供应商目录依赖。
- 当前仅剩带渲染窗口的 PIE 观感验收：弹体尺寸/朝向、飞行可读性、枪体材质、枪口 Niagara、统一武器命中 Niagara 与环境贴花尺寸。

## 2026-09-04 Phantom 静止测试 AI

- 为爆炸弹命中与范围逻辑调试创建 Phantom 专属测试行为树 `/Game/Enemy/Humanoid/Phantom/AI/BT_Phantom_TestIdle`，结构为 `Root -> Sequence -> Wait`，Wait 固定为 86400 秒；行为树不包含 MoveTo、攻击或搜索节点。
- 从公共人形 AI Controller 派生资产 `/Game/Enemy/Humanoid/Phantom/AI/BP_Phantom_TestIdleAIController`，其 `BehaviorTree` 指向上述静止树；`BP_Phantom.AIControllerClass` 已切换到该专用测试 Controller，不影响其他人形敌人。
- 为保证 Phantom 即使感知玩家、受击进入 Aim 或收到巡逻配置也不发生位移，`BP_Phantom` 的 `PatrolWalkSpeed`、`CombatWalkSpeed`、`TurnWalkSpeed`、`SearchRushSpeed` 与 CharacterMovement `MaxWalkSpeed` 均临时设为 0。
- Blueprint 在 UE 编辑器内编译保存成功；行为树结构回读为 1 个 Sequence + 1 个 Wait，运行时 `BehaviorTreeComponent` 为 active/running。编辑器重启后的命令行冷加载再次确认测试树、Controller 引用及全部 0 速度配置已持久化。
- NullRHI PIE 中生成 `Codex_PhantomIdleProbe`，确认使用 `BP_Phantom_TestIdleAIController_C`；连续 5 秒位置保持 `(0, 0, 90.15)`、速度保持 `(0, 0, 0)`。测试 Actor 仅存在于 PIE，停止 PIE 后未保存进 TestMap。
- 这是 FEAT-080 的临时命中测试支架；恢复正式 Phantom AI 时需把 `BP_Phantom.AIControllerClass` 改回 `/Game/Enemy/Humanoid/_Shared/AI/BP_HumanoidAIController_C`，并恢复速度 `150/300/50/600` 与 CharacterMovement `MaxWalkSpeed=600`。

## 2026-09-04 EnemyBase 简易血条与临时视角后坐开关

- `AEnemyBase` 新增屏幕空间 `UWidgetComponent`，默认位于角色根节点上方 120cm、尺寸 180×18；原生 `UEnemyHealthBarWidgetBase` 绘制黑色边框、暗红底和亮红填充。所有 EnemyBase 子类（包括 Phantom）自动继承。
- BeginPlay 在初始 GE 后绑定敌人自身 ASC 的 Health/MaxHealth 委托；首次显示与后续受伤均即时刷新，死亡销毁前隐藏。
- `AFirearm` 新增 `bEnableViewRecoil`，公共默认暂时为 false；`UGA_Shoot` 保留 `FireCameraShake` 播放，只跳过改变 Controller Rotation 的 `AddRecoil`。Pitch/Yaw/Damping 原配置完整保留，恢复时把开关改回 true。
- Development Editor / Win64 构建成功。`TheManTest.Enemy.Shared.EnemyBaseHealthBar` 在 NullRHI PIE 中直接生成 `BP_Phantom`，确认继承的屏幕空间组件和原生 Widget 已初始化，并验证 Health 从 100 改为 75 后界面立即同步；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 确认三把枪视角后坐关闭且 CameraShake 资产仍配置，两项测试均为 Success。
- `ABulletBase` 构造函数通过共享资产路径默认加载 `GE_BulletDamage` 作为 HitEffectClass；之后新建普通伤害子弹只需填写 Damage，仍允许特殊子弹覆盖或清空。Development Editor 构建及冷启动 `ThreeWeaponBaseline` 默认类断言为 Success。
- 按用户确认移除 `UGCN_ImpactFeedbackBase.CharacterImpactEffect` 与 Character Niagara 选择分支；电击枪/爆炸枪现在对所有目标分别统一使用 Energy Impact 3/Physical Impact 3，敌人额外反馈交给自身 Character Hit Cue。两个 `NS_*_EnemyImpact` 和 12 个确认无引用的专属材质/纹理已通过 Unreal 资产接口删除；被枪口或环境命中引用的依赖保留。两项 Cue 编译保存、冷启动 `ThreeWeaponBaseline` 与删除资产断言均为 Success。

## 2026-09-04 暗场 VFX 测试地图

- 新建独立地图 `/Game/Maps/VFXTestMap`，不修改既有 `TestMap`。地图使用 22m×14m 的封闭测试房，包含地面、四面墙、天花板和两块不同尺寸的环境命中靶面。
- 新建地图专属哑光材质 `/Game/Maps/VFXTest/Materials/M_VFXTest_Dark`，基础色为深蓝灰、粗糙度 0.92；场景使用 850 强度冷蓝主光与 260 强度暖橙侧光。
- Unbound PostProcess 固定使用 Manual Exposure，Exposure Bias=-1.6、Bloom=1.15、Motion Blur=0，避免自动曝光把暗场提亮，同时让枪口和命中发光更清晰。
- `VFXTest_PlayerStart` 朝向正前方的 `VFXTest_Phantom`；Phantom 沿用专属静止 AI、0 移速和 EnemyBase 通用血条。地图 GameMode 为 `BP_TheManGamemodeBase_C`。
- 冷启动校验确认 13 个预期 Actor、Phantom 位置 `(250,0,96)`、MaxWalkSpeed=0、1 个继承血条组件、正确 GameMode 与 Manual Exposure；MapCheck 为 0 Error / 0 Warning。
- 实际 `-game` 启动确认地图进入 Play、默认玩家 Pawn 与 RepairGun 成功生成；渲染截帧确认深灰房间、冷暖分区、Phantom 和环境靶面均清晰可见。截图保存在 `Saved/Screenshots/WindowsEditor/ScreenShot00002.png`。

## 2026-09-04 电击枪 Laser VFX 替换与地图归档

- 按用户要求从外部资源项目迁入 `/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/VFX/Presets/Muzzle/NE_VFX_Muzzle_Laser_Burst_2` 与 `/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/VFX/Presets/Impacts/NE_VFX_Projectile_Impact_Laser_2`，连同依赖共 51 个包；未迁移源武器蓝图、角色或动画。
- 两个 Niagara System 在目标项目内分别语义化命名为 `/Game/Weapons/ElectricGun/Effects/Muzzle/Systems/NS_ElectricGun_LaserMuzzle` 与 `/Game/Weapons/ElectricGun/Effects/Impact/Systems/NS_ElectricGun_LaserImpact`。`BP_ElectricGun.MuzzleEffect` 和 `GC_Weapon_ElectricGun_Impact.ImpactEffect` 已改为对应新系统。
- 依赖按 ElectricGun 所有权合并/重命名到 `/Game/Weapons/ElectricGun/Effects`；冷启动递归检查覆盖 51 个依赖包，所有 `/Game` 依赖均位于 ElectricGun 目录。迁移期 40 个 Redirector 通过 UE 5.7 `ResavePackages -FixupRedirects` 删除，供应商路径为空。
- 删除被替换的 `NS_ElectricGun_Muzzle`、`NS_ElectricGun_Impact` 及 5 个确认无外部引用的旧专属效果依赖；保留紫色环境贴花、命中音效、CameraShake 与所有仍被新系统或其他电击枪资产引用的资源。
- 将测试地图从 `/Game/Maps/VFXTestMap` 整理到 `/Game/Maps/VFXTest/VFXTestMap`，与 `/Game/Maps/VFXTest/Materials/M_VFXTest_Dark` 同目录归档。冷启动打开新地图确认 13 个预置 Actor、`VFXTest_Phantom` 与 `VFXTest_PlayerStart` 均保留，旧根目录地图不存在。
- `CombatHUDTests.cpp` 的 ThreeWeaponBaseline 硬编码路径同步更新为新 Laser 系统；Blueprint/Cue 已在 Unreal 内编译保存。Development Editor / Win64 构建成功；冷启动校验输出 `CODEX_ELECTRIC_LASER_VALIDATE|DONE|dependencies=51|actors=13`；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 为 Success。

## 2026-09-04 两枪模型互换与 Laser 方形面片修复

- 按用户要求互换电击枪与爆炸枪的主模型和 Outline。最终仍保留 owner-local 语义路径：`SM_ElectricGun` 现承载原 Ballistics Rifle 02 几何体（LOD0 10272 顶点），`SM_ExplosionGun` 现承载原 SMG 02 几何体（LOD0 8706 顶点），未建立跨武器目录引用。
- 与几何体绑定的 StaticMesh 相对位置及 `MuzzleLocalTransform` 同步交换；电击枪使用原步枪握持/枪口位置，爆炸枪使用原 SMG 握持/枪口位置。枪体材质没有跟随来源交叉引用，仍分别使用 `MI_ElectricGun` 青蓝主题与 `MI_ExplosionGun` 橙色主题。
- 排查原工程和目标工程的 Laser 材质后确认：主材质均为 Translucent/Surface/TwoSided，问题不是 Niagara 加载或 BlendMode；目标工程有 11 个材质实例的 `Main_Texture` 参数为空，而源工程对应参数均有效，导致白色默认纹理把 Niagara Sprite 卡片显示成明显方块。
- 已恢复 LensFlare 1 项、Lightning 5 项、MuzzleFlash 1 项、Smoke 2 项、Fire 1 项与 Rocks 1 项，共 11 个 owner-local 贴图引用。冷启动精确回读 11/11 材质参数、两把枪主模型/Outline/材质/握持/枪口位置及两个 Laser System 均通过，临时交换目录在 Asset Registry 与磁盘均不存在。
- Blueprint 在 UE 内重新编译保存；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 与 `TheManTest.Player.Weapons.ThreeWeaponPIESwitch` 在 NullRHI 和真实 D3D 渲染设备下均为 Success，未出现 Material、Niagara、Shader 或 D3D 错误；定向依赖扫描为 DONE。写入前的地图归档和 Laser 初次迁移结果已封存于 WIP checkpoint `6549004`；本轮结果等待用户明确要求后再更新 Git。

## 2026-09-04 电击枪开火点光增强

- 以 `D:\ROG\Videos\EV录屏\20260904_133033.mp4` 为视觉验收基准逐帧检查：原版青绿色枪体反光从约 6.083s 开始，最强位于 6.100–6.117s，约 3–5 帧后衰减；表现核心是枪身瞬时变为高亮青绿色，而不是整间环境持续变绿。源武器使用同一 Laser Burst 2；`Weapon_Idle_Particle` 不是本次开火反光来源。
- `AFirearm` 的默认关闭 PointLight 增加 `SourceRadius` 与枪口局部 `LocalOffset`，组件对齐源设置：Unitless、Inverse Square、FalloffExponent=8、Cast Shadows、Affect Translucent Lighting。颜色修正为源资产真实线性值 `(0.075319,1,0.652928)`；定时器改为 0.1 秒线性淡出，快速连射重置计时，`Unequip` 立即清灯。
- 两枪模型互换后，电击枪当前 `MuzzleLocalTransform` 坐标轴与源展示蓝图不同，不能直接复用源偏移。最终把枪口位置前移到源枪管位置 `(0.000751,67.696307,6.434297)`，并用当前坐标系反算灯位偏移 `(1.480382,-6.734961,-15.577805)`；Niagara 保持 CameraForward 的 0°附加旋转，90°会把方向性光束错误横扫屏幕。
- `BP_ElectricGun` 最终设置 `MuzzleEffectScale=(1,1,1)`、点光 Intensity=1800、AttenuationRadius=200、SourceRadius=60、Duration=0.1s。源 UE 5.1 的 600 在当前 UE 5.7/枪体材质/曝光链下实际画面只有参考视频约三分之一亮度，因此实例强度按视觉标准补偿；RepairGun 与 ExplosionGun 继续继承默认关闭。
- `/Game/Maps/VFXTest/VFXTestMap` 的 Bloom 临时从 1.15 调为 4.0、Threshold=0.5，以对齐源 MainScene 的审核环境；不影响其他地图。新增 `ElectricMuzzleVisualCapture`，预热 Niagara 后走真实 `PrimaryFire` 并直接读取 PIE 游戏视口，证据截图为 `Saved/Screenshots/WindowsEditor/TMT_ElectricMuzzle_VideoStandard.png`，截帧时点光强度 1500。
- Development Editor / Win64 构建成功。NullRHI `ThreeWeaponBaseline` 与 `ThreeWeaponPIESwitch` 2/2 Success；D3D12/SM6 `ElectricMuzzleVisualCapture` Success。D3D 日志仍仅报告项目既有 `M_UE4Man_Body` 缺失输入纹理与 AimIK 警告，与本轮武器改动无关。

## 2026-09-04 枪口 VFX 与点光统一尺寸倍率

- `MuzzleEffectScale` 现在是每把枪统一的枪口尺寸入口。`UGA_Shoot` 创建 Niagara Component 后先写入 float `User.MuzzleScale`，再激活系统，避免首帧沿用默认倍率。
- `/Game/Weapons/ElectricGun/Effects/Muzzle/Systems/NS_ElectricGun_LaserMuzzle` 已暴露 `User.MuzzleScale`，16 个 emitter 均在 Particle Spawn 阶段通过 Uniform `ScaleSpriteSize` 模块缩放 SpriteSize；倍率 1 保持源外观，倍率 3 的实际 D3D 截图确认闪光、光晕与 Sprite 同步增大。
- 点光尺寸使用同一个倍率：`AttenuationRadius` 与 `SourceRadius` 乘以 `MuzzleEffectScale` 最大绝对轴，Intensity、Color、Duration 与 LocalOffset 不变。蓝图建议 XYZ 填相同值；`BP_ElectricGun` 默认已恢复 `(1,1,1)`。
- Development Editor / Win64 构建成功。NullRHI `ThreeWeaponBaseline` 与 `ThreeWeaponPIESwitch` 2/2 Success；D3D12/SM6 `ElectricMuzzleVisualCapture` 在 1x 与 3x 均为 Success。1x 实测点光半径为 200/60，3x 为 600/180；证据图为 `Saved/Codex/MuzzleSpawnModuleScale_1.png`、`Saved/Codex/MuzzleSpawnModuleScale_3.png` 与最终 `Saved/Codex/MuzzleScaleFinal_1.png`。
