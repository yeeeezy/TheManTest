# FEAT-072 — RepairGun应用Sniper Scout枪口火焰与烟雾

**状态：** done

**创建：** 2026-08-01

## 目标

- 使用外部 VFXPack 展示地图中 `BP_Weapon_Sniper_Scout_Child` 的实际枪口火焰与烟雾效果。
- 只迁移精确效果和必要依赖，整理到 RepairGun 专属语义目录。
- 将效果接入 `BP_RepairGun` 并验证开火表现和资源健康度。

## 已确认来源与方案

- 外部项目：`D:\Unreal Projects\UE389_MuzzleSource\VFX Pack - Stylized FPS Muzzle and Impacts Effects 5.1\VFXPack\VFXPack.uproject`。
- 展示地图：`/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/Maps/MainScene`。
- Sniper Scout 蓝图：`/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/Example_Content/DemoContent/Blueprints/Weapons/BP_Weapon_Sniper_Scout_Child`。
- 该蓝图 CDO 实际引用：`/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/VFX/Presets/Muzzle/NE_VFX_Muzzle_Energy_Burst_1`（Niagara System）。
- 正式目标根目录：`/Game/Weapons/RepairGun/Effects/Muzzle/`。
- 用户已于 2026-08-01 明确确认迁移、整理、接入和验证方案。
- 写入前 WIP checkpoint：`e153470`。

## 已实施

- 原生 Migrate 仅迁移 `NE_VFX_Muzzle_Energy_Burst_1` 及其 31 个必要依赖，没有迁移示例地图、Sniper 蓝图或武器 Mesh。
- 29 个项目中已存在的相同依赖合并至 `/Game/Core/_Shared/Effects/Muzzle/`，避免重复资产。
- RepairGun 专属新增资产：
  - `/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_SniperScout_Muzzle`
  - `/Game/Weapons/RepairGun/Effects/Muzzle/Materials/MI_RepairGun_Smoke_Puff_Forward_01`
  - `/Game/Weapons/RepairGun/Effects/Muzzle/Textures/T_RepairGun_Smoke_Puff_Forward_01`
- `BP_RepairGun.MuzzleEffect` 已改为新 System；保留原有 RepairGun 枪口变换配置。
- 供应商目录已清空并移除；本次产生的 32 个中间 Redirector 在确认无引用后删除。

## 验证

- 命令行冷启动回读：新资产类型为 `NiagaraSystem`，`BP_RepairGun` 引用落盘。
- 新 System 的直接项目依赖仅指向 `Core/_Shared` 与 RepairGun 专属前向烟雾材质；供应商资产计数 0、目标目录 Redirector 0。
- 编辑器中 `BP_RepairGun` 编译并保存成功。
- Niagara System 验证：valid，0 errors，0 warnings。
- RepairGun 蓝图与 Muzzle 目录资产验证：全部可加载，目录共 4 个正式资产（含保留的旧 System）。
- PIE 中触发 LMB 开火并即时截图，确认新枪口效果在枪口生成；近期日志无 Niagara、Socket、加载或运行错误。
