# 当前进度

## Active Feature

- FEAT-080，in_progress。2026-09-05夜间Enemy爆炸、Control Rig受击与独立声音已完成实现和自动验收；用户明日验收手感，整体功能不关闭。
- 详细历史见archive/FEAT-080-three-weapon-setup.md。本轮用户明确授权自验、记录、保存后正常关机。

## 明日验收入口

1. 打开Maps/VFXTest/VFXTestMap，使用爆炸枪向Phantom不同位置射击，等待附着倒计时结束。观察上身朝冲击方向偏转再回弹，脚和角色位置不被Rig移动。
2. 分别试胸口、左/右臂和头部；再在敌人附近地面引爆，确认受范围伤害时也有方向反应。墙后或范围外不触发。
3. Enemy爆炸使用指定N_ExplosionGround_006的本项目版本，投射到已标记地面，声音是新生成的能量爆破；地面爆炸保留原Alien Cannon声音。没有合格地面时跳过大地面特效，不把贴花挂在身体上。
4. 敌人血量仍100、每次初击5/爆炸20，连续多次后会直接销毁；死亡动画/击退本轮没有实现。测试受击时用存活敌人，必要时重启PIE。

## 调参位置

- BP_Phantom的原生ExplosionHitReaction组件：Enabled、MaxAngleDegrees=22、AttackDuration=.055、RecoveryDuration=.55游戏秒（会随子弹时间变慢）。
- Phantom/Animations/ControlRig/CR_Phantom_ExplosionReaction：原生Enemy Directional Hit Reaction节点。配套ABP_Phantom_ExplosionReaction继承UEnemyHitReactionAnimInstance，通过Mesh.PostProcessAnimBlueprint叠在原动画末端。
- GC_Weapon_ExplosionGun_Explosion：EnemyExplosionEffect/EnemyEffectScale/EnemyEffectOnGround、EnemyExplosionSound/EnemyVolumeMultiplier独立配置；当前地面效果开关true，声音为SCue_ExplosionGun_EnemyDetonation。
- 用户最新值已保留：BP_ExplosionGunBullet BulletTime倍率.05、SlowIn.01、Hold1、Recovery.01真实秒、200/1500cm；爆炸GC震屏6、环境音量3。原生BulletTime默认仍.2/.05/.08/.25，不覆盖用户BP。

## 已完成

- Enemy/Humanoid/Animation下EnemyHitReactionComponent保存方向/部位/包络，EnemyHitReactionAnimInstance在游戏线程采样并传给Rig；FRigUnit_EnemyHitReaction分配spine_01/02/03旋转，手臂/头部附加响应。根/腿不改，不在RigVM线程访问Actor。
- ExplosionGunBullet保留附着骨骼/Actor弱引用/入射方向；原范围伤害通过可见性筛选并结算后向存活敌人请求受击。近零方向用入射方向兜底，墙后不触发。
- Phantom专属Rig和后处理AnimBP归Phantom/Animations/ControlRig；现役OriginalRifle/Meshes/SK_Mannequin已接后处理。原共享ABP_HumanoidEnemy、ABP_Phantom_OriginalRifle图和AI/locomotion不改，没有动画重定向。
- 指定N_ExplosionGround_006已复用当前NS_ExplosionGun_Detonation，无再次迁移/复制116包。EnemyEffectOnGround=true，地面Hit只用于视觉，声震/物理仍真实爆点。
- 独立程序合成S_ExplosionGun_EnemyDetonation：1.3秒/48kHz/mono，Sound Cue pitch .95~1.05/volume .95~1，复用本枪爆炸衰减/并发。原Alien Cannon、肉体5倍和痛呼1倍不变。脚本Scripts/Audio/synthesize_enemy_detonation.py保存生成方法。
- 原Fuse2、首次Damage5、延迟范围20/400cm/Enemy去重/墙体遮挡、Chaos和BulletTime不变。三枪材质/枪口/血迹/切换表现不改。

## 验证证据

- Development Editor Win64最终成功；Blueprint、Control Rig、后处理AnimBP在UE打开/编译/保存。
- Saved/Logs/EnemyExplosionRigFinal.log：7/7 Success，exit0。EnemyExplosionControlRig、ExplosionRadialDamage、BulletTimeAndPain、StickyExplosionAndBlood、ExplosionChaosGround、ExplosionDirectionalShake、ThreeWeaponBaseline。
- 断言覆盖四方向实际骨骼位移、脚/胶囊不动、完整回原Pose、左臂局部反应、真实爆炸触发/墙后不触发；新Enemy音效实际播放、Niagara在地面落点；原伤害/血花/痛呼/倒计时/Chaos/震屏和时间恢复。
- Saved/Logs/ValidateEnemyExplosionCold.log：ENEMY_EXPLOSION_ASSETS_OK cold，新资产/源音频/依赖/PostProcess/Rig/随机/3D/并发、无Redirector通过。工厂临时资产磁盘不存在。
- 已查看Saved/Screenshots/WindowsEditor/TMT_EnemyRig_Directions.png，Before图同目录，可对照不同方向。最终手感由用户明日确认。
- 首次创建变量需用/Script/CoreUObject.Vector而非FVector，已修复；安装成功后的Control Rig编辑器Slate退出曾崩溃，独立冷回读和两轮PIE均正常完成，未把崩溃当验收。已在安装脚本退出前关闭资产编辑器。
- 既有M_UE4Man_Body缺纹理和AimIK警告未处理。

## 会话交接与关机

- 本轮写前任务范围干净，14d9138已在origin/main作为恢复点；本轮新增/修改已保存磁盘，但未自动Git提交/push。
- 地图及TestMap外部Actor不在本轮写入：65个旧地形标签包、用户新增3/YJ、6/26、B/CV、B/ED、C/ZF目录保留。禁止撤销或全量提交这些内容。
- Scripts/Audio/configure_enemy_explosion.py支持-EnemyExplosionValidateOnly冷只读；通用音频验证不再把可调BulletTime/震屏钉死旧值。
- 用户授权：助手完成自验和记录后正常关机，不强制关闭有未保存内容的程序。关机命令结果另见Saved/Logs/EnemyExplosionShutdown.log（若存在）；如果其他程序阻止正常关机，保留电脑运行，不强杀。
