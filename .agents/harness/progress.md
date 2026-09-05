# 当前进度

## Active Feature

- FEAT-080，in_progress。2026-09-05本轮条件子弹时间与Enemy Air007已实现并通过自验；整体三枪功能保留用户手感验收，不关闭。
- 用户确认：只有本次爆炸实际炸碎Chaos或击杀Enemy才触发子弹时间；Enemy身上爆炸使用TMIIR地图的N_ExplosionAir_007。
- 详细历史见archive/FEAT-080-three-weapon-setup.md。

## 最新行为与验收入口

1. 打开Maps/VFXTest/VFXTestMap，爆炸枪打Enemy：首次5点伤害与2秒倒计时保留，延迟爆炸20点/400cm。仅受伤不减速；本次爆炸击杀Enemy才减速。
2. 打地面或Chaos Cube：没有新破坏不减速；真正炸碎触发一次。炸不碎、对已散落碎块施加冲量不算新破坏；击杀和破碎同时发生也只请求一次。
3. Enemy爆炸使用NS_ExplosionGun_EnemyDetonation（TMIIR N_ExplosionAir_007），在身体实际爆点播放，不再投射到脚下。地面仍NS_ExplosionGun_Detonation（Ground006）。独立Enemy能量音效、环境Alien Cannon音效和原震屏不变。
4. Phantom方向性上半身Control Rig与部位反应保留；墙后或范围外不受范围伤害。死亡仍即时销毁，没有新增死亡动画/击退。

## 调参位置

- BP_ExplosionGunBullet → Bullet|Explosion|Bullet Time：保留用户TimeScale=.05、SlowIn=.01、Hold=1、Recovery=.01真实秒、Inner/Outer=200/1500cm。原生默认仍.2/.05/.08/.25。
- GC_Weapon_ExplosionGun_Explosion：EnemyExplosionEffect指向Effects/EnemyExplosion/Systems/NS_ExplosionGun_EnemyDetonation；EnemyEffectOnGround=false、EnemyEffectScale=1。CameraShakeScale=6、环境VolumeMultiplier=3保留。
- BP_Phantom的ExplosionHitReaction组件：Enabled、MaxAngleDegrees=22、AttackDuration=.055、RecoveryDuration=.55游戏秒。专属CR/后处理ABP位于Phantom/Animations/ControlRig；原动画与根/腿/胶囊保持原行为。

## 实现与验证

- ExplosionGunBullet根据本次范围GE前后Health判击杀；Chaos提交Strain之前由本枪ExplosionOutcomeSubsystem监听受影响组件OnChaosBreakEvent，0.2游戏秒窗口、组件/半径筛选、同次爆炸共享一次结果。弹体销毁不影响异步结果；超时/结束移除订阅并恢复组件原通知设置。
- Development Editor Win64构建成功。ExplosionOutcomeFirst.log：ExplosionOutcomeBulletTime Success（九场景：空地、非致死、致死、抗破坏、真破坏、碎块再击、两者同时、墙后、禁用）。实际Chaos与Health状态、单次启动、速度恢复、通知恢复均通过。
- ExplosionOutcomeRegression.log六项6/6 Success：BulletTimeAndPain、ExplosionChaosGround、ExplosionDirectionalShake、ExplosionRadialDamage、StickyExplosionAndBlood、ThreeWeaponBaseline；ExplosionOutcomeRigRegression.log的EnemyExplosionControlRig也Success。
- ValidateEnemyAirCold.log：AIR_COLD_OK，115包闭包全部在本枪EnemyExplosion目录，依赖可加载、无Redirector、供应商磁盘目录不存在；两个VFX/声音槽、身体爆点开关和用户参数冷回读通过。Blueprint已在UE打开/编译/保存。
- 已查看Saved/Screenshots/WindowsEditor/TMT_StickyExplosion.png，实际Enemy空中火花/烟雾可见。
- 安装保存后的Python关闭编辑器调用发现close_all_asset_editors未暴露，已改为close_all_editors_for_asset；独立冷回读与PIE证明资产保存有效。既有M_UE4Man_Body缺纹理/AimIK警告未处理。

## 会话交接

- 写前选择性检查点4f15ed0保存上轮Control Rig/独立音效工作；本轮结果未最终Git提交/push。
- 地图、TestMap External Actor及用户新增3/YJ、6/26、B/CV、B/ED、C/ZF目录不纳入本轮提交，禁止撤销或全量提交。源TMIIR只迁移最终Niagara与表现依赖，无地图/角色/动画迁入。
- Scripts/VFX/migrate_enemy_air_explosion.py在TMIIR执行；install_enemy_air_explosion.py在目标整理接入；validate_enemy_air_explosion.py冷只读验证。旧Scripts/Audio/configure_enemy_explosion.py已同步Air007与关闭Ground投射，避免重跑还原旧配置。
- 上轮自验后关机是上一轮授权，本轮未执行关机。
