# FEAT-078 — 全项目资产目录规范化

**创建日期：** 2026-08-22
**状态：** done

## 目标

- 建立并强制执行所有者优先、资源类型次之的 Content 目录规范。
- 清空 CharacterBase 中具体角色表现资产，只保留基类 Blueprint、Data 与 GAS 基础设施。
- 统一 Characters、Actors、Weapons、Enemy 的分类名、拼写和 `_Shared` 边界。
- 通过 Unreal AssetTools 迁移，保持引用完整并清理 Redirector 与空旧目录。

## 用户确认

- 2026-08-22：用户确认执行全项目统一整理。
- MaintenanceWorker 专属资产全部迁回具体角色目录。
- Infiltrator 与 TheExecutive 尚无具体 Mesh，不创建或复制占位美术资源。
- 当前临时共用但未来计划独立替换的表现资产，按具体所有者管理，不提升到 `_Shared`。

## 执行记录

- FEAT-077 Linked Anim Layer 与双 AnimBP 工作已保存至安全检查点 `d923f88`。
- 新增 `arch/00-asset-directory-standard.md`，作为后续新建、导入和迁移的强制规范。
- 通过 AssetTools 单次批量 Rename 迁移 248 个资产，第二批补齐 InteractableBase 内部 4 个分类资产。
- CharacterBase 最终只保留 `BP_CharacterBase`、`DA_BaseCharacterAttributes`、`BGA_Shoot`、`GE_CharacterBase_Init`；全部角色表现资产归入 MaintenanceWorker。
- Infiltrator 与 TheExecutive 只整理现有 Data/GAS/Materials 等逻辑资源，没有创建或复制占位 Mesh。
- 统一 Actors、Weapons、Enemy 的 `Animations/Meshes/Materials/Data/Abilities/Effects` 命名，并修正 `Actors/Interable` 为 `Actors/InteractableBase`。
- 两条零引用且 Skeleton 已损坏的历史资产 `AS_Rifle_A_Equip`、`AS_Rifle_A_WalkFwd` 删除；检查点 `d923f88` 可恢复。
- 修复 TestMap 两个 World Partition External Actor 的旧引用；二次 Fixup 后 Characters、Actors、Weapons、Enemy Redirector 均为 0。
- 清理 10 个 AssetTools 遗留且零引用的旧 Skeletal Mesh 完整副本，以及 72 个空目录。

## 验证

- `TheManTestEditor Win64 Development`：Success。
- 冷加载：MaintenanceWorker 33 条 AnimSequence 全部具有有效 Skeleton；核心 Body/FirstPerson AnimBP、武器接口、模板和 RepairGun 子层均可加载且引用新路径。
- 非规范目录资产扫描：`Animation/Mesh/Material/DataAsset/GameplayAbility/GameplayEffect/Interable` 为 0。
- Redirector：`/Game/Characters`、`/Game/Actors`、`/Game/Weapons`、`/Game/Enemy` 均为 0。
- `TheManTest.Player.Shadow.UpperBodyEvidence`：Success。
- `TheManTest.Player.Viewmodel.FramingCapture`：Success。
- `TheManTest.Player.Viewmodel.EquipDissolveEvidence`：Success。
