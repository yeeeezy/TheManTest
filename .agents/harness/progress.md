# 当前进度

## Active Feature

- FEAT-080：三枪独立表现，in_progress；本轮爆炸弹附着/可调倒计时/独立爆炸Cue与默认敌人血迹已接入，通过D3D PIE功能回归。电击枪材质保持不动。
- 历史与验证详情：archive/FEAT-080-three-weapon-setup.md。

## 当前配置

- 爆炸音量倍率现为3.0；Explosion Cue新增0.45秒方位震屏，200cm内全强，1800cm外不震。配置CameraShakeClass/CameraShakeScale/ShakeInnerRadius/ShakeOuterRadius。

- 爆炸弹 `BP_ExplosionGunBullet` 继承 `AExplosionGunBullet`，`Bullet|Explosion / ExplosionDelay=2s`，可调。原首次5点伤害及PhysicalImpact不变；附着后停止碰撞/移动，倒计时结束仅播放爆炸，不追加伤害。
- 爆炸表现与音效配置于 `ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion`；系统 `Effects/Explosion/Systems/NS_ExplosionGun_Detonation` 来自TMIIR N_ExplosionGround_006；独立Audio/S_ExplosionGun_Detonation。EffectLifeSpan默认8秒兜底清理长尾焰（源效果在14秒仍active，已加weak Timer避免积累）。
- 敌人默认 `GC_Character_Enemy_Hit` 配置 BloodSprayMaterial / BloodStainMaterial / BloodScale / BloodStainLifeSpan；血迹默认12秒并淡出，飙血0.55秒后销毁。资产在Enemy/_Shared/Effects/Hit，不改变各枪原命中Cue。

- ElectricGun：Ballistics Rifle 02，LaserMuzzle/LaserImpact；专属 M_ElectricGun_Surface + MI_ElectricGun，深色金属、青蓝能量嵌条。
- ExplosionGun：Ballistics Rifle 01，PhysicalMuzzle/PhysicalImpact；专属 M_ExplosionGun_Surface + MI_ExplosionGun_Rifle，深色金属、橙红能量舱、亮黄核心、0.65Hz脉冲及槽线流动。参数 CorePulseRate / EnergyFlowSpeed / PlasmaShellColor / PlasmaCoreColor / PlasmaIntensity。
- MuzzleEffectScale 原生默认与两把新枪均 XYZ=2，RepairGun 保留0.85。ElectricGun16个/ExplosionGun9个发射器的 Spawn ScaleSpriteSize 都读取 User.MuzzleScale；点光范围也跟随倍率。
- 三枪描边壳、共享描边材质已删除；AFirearm.StaticMeshOverlay 已移除。
- 切装备由 EquipmentManager.QueueEquipPresentation 统一调用 EquipmentBase.PlayEquipEffect → EquipmentEquipEffectComponent；0.5秒，MID只继承原材质，不再临时换成灰色备用表面。
- 共享目录 Weapons/_Shared/Equipment/Effects/Equip 只保留溶解函数和噪声；新自定义材质要接该函数及 Amount (S) 才参与显现，未接的槽保持原材质。
- 原共享表面仅剩 RepairGun 使用，迁回 RepairGun/Materials/M_RepairGun_Rifle；维修枪实际骨骼材质保持原样。
- 各装备 bPlayEquipAnimation 默认false，可独立选择 EquipMontage / EquipmentAnimLayerClass。
- VFXTestMap 路径 /Game/Maps/VFXTest/VFXTestMap；静止Phantom、敌人血条及临时关闭视角后坐配置保留。

## 最新验证

- 最新ZeroDamageEnemyHit.log：D3D PIE三项StickyExplosionAndBlood、ThreeWeaponBaseline、ExplosionDirectionalShake 3/3 Success。电击弹零伤害有效Hit出一次血花，重复/穿透不出额外血花，Health不变，正伤害血花不重复。Development Editor Win64编译通过。

- 本轮最终 `ExplosionDirectionalShake.log`：ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline 3/3 Success/exit0；三倍音量、左右方位、远近衰减、自动结束、真实爆炸Cue触发本地震屏均通过，原伤害/清理保持。Development Editor Win64编译成功。

- 最终 `StickyBloodFinal.log`：StickyExplosionAndBlood + ThreeWeaponBaseline 2/2 Success，含爆炸/血迹14秒后均清理断言；`ValidateStickyBloodFinal2.log` Cue编译保存与116包引用复验成功。已查看截图 `TMT_StickyExplosion.png` 和 `TMT_EnemyBloodHit_Isolated.png`。测试编辑器正常退出，未写入关卡。

- 最新Development Editor Win64构建成功；`StickyBloodD3D.log` 的StickyExplosionAndBlood、ThreeWeaponBaseline 2/2 Success：附着跟随、停止移动/碰撞、5点初次伤害、重复Hit、无二次伤害、0秒/缺失SourceASC、Phantom穿透、默认敌人血迹/喷溅及销毁均通过。
- `ValidateStickyBlood.log` 冷加载：两项GameplayCueName匹配正式Tag，116包Explosion依赖owner-local，所有材质纹理参数有效，Registry/磁盘均无NiagaraExplosion01、无Redirector。仅一个Python旧接口弃用警告。

- 最新 ExplosionEnergySurface.log 材质编辑验证成功0错误0警告；ExplosionEnergyD3D.log：SharedEquipReveal、ExplosionVisualCapture、ThreeWeaponBaseline 3/3 Success/exit0。实机截图 TMT_ExplosionEnergyCore.png。本轮无C++改动，仅两项爆炸枪材质资产改变。

- Development Editor / Win64 构建成功；ValidateWeaponSurfacesFinal.log 冷回读、BP编译、材质/引用检查成功0警告。
- WeaponSurfacesD3DFinal.log：SharedEquipReveal、EquipDissolveEvidence、ExplosionVisualCapture、ThreeWeaponBaseline、ThreeWeaponPIESwitch 5/5 Success。
- ExplosionScale1D3D.log：1倍开火与显现证据2/2 Success；BP默认仍2。1/2倍对照已查看，粒子尺寸实际改变。
- 截图 Saved/Screenshots/WindowsEditor/TMT_WeaponSurface_1.png、_2.png；TMT_EquipReveal_0~2.png；TMT_ExplosionScale_1.png、_2.png。
- 删除4项描边资产，可从本地检查点3be752e恢复；RepairGun11项非语义重存已撤回，不误改原材质。

## 当前待办 / 会话交接

- 本轮用户确认爆炸三倍音量+按方位震屏，已实现并编译，验证见ExplosionDirectionalShake.log。检查点84aa9cd保存前轮结果，本轮未最终提交。
- 用户已确认零伤害也触发血花：ABulletBase有效零伤害首次Hit显式通知敌人Cue，Damage仍为0。正伤害走原Health回调，穿透/重复Hit不触发额外Cue。最新检查点dca1224保存前轮爆炸音量/震屏，当前代码未最终提交；验证日志ZeroDamageEnemyHit.log。

- 用户可在VFXTestMap审核附着弹、2秒延时爆炸和敌人血迹的最终观感。当前明确只保留第一次伤害，后续范围伤害另行确认，FEAT-080未整体归档。
- 本轮结果不自动提交，等待用户明确要求“更新 Git”；未push。
- 最新检查点5f029d4封存前轮爆炸枪高能材质；本轮附着弹/爆炸Cue/敌人血迹未最终提交。迁移脚本install_sticky_blood.py和validate_sticky_blood.py在Saved/Codex；源码不含迁移入口。首次无界面导入退出后已冷验证资产成功保存，第二次安装0错误0警告，D3D无相关导入错误。
- 切枪回归测试已改为等待实际显现结束（最多15秒），避免加载卡顿期间固定wall-clock等待过早发送切枪输入；运行时切换锁没有改动。
- 既有 M_UE4Man_Body 缺失纹理和AimIK警告仍存在，不属本轮枪体材质。
