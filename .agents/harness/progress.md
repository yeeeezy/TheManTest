# 当前进度

## Active Feature

- `FEAT-080`：RepairGun、电击枪与爆炸枪统一动画和独立 VFX
- 状态：`in_progress`
- 当前阶段：暗场地图已整理到 `/Game/Maps/VFXTest/VFXTestMap`，电击枪已换成 Laser Burst 2 枪口与 Laser 2 命中效果；Phantom 静止命中目标、EnemyBase 通用血条和无视角后坐测试配置已就绪，可继续开发爆炸弹逻辑。
- 详细历史：`archive/FEAT-080-three-weapon-setup.md`

## 已确认方案

- RepairGun、电击枪、爆炸枪使用同一套现有第一人称持枪动画内容，但开火序列、开火蒙太奇、装备蒙太奇和 AnimBP 均按武器复制并语义重命名。
- 电击枪使用外部 VFXPack 的 SMG 02 模型；枪口改用 `NE_VFX_Muzzle_Laser_Burst_2`，环境/角色统一命中改用 `NE_VFX_Projectile_Impact_Laser_2`，在项目内分别命名为 `NS_ElectricGun_LaserMuzzle` 与 `NS_ElectricGun_LaserImpact`。
- 爆炸枪使用 Ballistics Rifle 02 模型、Physical Burst 3 枪口效果和 Physical Impact 3 命中效果。
- 两把新枪除模型和 VFX 外直接复制 RepairGun 配置。
- 即使素材相同，也复制并重命名到各武器所有者目录，确保后续独立修改。
- 不迁移外部项目的角色、武器蓝图或动画，不进行动画重定向。

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

## 当前待办

- 继续实现并验证爆炸弹玩法逻辑；优先使用 `/Game/Maps/VFXTest/VFXTestMap`，其中 Phantom 可作为不会移动的命中目标。
- 用户在带渲染窗口的 PIE 中确认枪体和弹体材质、弹体尺寸/朝向、模型握持位置、枪口 VFX、统一武器命中 VFX 和环境贴花最终观感。
- 用户确认后将 FEAT-080 归档为 done；当前不自动提交，等待用户明确说“更新 Git”。

## 会话交接

- FEAT-079 核心实现与 5/5 自动化保持有效，已封存在 WIP checkpoint `5a39440`；用户因暂无实际业务 Actor 暂缓其前台验收。
- FEAT-080 三枪基础实现及无渲染自动化已完成。两把新枪与 RepairGun 可在 PIE 顺序切换并启动各自开火蒙太奇；两把新枪已有独立弹体、材质和统一武器命中表现。
- 2026-09-04 删除武器 Cue 的 CharacterImpactEffect 分支与两套 HitBox Flash 资产；默认 `GameplayCue.Character.Enemy.Hit` 保留，具体敌人需要差异时再新增专属 Character Hit Tag。
- 2026-09-04 创建 `BT_Phantom_TestIdle` 与 `BP_Phantom_TestIdleAIController` 并挂到 `BP_Phantom`；PIE 验证 Phantom 静止。正式 AI 的恢复值已记录在 archive 与 `arch/11-enemy-ai.md`。
- 2026-09-04 `AEnemyBase` 通用血条与三枪临时无视角后坐配置已实现；恢复视角后坐只需把 `bEnableViewRecoil` 默认值或武器 Blueprint 覆盖改为 true，Camera Shake 无需恢复。
- 默认 Bullet GE 改动及此前现场已封存于本地 WIP checkpoint `3419214`；统一武器命中 Cue、CharacterImpact 资产清理以及用户调整的 `BP_ExplosionGun` 已封存于本地 WIP checkpoint `5ecbdec`；暗场地图初版已封存在 WIP checkpoint `ddf7085`。本次完成地图归档和电击枪 Laser VFX 替换，不要重复迁移或重新生成武器资产。
