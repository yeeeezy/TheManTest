# FEAT-068 — 全项目资产与源码语义目录整理

**状态：** completed  
**创建：** 2026-08-01

## 目标

- 不保留外部素材包、供应商或迁移暂存目录。
- Unreal 资产按项目既有顶层语义分类，并优先归属具体功能对象。
- C++ 与编辑器资产采用一致的所有权原则：专属代码进入具体 Character、Enemy 或 Weapon；跨多个实际使用方的代码进入 `_Shared`。
- 整理后完成引用、Redirector、编译和自动化回归验证。

## 开始状态

- `/Game/Effects/_Shared/Muzzle` 有 46 个上一轮错误放置的枪口资产。
- `/Game/Effects/_Shared` 有 6 个 ShapesFX 遗留资产。
- `/Game/ShapesFX_Pack` 与 `/Game/RTG` 无注册资产，但磁盘残留空目录。
- `/Game/Inputs` 有 13 个输入资产，应归入 `/Game/Core/Input`。
- C++ Gameplay Ability 集中在 `GAS/Abilities`，未表达角色、Enemy、Weapon 所有权。
- 写入前安全检查点：`bffbfb7`。

## 用户确认的规则

- 不使用外部导入目录作为项目正式目录。
- 特效不得建立顶层 `Effects`；例如 RepairGun 枪口特效进入 RepairGun 自己的目录。
- 专属 GAS 代码进入具体角色/敌人/武器目录，通用内容建立 `_Shared`，尽量与编辑器目录一致。
- 整理完成后检查编译问题。

## 完成结果

- 迁移 13 个输入资产到 `/Game/Core/Input/{Actions,Contexts}`。
- RepairGun System 迁入 `/Game/Weapons/RepairGun/Effects/Muzzle/Systems/`。
- Phantom System 迁入 `/Game/Enemy/Phantom/Effects/Muzzle/Systems/`。
- 两套枪口共用的 Niagara、材质与纹理依赖迁入 `/Game/Core/_Shared/Effects/Muzzle/`。
- Interactable 与 Infiltrator 共用的 Shapes 基础资产迁入 `/Game/Core/_Shared/Effects/Shapes/`。
- 删除/清理 `Effects`、`Inputs`、`ShapesFX_Pack`、`RTG` 旧目录；冷启动 Asset Registry 均返回 0 个旧路径资产。
- C++：`GA_Shoot` → `Weapons/_Shared`；`GA_InfiltratorScan` → Infiltrator；Enemy 公共射击/自动射击/换弹/掩体 → `Enemy/_Shared`；AreaBarrage → Phantom；全局 Gameplay Tags → `Core/_Shared`。
- 更新全部 include、硬编码枪口路径、测试路径和架构文档。

## 验证

- WIP 安全检查点：`bffbfb7`。
- Development Editor / Win64 冷编译：Succeeded，UHT 15 个文件、UBT 18/18 actions 成功。
- 冷启动加载：两套 Niagara System、共享 Niagara Effect Type、共享 Shapes Function、`IMC_Default` 全部成功。
- 两套 Niagara System 依赖中无 `/Game/Effects`；本次目标目录 Redirector 为 0。
- 顶层目录已收敛，不再有 `Effects`、`Inputs`、`ShapesFX_Pack`、`RTG`。
- 自动化命令已启动但本次编辑器日志未返回完成汇总，不记为通过。启动日志另有既存 `TestGun/A_HandFire` 缺 Skeleton 错误，与本次移动路径无关。
