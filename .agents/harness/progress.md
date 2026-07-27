# 进度日志

## 当前状态

**最后更新：** 2026-07-27-session92
**当前功能：** **FEAT-051（基于原始骨架重建角色与 Enemy 动画蓝图）**
**会话编号：** 92

用户已手动删除一部分效果不佳的重定向动画和动画蓝图。现有 C++ AnimInstance、无骨架 Template AnimBP 和状态机驱动架构继续保留。

玩家仍让 `GetMesh()`、`ArmsViewMesh` 与武器 Linked Anim Layer 共用玩家 Skeleton；玩家下半身可使用效果合格的重定向动画。Enemy 优先使用各自动画原始 Skeleton，并从无骨架 Template AnimBP 创建对应骨架的子 AnimBP。

---

## 当前完成项

- [x] Unreal MCP 复扫用户删除后的 Player / Enemy / Weapon 动画资产。
- [x] 玩家模板已整理为 `TABP_BodyLocomotion`；维修工子 AnimBP `ABP_MaintenanceWorker` 及 `BS_RunWalk_MaintenanceWorker` 已创建并由用户编译通过。
- [x] 维修工身体、下半身、手臂与临时动画统一到手臂 Skeleton；当前阶段接受参考姿势差异，只验证代码和 AnimBP 架构。
- [x] RepairGun 专属层 `ABP_RepairGun_AnimLayer` 与 `BS_WalkRun_RepairGun` 已创建。
- [x] 将武器上半身分层职责集中到主 `TABP_BodyLocomotion`：唯一 `Layered Blend per Bone` 从 `spine_01`、Depth 4 渐进混合并开启 `Mesh Space Rotation Blend`；武器模板只输出武器姿势，所有武器统一避免 A/D 横移带偏枪口。
- [x] 影子腰部断层最终诊断为当前完整身体模型自身的腰部几何分段，并非动画混合产生；中央混合不能修复模型轮廓，后续需补齐/重叠腰部网格或替换连续全身模型。`FullBodySlot` 仍调整为最终最高优先级覆盖。
- [x] MCP 编译并保存 `TABP_Firearm_UpperBodyBase`、`ABP_RepairGun_AnimLayer`、`TABP_BodyLocomotion`、`ABP_MaintenanceWorker` 均成功；PIE 自动 A/D 横移运行无加载/编译错误。
- [x] PIE 定量验证通过：A 与 D 在相同 Walk 动画相位下，`ArmsViewMesh.hand_r` 相对相机的 5 组 Pitch/Yaw/Roll 样本逐项一致；左右方向只改变全身 `spine_01` 姿势，不再改变持枪手最终朝向。运行时 `CharacterMesh0` 与 `ArmsViewMesh` 均使用 `ABP_MaintenanceWorker_C`，`ShadowBodyMesh` 与 `LegsMesh` 的 Leader Pose 均为 `CharacterMesh0`，同步链保持不变。
- [x] 中央混合重构后再次编译保存四个 AnimBP 并完成 PIE A/D 复测；运行时仍只有 `ShadowBodyMesh` 投影，`ShadowBodyMesh` / `LegsMesh` 的 Leader Pose 均为 `CharacterMesh0`，第一人称与全身 AnimClass 均为 `ABP_MaintenanceWorker_C`。
- [x] 修复中央混合首次重接造成的 locomotion 回归：Pose 输出不能一对多，连接 `WeaponUpperBody.UpperBodyInPose` 时曾顶掉中央混合的 `BasePose`；现恢复 `DefaultSlot.Pose -> LayeredBlend.BasePose`，纯武器层不读取接口输入。PIE W 移动连续 5 次腿骨采样均变化，A/D 截图也显示移动动画恢复。
- [x] 修复 TestMap 进入 PIE 后角色延迟显示：移除 `AFPSCharacterBase::BeginPlay` 对身体、第一人称手臂、影子、腿和当前武器的整套隐藏/下一帧显示流程；装备仍同步初始化并保持可见，仅将可选拔枪 Montage 留到下一帧。Development Editor/Win64 与 Live Coding 均成功，PIE 确认默认 `BP_MaintenanceWorker` 同步生成且首个正常动画 Tick 后手臂、武器、影子完整显示。
- [x] 修复即时显示调整后暴露的影子动画刷新问题：`BP_MaintenanceWorker` 的旧序列化值曾把不可见动画宿主 `CharacterMesh0` 覆盖为 `AlwaysTickPose`；现由 `AFPSCharacterBase::BeginPlay` 强制恢复 `AlwaysTickPoseAndRefreshBones`。PIE W 移动 5 组 `thigh_l` 样本持续变化，Leader 与 `ShadowBodyMesh` 坐标逐组完全一致。
- [x] 修复切回 RepairGun 时 Equip Montage 在下一动画 Tick 被取消：切枪完成 Linked Layer/挂载后改为下一帧播放，并校验快速滚轮后目标仍是当前装备；`PlayEquipMontage()` 同步驱动 `ArmsViewMesh` 与 `CharacterMesh0`，影子经 Leader Pose 继承。PIE 两实例从 0 同步推进至 0.3333 秒，结束后 W locomotion/影子同步正常。
- [x] 修复 RepairGun Equip Montage “正在播放但画面无动作”：原 Montage 仅有 `DefaultSlot`，其上半身输出被主 ABP 后续中央 `WeaponUpperBody` 完全覆盖；现加入同源动画的 `UpperBodySlot` 轨道。暂停 PIE 逐帧截图在 0/0.333/0.666 秒显示清晰拔枪姿势变化，第一人称与影子同步。
- [x] 修复 RepairGun 切入时先从上方放下再拿起：源序列采样证明动画本身从下方向上抬，异常来自 Montage 默认 0.25 秒 Blend In 与武器提前显示。现 Blend In=0；切枪时有 Montage 的武器先隐藏，启动并评估一帧后再显示。PIE 首张可见图位于下方，后续只向上抬；移动速度 250、影子 Leader/腿骨同步正常。
- [x] 修复新武器抬起前的一帧空手残影：不再先隐藏旧武器；有 Montage 时旧武器保持在手，待新姿势就绪后旧/新武器同帧原子交换。PIE 各阶段始终恰有一把武器可见，连续 4 次快速切换也无双隐藏；移动/影子回归正常。
- [x] 删除无用 `EquipmentAnimClass` 整体替换路径；武器只通过 `EquipmentAnimLayerClass` 链接专属层。
- [x] 暂停玩家原地转身：删除 `BodyVisualYaw`/45° Turn/曲线进度 C++ 链，`BodyRoot` 直接跟随 Actor yaw；ABP 转体节点待用户手动清理。
- [x] 用户已清理 `TABP_BodyLocomotion` 的旧 Turn 节点；修改已保存到本地 WIP checkpoint `8e6a8e0`。
- [x] 只读调研 `D:\Unreal Projects\GameAnimationSample`：其转向脚步依赖 Motion Matching 数据库、左右支撑脚动画、Offset Root Bone、Orientation Warping 与 Foot Placement；仅作为未来独立转体方案参考。
- [x] 修复 RepairGun 子弹停在枪口：根 `CollisionSphere` 曾误设为 Static，导致 ProjectileMovement 无法移动；恢复 Movable 后用户确认飞行和命中膨胀正常。
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

当前无阻塞。A/D 横移的第一人称持枪朝向与影子同步链已完成运行时验证。

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

## Session98 handoff - FEAT-051 active (2026-07-27)

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
- TestMap 的默认角色没有异步加载问题；`BP_TheManGamemodeBase` 在未选择角色时同步回退到 `BP_MaintenanceWorker`。此前的可见延迟来自 `AFPSCharacterBase::BeginPlay` 主动隐藏全部角色渲染组件与武器，再用 next-tick 回调显示；该隐藏流程已删除，下一帧回调现在只负责播放可选 Equip Montage。
- MCP 自动化失焦时日志显示 PIE 会被编辑器节流到 3 FPS，因此“强制零 Tick 截图”只显示影子参考姿势，不能用作玩家首帧判断；约 1200ms（已有正常 Tick）截图确认第一人称手臂、RepairGun 和影子均正常。正常前台 60 FPS 下动画首 Tick 约为 16.7ms。
- 影子动画刷新回归已修复：运行时审计发现 `CharacterMesh0` 被角色 BP 旧值覆盖成 `AlwaysTickPose`，而它作为 OwnerNoSee 的 Leader 不会稳定刷新骨骼；BeginPlay 现强制 `AlwaysTickPoseAndRefreshBones`。W 移动时角色速度 250，5 组 Leader/Shadow `thigh_l` 组件空间位置完全一致，截图 `ShadowRefresh_Fixed_Move.png` 显示正常持枪移动影子。
- RepairGun Equip Montage 切枪时序已修复：旧流程在 LinkAnimClassLayers 同帧立即播放，下一动画更新会把 Montage 清掉；现在下一帧确认当前装备后再播放，且手臂/身体 Leader 双实例同步。帧步进验证两边位置由 0 同步到 0.3333 秒；截图 `EquipMontage_Synced_0333.png`，结束后移动回归截图 `EquipMontage_PostMove.png`。TestGun 当前 `EquipMontage=None`，切入 TestGun 无动画属于资产尚未配置。
- RepairGun Montage 的可视输出也已修复：原资产只有 `DefaultSlot`，被中央武器层从 `spine_01` 覆盖，所以运行时显示 playing 但动作不可见；现 Montage 有 `UpperBodySlot` 轨道（总计 2 slots、0 notifies、0.8667 秒）。逐帧截图 `EquipUpperSlot_T0.png` / `T0333.png` / `T0666.png` 显示枪与双臂明显下沉、展开、抬起，影子同步。
- “先放下再拿起”并非源动画方向错误：`AS_Rifle_A_Equip` 的 hand_r/hand_l 原始姿势从 t=0 起持续向抬枪方向变化；问题是 Montage 0.25 秒 Hermite Blend In 从已显示的持枪 Idle 混回下方起点。现 Montage Blend In=0，切枪新武器在 Montage 启动时保持隐藏，下一动画帧评估完成才显示。帧步进状态为 `hidden=True/playing=True/pos=0` → `hidden=False/playing=True/pos=0.3333`；截图 `EquipRaise_FirstVisible.png` → `EquipRaise_Later.png` 只显示从下往上。
- 空手残影来自旧武器先隐藏、新武器延迟显示而手臂持续渲染。现有 Montage 的切换保留旧武器为 `PendingVisibleEquipment`；新姿势就绪时调用 `FinalizeUnequippedEquipment` 收起旧武器，并在同一回调显示新武器。状态序列为 Repair visible → Test visible → 切回等待时 Test visible/Repair hidden → 原子交换后 Test hidden/Repair visible；从未同时隐藏。截图 `AtomicEquipSwap_FirstRepairFrame.png`，快速 4 连切与 W=250/Shadow Leader 骨骼同步均通过。
