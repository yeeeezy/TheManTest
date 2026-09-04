# FEAT-080 RepairGun、电击枪与爆炸枪统一动画和独立 VFX

## 目标

- 为 RepairGun 接通现有第一人称开火蒙太奇。
- 以 RepairGun 为完整配置基线创建电击枪和爆炸枪。
- 两把新枪只替换模型、枪口 VFX、命中 VFX 与贴花；玩法、动画、音频、弹药、GAS、后坐力和子弹行为保持与 RepairGun 一致。
- 外部来源仅迁移最终模型和 VFX 依赖，不迁移源项目角色、武器蓝图或动画。

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
