# 音效接入规范

## 职责与所有权

- Gameplay Cue不是Sound Cue。前者决定触发时机/命中位置/目标分工，后者封装音频素材、随机和声音配置。
- 新增一次性玩法音效必须使用owner-local `Audio/SCue_<Owner>_<Purpose>`；源Wave保留原样作为叶节点，不让Gameplay直接引用裸Wave。
- 单素材用Wave Player→Modulator→Output；多素材先通过Random（无放回）选择，再Modulator。不要复制相同素材冒充音频变体。
- 随机只在一层实现，C++/Blueprint/AnimNotify调用端不再追加随机音高/音量。基础音量倍率仍可调。
- UI/扫描识别音/对白/音乐/循环音允许例外，但必须记录原因；不得将长循环简单套随机音高导致变调或循环接缝。
- 音效留在具体武器/敌人Audio目录。跨系统共享的命中衰减放Core/_Shared，三枪共用枪声/机械声配置放Weapons/_Shared；只有爆炸枪使用的爆炸配置留在ExplosionGun。

## 当前参数

| 用途 | 音高 | 音量 | 衰减资产 / 全量半径 + 衰减距离 |
|---|---|---|---|
| 开火（含TestGun） | 0.96–1.04 | 0.95–1.00 | Weapons/_Shared/Audio/SA_WeaponFire / 250 + 4750cm |
| 空仓机械声 | 0.97–1.03 | 0.95–1.00 | Weapons/_Shared/Audio/SA_WeaponMechanical / 100 + 900cm |
| 武器环境命中 | 0.95–1.05 | 0.95–1.00 | Core/_Shared/Audio/SA_ProjectileImpact / 180 + 2200cm |
| 肉体受击 | 0.92–1.08 | 0.95–1.00 | 同上 |
| 爆炸 | 0.96–1.04 | 0.95–1.00 | ExplosionGun/Audio/SA_ExplosionGun_Detonation / 300 + 5700cm |

- 均为球形、线性、3D空间定位，StereoSpread=0；未额外启用遮挡。Sound Cue.VolumeMultiplier固定1，避免引擎默认0.75意外压低声音。
- 并发上限：开火16、机械8、环境命中12、肉体8、爆炸8；StopQuietest，防止连发无限叠加。
- 用户Gameplay倍率保留：Enemy Hit音量5，Explosion音量3、震屏8。肉体Sound Cue路由到既有SMX_EnemyFleshHit限幅总线，不能丢失这层路由。
- Infiltrator扫描音为稳定启停识别音，暂保留裸Wave且不随机。RepairGun旧Equip与模板Fire Wave没有引用，未凭空增加切枪/换弹播放行为。

## 命中声音分工

- GCN_ImpactFeedbackBase::ShouldPlayImpactSound：命中ACharacter不创建武器声音，环境正常；Niagara保持所有目标相同、武器贴花仍只落环境。
- GCN_EnemyHit重写上述策略，自身Hit Cue负责肉体声。禁止因目标没有配置Hit声就回退播放武器环境声。
- PitchVariation/CharacterSoundMultiplier旧字段已移除。ImpactAttenuation/ImpactConcurrency保留为显式调用覆盖，与Sound Cue所选资产一致，不是第二层衰减/并发叠乘。

## 新增音效必做检查

1. 确定用途/所有者，建立Sound Cue与正确源Wave，按用途设置轻微随机或登记例外。
2. 选择对应用途的衰减及并发；需要不同传播范围时独立配置，禁止全项目套一个半径。
3. 更新实际消费者引用；Blueprint打开/编译/保存，检查AnimNotify等入口不重复播放或随机。
4. 冷加载验证Cue图连接、源Wave、衰减/并发和消费者；PIE确认连续播放有细微变化，世界声有左右/远近差异。
5. 将正式新增消费者加入Scripts/Audio/configure_audio_cues.py清单或等价的专属验证测试，不能只靠记忆。

## 工具与验收入口

- Core/Editor/TheManAudioAssetLibrary.InitializeVariationCue：仅初始化空Sound Cue，不覆盖已编辑图；标准UE节点在运行时执行，没有新增全局播放Subsystem。
- Scripts/Audio/configure_audio_cues.py：显式12项现用音效消费者接入；`-AudioValidateOnly`只读冷验证。该脚本不是全项目任意Wave自动覆盖工具。
- TheManTest.Audio.AssetVariationPolicy：12个图/源Wave/参数/并发/衰减、未来多素材及禁止覆盖测试。
- TheManTest.Player.Weapons.SpatialImpactFeedback：实际PIE命中声分工和左右/远近录音；StickyExplosionAndBlood验证真实角色Hit声及既有伤害/血迹。
