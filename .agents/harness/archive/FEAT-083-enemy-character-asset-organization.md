# FEAT-083 Enemy／Character 资产目录整理

## 2026-09-15 实现

- 用户确认执行此前审计方案。操作前本地检查点 `64f5d15` 保存已验证的地图目录整理成果；未推送远端。
- 通过 Unreal AssetTools 移动 468 个资产：CoreMorph 的 154 个 Manta 网格进入 `/Game/Enemy/Boss/CoreMorph/Meshes/Manta`，301 个 Scorpion 网格进入 `Meshes/Scorpion`；`BP_EnemyCoverPoint` 进入 `/Game/Enemy/Humanoid/_Shared/Cover/Blueprint`。
- MaintenanceWorker 旧 `/FirstPerson/Materials/UE4Mannequin` 下 12 个资产完成语义化归档。审计发现其中 8 个资产与既有 `Materials/Layers` 同名，但两套分别被第一人称手臂／腿材质和 `M_UE4Man_Body` 使用，不能覆盖或删除；旧套因此进入 `Materials/Layers/Arms` 与 `Textures/Arms`，`M_UE4Man_Body_Source` 进入 `Materials/Source`。旧 `UE4Mannequin` 目录已清除。
- 没有执行动画重定向、IK Retargeter 设置或动画资产生成。

## 验证

- `organize-enemy-character.log`：`ENEMY_CHARACTER_ORGANIZED`，468 项迁移完成；Manta 154、Scorpion 301、Character 12、Cover 1，Redirector 0。
- 独立冷启动 `verify-enemy-character-organization.log`：`ENEMY_CHARACTER_COLD_OK`。468 项新路径资产全部加载；CoreMorph 主蓝图和 Cover 蓝图编译；维修工第一人称手臂／腿材质加载；旧 Character 路径、CoreMorph 根目录散落网格和范围内 Redirector 均为 0。
- 磁盘复查确认旧 Character 目录不存在，CoreMorph `Meshes` 根只保留 Manta／Scorpion 两个目录，Cover 根只保留标准分类目录。
- 额外尝试运行 `TheManTest.Enemy.CoreMorph` 自动化筛选，但当前冷编辑器报告没有匹配测试、执行 0 项；未将其计作通过证据。

## Git

- 本批结果未最终提交、未 push。`Saved/Codex` 下执行与验证脚本／日志为忽略的本地证据。
