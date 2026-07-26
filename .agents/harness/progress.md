# 进度日志

## 当前状态

**最后更新：** 2026-07-26-session79
**当前功能：** **FEAT-046（步枪上半身 1D BlendSpace 状态机）**  
**会话编号：** 75

FEAT-045（新选角场景摄像机远近切换）已关闭，状态和验证证据已写入 `feature_list.json` 与 `archive/FEAT-045-character-select-camera-toggle.md`。

FEAT-047（RepairGun 球形子弹 Static Mesh）已作为一次性资产任务完成并归档；active feature 保持 FEAT-046。

FEAT-048（RepairGun 科技多面体泡沫子弹）已覆盖 FEAT-047 的简单球体并完成归档；active feature 保持 FEAT-046。

FEAT-049（Git 与 Git LFS 项目基线）已完成；本地 `main` 分支和 LFS 基线已建立，尚未配置远程。active feature 保持 FEAT-046。

当前只做 FEAT-046：步枪上半身 Linked Anim Layer 保留 `SM_FirearmUpperBody` 状态机壳，改为普通 `Idle <-> Locomotion`。`Locomotion` 内使用 1D BlendSpace 按 `Speed` 混合 Idle/Walk/Run 上半身持枪循环。

---

## 当前完成项

- [x] 新建 FEAT-046，并设为 `active_feature`。
- [x] 新建 `archive/FEAT-046-rifle-upperbody-start-end.md`。
- [x] 曾尝试 `Idle / WalkStart / WalkLoop / WalkEnd / RunStart / RunLoop / RunEnd` 主路径；用户确认动画衔接不理想，决定回退为普通 1D BlendSpace。
- [x] 已删除 FEAT-046 为 Start/End 状态机新增的 C++ 临时变量；`UEquipmentAnimInstance` 只保留通用装备动画层变量：`Speed` / `Direction` / `Velocity_Z` / `bIsFalling`。
- [x] 已同步：
  - `archive/FEAT-046-rifle-upperbody-start-end.md`
  - `feature_list.json`
  - `arch/06-animation.md`
  - `arch/12-anim-blueprint.md`
- [x] 用户已在 `SM_FirearmUpperBody` 中改为第一版普通结构：`Idle <-> Locomotion`，转移条件为 `Speed > 3.0` / `Speed <= 3.0`。

---

## 当前待办

- [ ] 在 `SM_FirearmUpperBody` 的 `Locomotion` 状态里放入 1D BlendSpace Player，并用 `Speed` 驱动。
- [ ] 创建或确认共用 1D BlendSpace 资产，推荐命名 `BS_Rifle_UpperBody_Locomotion`；不要带 MaintenanceWorker 名，因为该资产是步枪上半身通用层，MaintenanceWorker 使用 Rifle 时复用它。
- [ ] 1D BlendSpace 配置：
  - 0：持枪 Idle
  - 走速点：持枪 Walk Loop
  - 跑速点：持枪 Run Loop
- [ ] 保持已验证的上半身混合设置：
  - `Layered Blend per Bone`
  - Branch Filter Bone = `spine_01`
  - Blend Depth = `2`
  - Blend Weight = `1.0`
  - 启用 Mesh Space Rotation Blend
- [x] C++ `TheManTestEditor Win64 Development` 重新编译通过，确认删除变量后无错误无新增警告。
- [ ] 编译 `TABP_Firearm_UpperBodyBase` / `ABP_Rifle_UpperBody`。
- [ ] PIE 验证：
  - 步枪装备后站立正常。
  - 走/跑循环随速度 BlendSpace 平滑变化。
  - `ArmsViewMesh` 与 `GetMesh()` 上半身层同步生效。

---

## 当前阻塞

无。

---

## 注意事项

- 按用户规则，任何新的实现、代码/资产/配置修改、GameMode/UI/输入方案调整，都必须先列方案并等待用户确认。
- 用户要求编辑器操作一步一步教，每一步做完用户说“好了”再继续。
- 独立 Blender 工程统一放在 `D:\Blender Projects\<项目名>\`，不得把 `.blend`、建模脚本或 MCP 下载文件放进 Unreal 项目；完整规则见 `AGENTS.md` 的“Blender 资产与 MCP 规则”。
- `progress.md` 只保留当前工作面板：不是每次重写整份文件，而是清掉已经结束、废弃、无行动价值的旧信息，保留继续当前工作所需的最小面板。
- 如需回查历史：
  - FEAT-045：`archive/FEAT-045-character-select-camera-toggle.md`
  - FEAT-046：`archive/FEAT-046-rifle-upperbody-start-end.md`
  - 上半身武器架构：`arch/06-animation.md`、`arch/09-equipment-system.md`、`arch/12-anim-blueprint.md`

---

# 会话交接

## Session78 handoff - FEAT-046 remains active (2026-07-26)

- 当前 active_feature 是 `FEAT-046`。
- 用户决定放弃 Walk/Run Start/Loop/End 多状态方案，因为动画衔接不理想。
- 新方向：保留 `SM_FirearmUpperBody` 状态机壳，内部只用 `Idle <-> Locomotion`；`Locomotion` 内放 1D BlendSpace，按 `Speed` 混合持枪 Idle/Walk/Run 循环。
- C++ 已删除 Start/End 状态机专用临时变量，只保留 `Speed` / `Direction` / `Velocity_Z` / `bIsFalling`。
- 用户已完成 `Idle <-> Locomotion` 两个状态和基础转移：`Speed > 3.0` / `Speed <= 3.0`。
- 下次继续：先创建或确认 `BS_Rifle_UpperBody_Locomotion`，把 1D BlendSpace Player 放进 `Locomotion` 状态并接 `Speed`，再编译 AnimBP、PIE 验证。
- 用户要求一步一步教编辑器操作，并且每一步做完用户说“好了”再继续。
- FEAT-047 已完成：`/Game/Weapons/RepairGun/Mesh/SM_RepairGun_Bullet` 与蓝色材质已导入并通过独立命令行加载验证；未赋值到 RepairGun 子弹蓝图。详见 `archive/FEAT-047-repairgun-bullet-mesh.md`。
- FEAT-048 已完成并取代 FEAT-047 的简单球体：`SM_RepairGun_Bullet` 现为用户确认的科技多面体泡沫核心；独立加载验证通过。详见 `archive/FEAT-048-repairgun-tech-foam-bullet.md`。
- FEAT-049 已完成：项目现有本地 Git `main` 基线和 Git LFS；基线提交为 `33c70d1`，无远程、无上传。详见 `archive/FEAT-049-git-lfs-baseline.md`。

## Session79 handoff - Unreal MCP isolated integration (2026-07-26)

- FEAT-050 已在 `test/unreal-mcp-5.7` 分支完成；`main` 未合并、未受影响。
- 外部插件固定为 `D:\Unreal Plugins\Unreal_mcp-v0.5.30`，项目通过 `AdditionalPluginDirectories` 引用。
- UE 5.7.4 Development Editor / Win64 编译成功；编辑器运行时 MCP 仅监听 `127.0.0.1:3000`。
- 原生 MCP `initialize` 与 `tools/list` 已验证，返回 23 个工具；未执行任何资产写入。
- Codex 用户配置已登记 `unreal-engine`；需要重启 Codex 会话后才会作为原生工具加载。
- `.agents/harness/init.ps1` 仍因既有乱码/解析错误无法运行，本次按 caveat 未修复。
- active_feature 仍为 FEAT-046；后续业务开发继续其 1D BlendSpace 编辑器验证。
