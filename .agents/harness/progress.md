# 当前进度

## Active Feature

- FEAT-080：三枪独立表现，in_progress；本轮仅爆炸枪高能核心材质调整完成，电击枪获用户认可并保持不动。
- 历史与验证详情：archive/FEAT-080-three-weapon-setup.md。

## 当前配置

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

- 最新 ExplosionEnergySurface.log 材质编辑验证成功0错误0警告；ExplosionEnergyD3D.log：SharedEquipReveal、ExplosionVisualCapture、ThreeWeaponBaseline 3/3 Success/exit0。实机截图 TMT_ExplosionEnergyCore.png。本轮无C++改动，仅两项爆炸枪材质资产改变。

- Development Editor / Win64 构建成功；ValidateWeaponSurfacesFinal.log 冷回读、BP编译、材质/引用检查成功0警告。
- WeaponSurfacesD3DFinal.log：SharedEquipReveal、EquipDissolveEvidence、ExplosionVisualCapture、ThreeWeaponBaseline、ThreeWeaponPIESwitch 5/5 Success。
- ExplosionScale1D3D.log：1倍开火与显现证据2/2 Success；BP默认仍2。1/2倍对照已查看，粒子尺寸实际改变。
- 截图 Saved/Screenshots/WindowsEditor/TMT_WeaponSurface_1.png、_2.png；TMT_EquipReveal_0~2.png；TMT_ExplosionScale_1.png、_2.png。
- 删除4项描边资产，可从本地检查点3be752e恢复；RepairGun11项非语义重存已撤回，不误改原材质。

## 当前待办 / 会话交接

- 用户可在测试地图审核爆炸枪新能量核心材质；电击枪已认可，不再改动。后续继续爆炸弹玩法逻辑。FEAT-080未整体归档。
- 本轮结果不自动提交，等待用户明确要求“更新 Git”；未push。
- 最新检查点 f3c38f0 封存前轮两枪材质/灰壳/描边/缩放改动；本轮两项爆炸枪材质未最终提交。一次性Niagara迁移代码及NiagaraEditor临时依赖已清除，最终源码不含迁移入口。
- 切枪回归测试已改为等待实际显现结束（最多15秒），避免加载卡顿期间固定wall-clock等待过早发送切枪输入；运行时切换锁没有改动。
- 既有 M_UE4Man_Body 缺失纹理和AimIK警告仍存在，不属本轮枪体材质。
