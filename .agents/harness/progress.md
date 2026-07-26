# 进度日志

## 当前状态

**最后更新：** 2026-07-26-session88
**当前功能：** **FEAT-051（基于原始骨架重建角色与 Enemy 动画蓝图）**
**会话编号：** 88

用户已手动删除一部分效果不佳的重定向动画和动画蓝图。现有 C++ AnimInstance、无骨架 Template AnimBP 和状态机驱动架构继续保留。

玩家仍让 `GetMesh()`、`ArmsViewMesh` 与武器 Linked Anim Layer 共用玩家 Skeleton；玩家下半身可使用效果合格的重定向动画。Enemy 优先使用各自动画原始 Skeleton，并从无骨架 Template AnimBP 创建对应骨架的子 AnimBP。

---

## 当前完成项

- [x] Unreal MCP 复扫用户删除后的 Player / Enemy / Weapon 动画资产。
- [x] 玩家模板已整理为 `TABP_BodyLocomotion`；维修工子 AnimBP `ABP_MaintenanceWorker` 及 `BS_RunWalk_MaintenanceWorker` 已创建并由用户编译通过。
- [x] 维修工身体、下半身、手臂与临时动画统一到手臂 Skeleton；当前阶段接受参考姿势差异，只验证代码和 AnimBP 架构。
- [x] RepairGun 专属层 `ABP_RepairGun_AnimLayer` 与 `BS_WalkRun_RepairGun` 已创建。
- [x] 删除无用 `EquipmentAnimClass` 整体替换路径；武器只通过 `EquipmentAnimLayerClass` 链接专属层。
- [x] Enemy 模板/子资产仍存在：`ABP_HumanoidEnemy`、`ABP_Phantom`；Phantom 保留原始 `SK_Cyber01_Skeleton` 动画集。
- [x] FEAT-046 改为 `needs_improvement`：实际 `SM_FirearmUpperBody` 为 `Idle <-> WalkRun`，不是旧记录中的 `Idle <-> Locomotion`。
- [x] MCP 确认 `BS_Rifle_UpperBody_IdleWalkRun` 为 2D BlendSpace 且 0 samples，原 1D 目标未完成。
- [x] 修正 `arch/09` 的旧动画层目标描述，以及 `arch/12` 顶部过期核心资产表。
- [x] 建立 FEAT-051，并记录玩家统一 Skeleton / Enemy 原始 Skeleton 策略。
- [x] FEAT-052：创建 `/Game/Weapons/_Shared/Mesh/SM_Shared_Bullet`，并按 Mesh / Material / Textures 整理通用与 RepairGun 专属资源。
- [x] FEAT-053：建立 `guides/unreal-mcp-workflow.md`，沉淀 UE 5.7 MCP 操作与排错经验。
- [x] FEAT-054：创建并导入 1 米 `SM_InteractableBase_Default`，替换 `BP_InteractableBase` 的 SCI-FI 默认方块引用。
- [x] FEAT-055：从 UE4 Mannequin 完整身体拆出维修工下半身 `SKM_MaintenanceWorker_LowerBody`，保留蒙皮并绑定迁入 Skeleton。

---

## 当前待办

- [ ] 在 `BP_RepairGun` 将 `EquipmentAnimLayerClass` 配置为 `ABP_RepairGun_AnimLayer`，编译并验证装备/卸下。
- [ ] 为每种 Enemy 使用其动画原始 Skeleton 创建或确认子 AnimBP。
- [ ] 按每套 Enemy 原始骨架检查 `hand_r` / `hand_l` / spine 链 / `AimSocket` / 武器握把与 IK 节点。
- [ ] 检查活动 AnimBP 是否存在指向已删除动画资产的失效引用，并逐个编译。
- [ ] PIE 验证玩家 locomotion/武器层，以及各 Enemy 的巡逻、转身、瞄准和攻击动画。
- [ ] 调查活动 `BP_Infiltrator` 对 `BP_Infiltrator_Old` 的硬引用来源；未经用户确认不自动修改。
- [ ] 替换 `BP_Infiltrator` 对 `/Game/SCI_FI_WEAPON_PACK/SCF_Rifle_02/Demo/FirstPerson/Character/Mesh/SK_Mannequin_Arms` 的直接引用。
- [ ] 替换 `BP_RepairGun` 对 `/Game/SCI_FI_WEAPON_PACK/SCF_Rifle/Demo/FirstPerson/Audio/FirstPersonTemplateWeaponFire02` 的直接引用。
- [ ] 确认维修工第一人称身体组件后，将 `SKM_MaintenanceWorker_LowerBody` 配置到对应 Mesh，并验证动画与视角裁切。

---

## 当前阻塞

具体动画资产由用户手动选择和配置；Codex 等待用户完成一个可验证的子 AnimBP 或指定下一步编辑器操作。

---

## 注意事项

- 按用户规则，任何新的实现、代码/资产/配置修改、GameMode/UI/输入方案调整，都必须先列方案并等待用户确认。
- 用户要求编辑器操作一步一步教，每一步做完用户说“好了”再继续。
- 独立 Blender 工程统一放在 `D:\Blender Projects\<项目名>\`，不得把 `.blend`、建模脚本或 MCP 下载文件放进 Unreal 项目；完整规则见 `AGENTS.md` 的“Blender 资产与 MCP 规则”。
- `progress.md` 只保留当前工作面板：不是每次重写整份文件，而是清掉已经结束、废弃、无行动价值的旧信息，保留继续当前工作所需的最小面板。
- 不把“Enemy 使用原始 Skeleton”误解为重构 C++ 动画架构；复用 AnimInstance 驱动和 Template 状态机，变化的是具体 Skeleton、子 AnimBP 和动画 Override。
- 玩家 Skeleton 继续统一，以保证 `GetMesh()`、`ArmsViewMesh` 和武器 Linked Anim Layer 兼容。
- Enemy 骨骼命名可能不同，骨骼相关节点不能未经核对直接复用。
- 写入代码、配置、蓝图、关卡或资产前先检查 Git；工作区存在明确改动时先创建本地 WIP checkpoint。范围异常或归属不明时先报告。只读操作不提交，写入结果仍等用户明确说“更新 Git”，且不自动 push/merge。
- 当前归档：FEAT-046 见 `archive/FEAT-046-rifle-upperbody-start-end.md`；FEAT-051 见 `archive/FEAT-051-original-skeleton-character-enemy-animation.md`。

---

# 会话交接

## Session87 handoff - FEAT-051 active (2026-07-26)

- 当前 active feature 是 `FEAT-051`。
- FEAT-046 已转为 `needs_improvement`；MCP 证实其实际状态和 BlendSpace 与旧记录不符。
- 玩家继续统一 Skeleton 和武器层；下半身允许使用效果合格的重定向动画。
- Enemy 使用动画原始 Skeleton，从现有无骨架 Template AnimBP 派生对应子 AnimBP，C++/状态机架构不变。
- 用户负责具体动画资产的手动删除、选择与 Override；下一步应从一个明确的 Enemy 子 AnimBP 开始，编译后再 PIE 验证。
- MCP 另发现 `BP_Infiltrator` 仍硬引用 `BP_Infiltrator_Old`，仅记录，尚未修改。
- 新 Git 安全规则：任何写入前检查工作区；必要时自动创建本地 WIP checkpoint，结果提交仍由用户明确触发。
- Rider 的 MCP C4702 编译错误已修复：外部插件本地分支 `fix/ue57-c4702`，commit `c9bee30`；UE 5.7 `TheManTestEditor Win64 Development -WarningsAsErrors` 40/40 构建通过。
- FEAT-052 已完成但不改变 active feature：新增通用弹体 `/Game/Weapons/_Shared/Mesh/SM_Shared_Bullet`；通用材质位于 `_Shared/Material`；RepairGun 子弹材质已移动到 `RepairGun/Material`；MCP 验证引用、尺寸与无重定向器均通过。
- FEAT-053 已完成但不改变 active feature：后续 Unreal MCP 资产操作先读 `guides/unreal-mcp-workflow.md`，并优先使用定向查询、结果复核和选择性 Git checkpoint。
- MCP 定向检查最初确认 3 个 SCI_FI_WEAPON_PACK 直接引用：`BP_Infiltrator` 的手臂 Mesh、`BP_InteractableBase` 的默认方块 Mesh、`BP_RepairGun` 的开火音效。`BP_TestGunBullet` 已使用共享子弹，无该资源包引用。
- FEAT-054 已解除其中的 `BP_InteractableBase` 引用：蓝图现使用 `/Game/Actors/Interable/InteractableBase/Mesh/SM_InteractableBase_Default`；尺寸、材质路径、蓝图依赖和 Redirector 均已由 MCP 验证。其余待处理引用为 `BP_Infiltrator` 手臂 Mesh 与 `BP_RepairGun` 开火音效。
- FEAT-055 已完成资产制作：维修工下半身位于 `/Game/Characters/MaintenanceWorker/TempCharacterBody/Meshes/SKM_MaintenanceWorker_LowerBody`，绑定迁入的 `SK_UE4Mannequin`；尚未配置到角色蓝图组件。
