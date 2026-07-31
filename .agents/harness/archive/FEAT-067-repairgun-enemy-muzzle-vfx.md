# FEAT-067 — RepairGun 与人形 Enemy 枪口特效

**状态：** done  
**创建/完成：** 2026-07-31

## 来源与范围

- 来源：`D:\游戏\游戏资产\特效\UE389`。
- ZIP 中央目录完整；原解压目录重复嵌套，因此重新解压到独立 `D:\Unreal Projects\UE389_MuzzleSource` 并用 UE 5.7 加载验证。
- 只选择 `Energy Burst 2`（RepairGun）和 `Physical Burst 1`（人形步枪）。Asset Registry 递归依赖闭包共 46 个资产；未迁移 Demo、地图、脚步或命中特效。

## 项目实现

- 全部资产经 AssetTools 移入 `/Game/Effects/_Shared/Muzzle/{Systems,Materials,Textures,Niagara}`，供应商目录已删除且无 Redirector 残留。
- 系统语义名：`NS_RepairGun_Muzzle`、`NS_HumanoidRifle_Muzzle`。
- `AFirearm` 提供可覆盖的 `MuzzleEffect/Rotation/Scale`，`UGA_Shoot` 在真实枪口 Socket 生成 Niagara。
- `UGA_EnemyShoot` 提供同样的公共数据和生成路径，三连发、扫射以及未来人形敌人技能自动复用。
- 现有旧蓝图包在保存备份阶段无法原子移动，因此未强制覆盖二进制包；两类基类构造默认值提供可靠引用，蓝图仍可覆盖。

## 验证

- UE 5.7 Development Editor 构建成功。
- 两套 Niagara System 可加载；RepairGun CDO 与 Phantom Burst CDO 默认引用断言通过。
- `Saved/Logs/FEAT067FinalRegression4.log`：8/8 Success。
- `Content/VFX_SciFi_Muzzle_And_Impact_Pack_1` 不存在；正式迁入文件正好 46 个。
