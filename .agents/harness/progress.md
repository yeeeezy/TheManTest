# 当前进度

## Active Feature

- `FEAT-080`：RepairGun、电击枪与爆炸枪统一动画和独立 VFX
- 状态：`in_progress`
- 当前阶段：暗场地图已整理到 `/Game/Maps/VFXTest/VFXTestMap`；两枪模型互换与 Laser 材质修复完成；电击枪已按用户原版录像完成枪口位置、短时青绿色点光和 D3D 实际画面验收；`MuzzleEffectScale` 现可同步缩放 Niagara 枪口效果与点光范围；Phantom 静止命中目标、EnemyBase 通用血条和无视角后坐测试配置已就绪，可继续开发爆炸弹逻辑。
- 详细历史：`archive/FEAT-080-three-weapon-setup.md`

## 已确认方案

- RepairGun、电击枪、爆炸枪使用同一套现有第一人称持枪动画内容，但开火序列、开火蒙太奇、装备蒙太奇和 AnimBP 均按武器复制并语义重命名。
- 电击枪当前使用原爆炸枪的 Ballistics Rifle 02 几何体；枪口使用 `NE_VFX_Muzzle_Laser_Burst_2`，环境/角色统一命中使用 `NE_VFX_Projectile_Impact_Laser_2`，在项目内分别命名为 `NS_ElectricGun_LaserMuzzle` 与 `NS_ElectricGun_LaserImpact`。
- 爆炸枪当前使用原电击枪的 SMG 02 几何体、Physical Burst 3 枪口效果和 Physical Impact 3 命中效果。
- 两把新枪除模型和 VFX 外直接复制 RepairGun 配置。
- 即使素材相同，也复制并重命名到各武器所有者目录，确保后续独立修改。
- 不迁移外部项目的角色、武器蓝图或动画，不进行动画重定向。
- `VFXTestMap` 审核环境临时使用 Bloom=4、Threshold=0.5 对齐源 MainScene；电击枪单独使用 `MuzzleEffectScale=1.0` 和青绿色短时枪口点光（UE 5.7 视觉补偿强度 1800、半径 200、SourceRadius 60、0.1 秒线性淡出），其他枪默认关闭。
- `MuzzleEffectScale` 是每把枪统一的枪口尺寸参数：开火时写入 Niagara 的 `User.MuzzleScale`，同时按最大绝对轴缩放点光 `AttenuationRadius` 与 `SourceRadius`；建议 XYZ 填相同值，亮度、颜色和持续时间不随尺寸倍率改变。

## 最近完成

- 外部模型/VFX 及依赖已通过 Unreal AssetTools 整理到 `/Game/Weapons/ElectricGun` 和 `/Game/Weapons/ExplosionGun`；供应商目录已清空并删除。
- 创建并配置两把新枪及各自的动画、AnimBP、音频、CameraShake、Bullet、GameplayCue 和表现依赖副本；RepairGun 同时接通并归档开火动画。
- 通过 BlenderMCP 制作并导入电击弹与爆破弹，分别配置三槽独立材质；枪体材质同步完成电击青蓝和爆破黑金橙配色调校，生成纹理均归属各自武器目录。
- 两个新 Bullet Blueprint 已改为直接继承 `ABulletBase`，不再携带 RepairGun 专属泡泡/减速行为；旧复制弹体 Mesh/Material 已删除。
- 武器命中 Cue 已统一：电击枪无论目标均使用 Laser Impact 2，爆炸枪无论目标均使用 Physical Impact 3；紫色 1.1/黄色 2.0 贴花仍只落环境。敌人额外受击表现由自身 Character Hit Cue 负责。
- `BP_MaintenanceWorker` 初始装备列表包含 RepairGun、电击枪和爆炸枪。
- Development Editor 构建成功；最新冷启动资产验证、ThreeWeaponBaseline 和定向依赖检查通过；先前 PIE 切枪/实时开火蒙太奇测试及 AmmoLifecycle 仍为 Success。
- 新建 Phantom 专属静止行为树与测试 AI Controller，并挂到 `BP_Phantom`；所有移动速度临时锁为 0。NullRHI PIE 连续 5 秒位置不变、速度为 0，行为树组件保持运行；编辑器重启冷加载确认配置已持久化。
- `AEnemyBase` 新增通用头顶简易血条，所有敌人子类自动继承并随 GAS Health/MaxHealth 即时刷新；Phantom PIE 测试确认 100→75 同步成功。
- `AFirearm::bEnableViewRecoil` 默认临时关闭，三把枪开火不再推动玩家视角；Camera Shake 配置与播放链保持启用。Development Editor 构建与定向自动化均通过。
- `ABulletBase` 现在默认使用共享 `GE_BulletDamage`；新建普通子弹 Blueprint 无需再手动填写 Hit Effect Class，只需配置 Damage，特殊子弹仍可覆盖。
- 暗场测试地图已从 `/Game/Maps/VFXTestMap` 整理到 `/Game/Maps/VFXTest/VFXTestMap`，和专属材质统一归档；地图仍保留 13 个预置 Actor、固定手动曝光、两块环境靶面与静止 `BP_Phantom`。
- 从外部 VFXPack 迁入 Laser Burst 2 与 Laser Impact 2 共 51 项依赖，全部重命名/合并到 `/Game/Weapons/ElectricGun`。新枪口与命中 Niagara 的依赖闭包全部为武器本地路径；旧 `NS_ElectricGun_Muzzle`、`NS_ElectricGun_Impact`、无引用旧依赖、供应商目录和 Redirector 已删除。
- 电击枪与爆炸枪的主枪体、Outline、模型握持偏移和 `MuzzleLocalTransform` 已成套互换；最终语义路径仍分别为各自 owner-local `SM_ElectricGun` / `SM_ExplosionGun`，枪体材质继续保持电击青蓝与爆破橙色主题。
- 查明 Laser 方形边缘来自迁移后 11 个材质实例 `Main_Texture` 为空，而非 Niagara 或混合模式加载失败；已按源工程恢复 LensFlare、Lightning、MuzzleFlash、Smoke、Fire 与 Rocks 的本地贴图引用。冷启动精确回读 11/11 通过，ThreeWeaponBaseline 与 ThreeWeaponPIESwitch 在 NullRHI 和真实 D3D 渲染设备下均为 Success。
- 逐帧读取用户原版录像 `D:\ROG\Videos\EV录屏\20260904_133033.mp4`，以 6.083–6.150s 的 3–5 帧青绿色枪体反光为验收标准。修正源线性颜色、Unitless 灯光单位、SourceRadius=60、当前枪口坐标下的持枪侧偏移和 0.1 秒线性淡出；枪口位置前移到源枪管位置，避免 Niagara 埋入模型。
- `BP_ElectricGun` 在 UE 5.7 中用 Intensity=1800 做视觉补偿；`VFXTestMap` 临时 Bloom=4/Threshold=0.5。Development Editor 构建成功；NullRHI 三武器回归 2/2 Success，D3D12/SM6 `ElectricMuzzleVisualCapture` Success，截帧强度 1500。审核图：`Saved/Screenshots/WindowsEditor/TMT_ElectricMuzzle_VideoStandard.png`。
- 电击枪 Laser System 已暴露 `User.MuzzleScale`，16 个 emitter 的 SpriteSize 在 Particle Spawn 阶段统一缩放；`GA_Shoot` 先写参数再激活 Niagara。1x 保持原外观，3x 截图确认粒子同步放大，点光 AttenuationRadius/SourceRadius 从 200/60 同步变为 600/180；蓝图默认已恢复 `(1,1,1)`。Development Editor 构建、NullRHI 两项回归和 D3D 实拍均通过。

## 当前待办

- 继续实现并验证爆炸弹玩法逻辑；优先使用 `/Game/Maps/VFXTest/VFXTestMap`，其中 Phantom 可作为不会移动的命中目标。
- 用户在 `/Game/Maps/VFXTest/VFXTestMap` 带渲染窗口的 PIE 中审核电击枪是否达到原版录像观感，并继续确认枪体/弹体材质、弹体尺寸/朝向、统一武器命中 VFX 和环境贴花。
- 用户确认后将 FEAT-080 归档为 done；当前不自动提交，等待用户明确说“更新 Git”。

## 会话交接

- FEAT-079 核心实现与 5/5 自动化保持有效，已封存在 WIP checkpoint `5a39440`；用户因暂无实际业务 Actor 暂缓其前台验收。
- FEAT-080 三枪基础实现及无渲染自动化已完成。两把新枪与 RepairGun 可在 PIE 顺序切换并启动各自开火蒙太奇；两把新枪已有独立弹体、材质和统一武器命中表现。
- 2026-09-04 删除武器 Cue 的 CharacterImpactEffect 分支与两套 HitBox Flash 资产；默认 `GameplayCue.Character.Enemy.Hit` 保留，具体敌人需要差异时再新增专属 Character Hit Tag。
- 2026-09-04 创建 `BT_Phantom_TestIdle` 与 `BP_Phantom_TestIdleAIController` 并挂到 `BP_Phantom`；PIE 验证 Phantom 静止。正式 AI 的恢复值已记录在 archive 与 `arch/11-enemy-ai.md`。
- 2026-09-04 `AEnemyBase` 通用血条与三枪临时无视角后坐配置已实现；恢复视角后坐只需把 `bEnableViewRecoil` 默认值或武器 Blueprint 覆盖改为 true，Camera Shake 无需恢复。
- 默认 Bullet GE 改动及此前现场已封存于本地 WIP checkpoint `3419214`；统一武器命中 Cue、CharacterImpact 资产清理以及用户调整的 `BP_ExplosionGun` 已封存于 `5ecbdec`；暗场地图初版为 `ddf7085`；地图归档和电击枪 Laser VFX 初次迁移为 `6549004`；模型交换与 Laser 材质修复已封存于 `a2bd562`；初版枪口点光为 `48ecffe`。录像标准下的颜色/灯位/单位/强度/枪口位置和截图测试仍未提交，等待用户明确说“更新 Git”。
