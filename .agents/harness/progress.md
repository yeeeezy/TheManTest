# 当前进度

## Active Feature

- FEAT-080：RepairGun、电击枪与爆炸枪统一动画和独立 VFX，状态 in_progress。
- 当前任务已完成：爆炸枪 Physical 1 资产替换，以及所有装备共用切换显现 VFX、独立可选动画和默认枪口倍率 2。
- 详细历史：archive/FEAT-080-three-weapon-setup.md。

## 当前配置

- ExplosionGun：Ballistics Rifle 01、Physical Burst 1 / Impact 1、黄色环境贴花 2.0；源橙黄色点光强度 300、基础半径 87.370407、SourceRadius 60、0.1 秒淡出。
- MuzzleEffectScale：原生默认 XYZ=2，ExplosionGun=2，ElectricGun=2，RepairGun 专属 0.85。灯光范围仍随倍率变化。
- 切装备 VFX：共享 UEquipmentEquipEffectComponent，0.5 秒溶解显现；首装/切换入口统一在 EquipmentManager。所有 EquipmentBase 自动继承；效果结束或取消恢复原材质。
- 共享材质/函数/噪声位于 /Game/Weapons/_Shared/Equipment/Effects/Equip；三枪保留各自表面材质实例和动画。
- bPlayEquipAnimation 默认 false，可为任意装备开启并指定自己的 EquipMontage；EquipmentAnimLayerClass 保持独立。
- VFXTestMap 位于 /Game/Maps/VFXTest/VFXTestMap，Phantom 为静止命中目标，EnemyBase 血条与无视角后坐测试配置保留。

## 验证和清理

- Development Editor / Win64 构建成功；相关蓝图编译保存和共享依赖冷启动检查通过。
- D3D12 SharedEquipReveal、EquipDissolveEvidence、ThreeWeaponBaseline、ThreeWeaponPIESwitch 最终 4/4 Success；本轮 ExplosionVisualCapture 也已通过。
- 三枪显现截图：Saved/Screenshots/WindowsEditor/TMT_EquipReveal_0.png、_1.png、_2.png。
- 本轮删除 4 项无引用旧父材质/溶解函数并清理空目录；原文件可从本地检查点 2dff0c6 恢复。

## 当前待办

- 在 VFXTestMap 继续爆炸弹玩法逻辑；三枪主观视觉可直接在编辑器审核。
- FEAT-080 尚未整体归档；结果不自动提交，等待明确要求“更新 Git”。
- FEAT-079 暂缓实际业务 Actor 前台验收，既有核心实现和 5/5 自动化保持有效。

## 会话交接

- 写入前检查点 2dff0c6 保存前轮 ExplosionGun Physical 1 替换（前轮已删除43项旧资产）。
- 维修枪 VFX 无效根因是可见骨骼材质无 Amount (S)，电击枪是 Noise 被 SurfaceDetail 覆盖；均已接上共享函数/噪声并实际截帧确认。
- 通用 VFX 代码位于 Weapons/_Shared/EquipmentBase/Effects；FPSCharacterBase.PlayInitialEquipEffect 已删除，首装/切换统一入口在 EquipmentManager.QueueEquipPresentation。
- 未来自定义材质未接溶解函数时，组件临时使用共享备用表面并在结束后恢复；需要显现期间保留原纹理时接共享函数。
- 旧 EquipDissolveEvidence 已改为 PIE 准备后明确启动测量播放，避免启动/着色器工作错过首帧；最终回归通过。
