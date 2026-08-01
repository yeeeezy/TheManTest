# FEAT-069 — 全项目 C++ 语义目录与未使用模板资产清理

**状态：** needs_improvement  
**创建：** 2026-08-01

## 目标

- 整理整个 `Source/TheManTest`，不只处理 GAS。
- 源码以 Character、Enemy、Weapon、Actor 等实际所有权归档，并与 Content 结构保持一致。
- 删除有明确 0 引用证据的 Engine/模板/测试迁入资产。
- 把仍在使用的 `TempCharacterArm` / `TempCharacterBody` 资产迁入 MaintenanceWorker 正式目录。
- 不删除 UE 安装目录 `/Engine`；不依据单一 0 referencer 结论误删入口 Map。

## 开始状态与安全点

- 上一轮结果安全检查点：`e32c1f8`。
- `Source/TheManTest/Characters/Enemy` 与 Content 顶层 `Enemy` 不一致。
- `Source/TheManTest/Equipment` 与 Content 顶层 `Weapons` 不一致。
- `Characters/BaseCharacter`、通用 Components/Animation、顶层 Interfaces/Editor/Tests 所有权不清晰。
- 只读 Asset Registry 审计发现 TempCharacterBody 2 个、TestGun 25 个、MaintenanceWorker Animation 17 个、IK Rig 1 个零引用候选；删除前继续核对软引用、C++ 路径和依赖。

## 已实施

- `Source/TheManTest` 顶层统一为 `Actors`、`Characters`、`Core`、`Enemy`、`Environment`、`UI`、`Weapons`；Enemy、角色基类、通用组件/动画、Editor、Tests、Equipment/Firearm/TestGun/RepairGun 等均按所有权迁移，include 与架构文档同步更新。
- 仍使用的 15 个手臂资产迁入 `/Game/Characters/MaintenanceWorker/FirstPersonArms`，8 个身体资产迁入 `/Game/Characters/MaintenanceWorker/Body`；旧 Temp 目录清空。
- 删除 MaintenanceWorker 17 个零引用动画、2 个零引用身体模板资产，以及零引用 `IK_SKM_CyberpunkMetalhead_FullBodyA`。该 IK Rig 是历史导入残留，本次删除依据是项目 destination-only 边界和零引用审计，不是泛化为“任何 IK 都禁止”。
- TestGun 从 51 个资产收敛为 12 个当前保留资产。用户确认 TestGun 动作之后会整体替换，因此清空 `BP_TestGun.FireMontage`，并删除仅由该链引用的 `AM_HandFire_Montage` 与缺 Skeleton 的 `A_HandFire`。
- TestMap、Lobby、GASP 和 World Partition 外部对象的递归依赖闭包均未触达 `/Game/Maps/SciFiIndustrialBase`；确认整包 328 个资产自包含且未使用后删除，约释放 1.67 GB。
- 修复 `FCharacterType::CharacterIcon` 未初始化警告；修复 Phantom 自动化测试在 NullRHI 下直接解引用空 RenderTarget resource 的崩溃。

## 验证

- Development Editor / Win64 最终构建：Succeeded（2026-08-01，8/8 actions）。
- MaintenanceWorker、TestGun、TestMap、LobbyMap 资产验证通过；目标蓝图编译成功；旧 Temp/SciFiIndustrialBase 路径和相关 Redirector 均为 0。
- `TheManTest.Enemy.Phantom` 自动化执行 8 项：6 通过；`PIESmoke` 在 NullRHI 下未观察到 RepairGun/Phantom 可见枪口 Niagara，`PIETacticalApproach` 的持续接近/侧移断言未通过。需前台 PIE 复核，因此不标记 done。
- `BP_TestGun.FireMontage` 回读为 null，蓝图重新编译并保存成功；`/Game/Weapons/TestGun` 递归验证通过，共 12 个资产。原 `A_HandFire` 缺 Skeleton 错误随旧动作链删除而消除。
