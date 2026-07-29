# 进度日志

## 当前状态

**最后更新：** 2026-07-29-session118
**当前功能：** **FEAT-051（原始骨架角色与 Enemy 动画蓝图，当前穿插扫描测试）**
**会话编号：** 118

用户重新要求临时测试扫描能力。`DefaultAbilityClasses` 已从潜行者专属类上移到 `AFPSCharacterBase`，所有玩家角色蓝图共享同一角色默认技能数组及授予流程；完整 Development Editor 构建和 Live Coding 均通过。重启后潜行者原扫描配置保留，维修工继承字段正常；维修工已临时配置扫描技能、`MPC_ScanEffect` 与地形材质。PIE 已看到绿色扫描环/全场轮廓效果，并通过 W 移动与连续两次跳跃回归。下一步由用户前台按 E 验证实际手感与视觉。

用户已手动删除一部分效果不佳的重定向动画和动画蓝图。现有 C++ AnimInstance、无骨架 Template AnimBP 和状态机驱动架构继续保留。

玩家仍让 `GetMesh()`、`ArmsViewMesh` 与武器 Linked Anim Layer 共用玩家 Skeleton；玩家下半身可使用效果合格的重定向动画。Enemy 优先使用各自动画原始 Skeleton，并从无骨架 Template AnimBP 创建对应骨架的子 AnimBP。

---

## 当前完成项

- [x] 将玩家角色默认技能数组与授予逻辑上移到 `AFPSCharacterBase`；潜行者和维修工蓝图编译通过，维修工临时获得扫描技能。恢复 9 个扫描专用材质依赖，运行时 Decal 正式纳入 Actor 组件生命周期；截图 `ScanTerrain_MaintenanceWorker_Working.png` 验证绿色扫描环与轮廓效果。W 移动和连续两次跳跃均正常落地。

- [x] 新增动画重定向项目边界：`TheManTest` 只接收最终动画；所有 IK Rig/IK Retargeter/批量重定向操作必须在 `D:\Unreal Projects\TMIIR` 或 `D:\Unreal Projects\FPSShooter1` 等外部资源项目完成，主项目不得保留源骨架、源 Mesh 或重定向中间目录。
- [x] 删除未采用的 GASP 落地动画及主项目中的 `Animations/Retargeting`、迁入的 UE4/UEFN 源资源；恢复旧 Land 覆盖后再安全删除，无断引用。
- [x] 用户在 `FPSShooter1:/Game/CodexRetargeting` 手动生成有效的 `RTG_MM_Jump`、`RTG_MM_Fall_Loop`、`RTG_MM_Land`。先前自动生成的 Shooter Rifle 产物实为 T-Pose，已解除引用并删除。手动产物迁入暂存后通过目标 `SKM_UE4Mannequin` 的 `AnimationLibrary.get_bone_poses_for_time` 在 0/25/50/75/99% 五个时间点验收：三段均有 5 个唯一姿势，综合关键骨骼变化量分别约 44.8/3.5/22.7。随后用 Consolidate 更新引用并原位替换维修工 `AnimationSequenceBody` 同名资源，清理暂存目录，重编译父子 AnimBP。PIE 起跳画面非 T-Pose；落地后 Arms/Shadow/Legs 可见，CharacterMesh0 的不可见为第一人称 OwnerNoSee 预期行为。

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
- [x] 修复切枪 Montage 前最终持枪位置闪帧：保留完整 Unequip/Equip 技能与层生命周期，在切换调用当帧立即播放 Montage 覆盖 Idle，再于 next tick 从 0 稳定重启，桥接 Linked Layer 初始化窗口；起始姿势只多保持一帧。快速切走用 0.01 秒 Blend Out 停止旧 Montage。PIE 状态严格为 Arms/Body `playing@0 → playing@0 → playing@0.3333`，首个评估截图位于下方起始姿势；结束后同层/hand_r 一致、无 T-Pose，W=250 与影子同步通过。
- [x] 优化切枪顺滑度：用 UE 5.7 原生 Linked Anim Graph Blending 替换上述重复 Montage 桥接。`WeaponAimOffset` / `WeaponUpperBody` 设 0.1 秒 Blend In/Out，主 `TABP_BodyLocomotion` 在 `FullBodySlot` 后新增 Inertialization；切枪现仅于 next tick 从 0 播放一次 Montage。PIE 严格序列为 `inactive → active@0 → active@0.3333`，Arms/Body `hand_r` 逐帧一致；结束后 RepairGun 层有效、W=250、Shadow Leader/腿骨同步通过。待用户前台正常帧率实机确认主观丝滑度。
- [x] 用户确认 next-tick-only 版本手感仍未改善后，进一步消除切枪调用帧的新武器 Idle 窗口：当帧立即起播，next tick 按真实已流逝时间恢复，不再重回 0。节流 PIE 严格序列为 Arms/Body `active@0 → active@0.3333`，对应前台 60 FPS 约 `0 → 0.0167`，hand_r 一致；Live Coding 与 Development Editor 完整构建均通过。待用户再次实机确认。
- [x] 完成开局首次装备与后续切回的同 PIE 逐帧对比：旧 TestGun 到首个可见 RepairGun Equip 姿势曾产生约 32.7cm 跳变。现由 Arms/Body AnimInstance 在解链前保存 `WeaponTransitionPose`，主 AnimBP 末端以 `WeaponTransitionAlpha` 从旧 Pose 直接桥接到暂停的 Montage 0 秒低位姿势，随后才恢复 1x 播放。60 FPS 轨迹在桥接阶段 Z 连续为 `111.68→114.81→118.61→122.90→127.46→131.13`，Montage 随后 `0→0.0167→0.0333` 单调推进，没有先放下再拿起；连续画面检查未见旧枪残影，原子换枪帧通过 Camera Cut 清除 TAA/TSR 历史。Arms/Body 同步、可见性互斥、无 T-Pose。Live Coding 通过，待用户前台手感确认后执行最终完整构建。
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
- [x] 修复维修工落地时全身短暂消失：`RTG_MM_Land` 从 Local Space Additive 恢复为 No Additive；60 FPS PIE 连续 2 次跳跃逐帧审计均无骨骼归零。

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

FEAT-056 主动暂停，等待用户回来继续。当前不是技术阻塞。

迁入资产已暂存在 `/Game/ShapesFX_Pack`，尚未整理到正式目录，也尚未应用到 RepairGun 子弹或潜伏者扫描。主项目命令行加载 `TestMap` 时被既有的 `A_HandFire` 无有效 Skeleton 问题中断，因此地形类型仍需在现有编辑器/MCP 中只读确认；不得借本功能范围自动修复该无关动画。

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

## Session117 handoff - 直接使用原版 Cube Mesh（2026-07-29）

- 按用户最终要求，从 TMIIR 直接迁入原始 `SM_Geo_Cube`，正式路径为 `/Game/Actors/Interable/InteractableBase/Mesh/SM_InteractableBase_OriginalCube`。
- `BP_InteractableBase.StaticMesh` 已改用原版 Mesh、原版 DemoMap 的 `0.5` 缩放和现有 Cube_03 材质实例；原 Mesh bounds 为 ±261.25 cm，最终显示尺寸 261.25 cm。
- 实际 Unreal 截图：`Saved/Screenshots/Cube03_OriginalMesh_Final.png`。临时 Actor 已销毁，TestMap 未保存。
- 无引用的自制 `SM_InteractableBase_EffectCube` 已在定向引用检查后删除；Blender 源文件仅保留在外部 Blender 工程供历史参考。

## Session116 handoff - Cube_03 实机截图验证修复（2026-07-29）

- 用户截图证明首版低面 Cube 只显示少量金色纵条；实际材质已赋值，但 Mesh 数据不兼容。
- 对比原项目 `SM_Geo_Cube`：原版 1088 顶点 / 5832 面、UV `-1..1`；首版自制 Cube 仅 96 顶点 / 98 面、普通 `0..1` UV。Cube_03 的 Mask、NormalPush 与 Shrink 效果依赖原版规则拓扑/UV。
- 最终 Blender 版本保留原版拓扑、法线和 UV，只把尺寸归一为 100 cm 并将枢轴修正到底面中心；Unreal Bounds 仍为 `(-50,-50,0)` 至 `(50,50,100)`。
- Unreal 实际视口截图 `Saved/Screenshots/Cube03_Unreal_Verification_Compatible.png` 与近景 `Cube03_Unreal_Verification_Close.png` 已确认完整白金高密度格纹出现，不再是纵条。
- 临时 `TEMP_Cube03_Verify` Actor 已立即销毁，TestMap 未保存。

## Session115 handoff - 简约 Cube + 原版 Cube_03 特效（2026-07-29）

- Blender 新建 `SM_InteractableBase_EffectCube`：100 × 100 × 100 cm、底面中心原点、1.8 cm 三段倒角、Cube UV、单材质槽；源文件、FBX、脚本和预览均位于 `D:\Blender Projects\InteractableBase`，预览已自动打开。
- 从获批资源项目 `D:\Unreal Projects\TMIIR` 迁入原版 `MI_ShapesFx_Cube_03` 及真实依赖；参数审计确认白色正面、暖金轮廓、`T_Mask_09`、动画速度约 0.45。
- 正式材质为 `/Game/Actors/Interable/InteractableBase/Effects/Materials/MI_InteractableCubeEffect`；`BP_InteractableBase` 已切换到新 Mesh 和唯一材质槽，编译保存成功。
- Unreal Bounds 回读为 min `(-50,-50,0)` / max `(50,50,100)`，确认坐标与底部枢轴正确。
- 旧 Icosahedron 实例及其专属 MatCap09/OutlineIcosahedron 在零引用确认后已删除；RepairGun 未修改，TestMap 未保存测试 Actor。
- 迁移过程中重新出现的 6 个 `/Game/ShapesFX_Pack` 暂存副本均无项目外引用，已按供应商目录清理规则删除；正式资产只保留在 InteractableBase 语义目录。

## Session114 handoff - InteractableBase 默认 Mesh 特效（2026-07-29）

- 用户确认撤回 FEAT-056 的 RepairGun 子弹与 Infiltrator 扫描特效；相关代码、蓝图和材质目录已恢复到 `fb48d59`，RepairGun 不再使用 ShapesFX。
- 用户随后明确要求把原 Icosahedron 特效用于 `BP_InteractableBase` 默认 Mesh。9 个实际依赖已通过 AssetTools 迁入 `/Game/Actors/Interable/InteractableBase/Effects` 并语义化命名。
- `BP_InteractableBase.StaticMesh` 保留槽 0 面板材质，槽 1 使用 `MI_InteractableDefaultEffect`；蓝图已编译保存并确认依赖。
- `BP_RepairGunBullet.BulletMesh` 仍使用 `M_RepairGun_Bullet`，`/Game/Weapons/RepairGun/Effects` 无资产；没有向 TestMap 保存测试 Actor。
- 写入前本地安全检查点：`8a99766`。当前 active feature 恢复为 FEAT-051；FEAT-056 原方案已撤销，禁止重新应用。

## Session112 handoff - FEAT-056 暂停（2026-07-29）

- 当前 active feature 已切换为 `FEAT-056`；`FEAT-051` 仅暂停，现状不变。
- 已在 harness 增加正式规则：外部资产迁入后去除供应商/素材包目录，按“功能归属优先、资产类型次之”整理，专属资源归所属角色/武器，共享资源才进 `/Game/Effects/_Shared`。
- 写入前安全检查点为 commit `481198e`，包含此前 FOV 110 与 FEAT-051 文档；用户的 `BP_MaintenanceWorker.uasset` 明确排除，仍是未提交脏资产。
- TMIIR 两个请求材质及完整依赖已通过 Unreal AssetTools 迁入主项目，目前 11 个资产全部只在临时 `/Game/ShapesFX_Pack` 下。没有资产进入目标 `Effects` 目录。
- 目标正式目录：共享母材质/函数/通用纹理放 `/Game/Effects/_Shared/{Materials,Functions,Textures}`；Icosahedron 实例及专属纹理放 `/Game/Weapons/RepairGun/Effects/{Materials,Textures}`；Cube 实例及专属纹理放 `/Game/Characters/Infiltrator/Effects/Scan/{Materials,Textures}`。
- 资产整理并应用到 `BP_RepairGunBullet` 的命令被用户中断。核对后 `BP_RepairGunBullet` 未变脏，说明材质赋值尚未落地；恢复时不要重新迁移，只从临时路径执行一次 AssetTools move/rename，再验证结果。
- 共享母材质已核对为 Opaque + Unlit Surface。Cube 实例能否直接作为地形叠加尚未验证，不得直接替换地形主材质。
- commandlet 加载 `TestMap` 时被无关旧资产 `/Game/Weapons/TestGun/Animation/Sequence/A_HandFire` 的 Invalid Skeleton 错误中断；保持范围，不自动修复。用户回来后优先利用已打开的编辑器/MCP确认实际地形 Actor/材质槽，再决定 Landscape overlay、静态地面材质叠加或项目包装材质。
- 本轮不删除 staging 资产、不 Fix Up Redirectors、不提交结果。恢复前先检查是否仍有 UnrealEditor/UnrealEditor-Cmd 进程和当前 Git 状态。

## Session104 handoff - FEAT-051 active (2026-07-28)

- 当前 active feature 是 `FEAT-051`。
- 切枪顺滑度方案已改为原生 Graph Blending + 末端 Inertialization；Montage 只在 Link 稳定后单次播放。编译与 PIE 定量验证通过，下一步是用户正常前台帧率实机手感确认。
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
- 早期 harness 证实旧版顺滑切枪依赖 `SetAnimInstanceClass → LinkAnimClassLayers → Montage_Play`，会整体替换角色 AnimBP；当前统一 locomotion 架构不能恢复该路径。
- 尝试让主 AnimInstance 内部消费请求，逐帧验证发现仍被同次 Linked Layer 初始化清除，已完整回退，未保留无效代码。
- 最终方案为首帧桥接：完整 Unequip/Equip 后当帧 `PlayEquipMontage()`，同时安排 next-tick 当前装备校验后再从 0 播放。快速切走时旧装备 active Montage 用 0.01 秒 Blend Out 结束；0 秒 Stop 经测试会阻止同 Montage 立即重播，已禁止。
- 最终逐帧证据：正常切回 RepairGun 后 Arms/Body 均 `true@0 → true@0 → true@0.3333334`；第一张评估截图 `EquipBridge_FirstEvaluated.png` 显示枪位于视野下方起始姿势。快速切走后旧 Montage 立即 inactive，切回桥接仍成立。结束后 RepairGun 层有效、两 mesh hand_r 完全一致；截图 `EquipBridge_PostMontage.png`，W=250、Shadow Leader=`CharacterMesh0`。
- 用户最终实机复测确认功能问题已消失，但主观观感仍不如游戏开始时首次装备丝滑；本轮到此暂停。后续优化应直接对比 BeginPlay 首次装备与切枪的动画评估时序，避免重新引入隐藏模型、T-Pose 或技能层重叠。
- Harness 功能索引已拆分：`feature_list.json` 只保留 20 个非 done 条目，35 个完成项迁入 `feature_archive.json`；启动无需加载历史索引。迁移后 55 个功能 ID 全部唯一，`FEAT-051` 在主索引中唯一匹配。

## Session108 handoff - 落地消失回归修复（2026-07-28）

- 用户暂停切枪丝滑度调整，转而排查跳跃落地时人物消失；现有切枪 WIP 未继续改动。
- 修复前 60 FPS PIE 逐帧审计确认 Actor 和 `ArmsViewMesh` / `ShadowBodyMesh` / `LegsMesh` 始终可见且未 Hidden，但落地第 82 帧后各 Mesh 的 `hand_r` 依次归零，约到第 130 帧才恢复，说明是动画姿势塌缩而非渲染隐藏。
- 根因是 `/Game/Characters/MaintenanceWorker/Animations/AnimationSequenceBody/RTG_MM_Land` 被重新设置成 `AAT_LOCAL_SPACE_BASE`；`LocomotionSM.Jump_End` 将该加性差值当完整姿势播放，复现了 BUG-039-002。
- 已将该资产恢复为 `AAT_NONE` / `ABPT_NONE` 并保存；Jump 和 Fall Loop 原本即为 No Additive，无需修改状态机或 C++。
- 修复后同一次 60 FPS PIE 自动连续跳跃两次，落地帧为 82 / 222；四套 SkeletalMesh 从空中、落地到恢复 Idle 全程骨骼坐标有效，`zero_frames=[]`、脚本异常为空，可见性也未变化。

## Session109 handoff - Shooter 落地混合对齐（2026-07-28）

- 只读检查 `FPSShooter1` 证实官方 Rifle/Pistol ABP 实际引用 Unarmed `MM_Jump` / `MM_Fall_Loop` / `MM_Land`；当前迁入动画与官方来源相同，问题不在 Rifle 文件夹素材。
- 官方 Rifle 跳跃状态机为 Linear 混合，最短跳跃过渡 0.1 秒；当前 `To Land -> Jump_End` 是 0.2 秒 Hermite，会在物理接地后继续缓慢混合，造成落地拖软。
- 已仅把 `TABP_BodyLocomotion.LocomotionSM.AnimStateTransitionNode_10` 改为 `Linear / 0.1s`；Jump_End 到 Idle/Run 的 0.2 秒恢复保持不变。
- 父模板与 `ABP_MaintenanceWorker` 均由现有编辑器 MCP 编译保存并成功回读参数；PIE 连续触发两次跳跃，结束后角色、手臂、武器和影子正常，无消失或蓝图错误。截图：`Saved/Screenshots/WindowsEditor/JumpBlend_ShooterAligned_AfterTwoJumps.png`。
- 下一步只需用户在前台正常帧率确认落地触感；如果仍不一致，下一轮应抓接地前后逐帧 Pose/状态权重，而不是继续替换动画。

## Session110 handoff - 默认相机 FOV 110（2026-07-28）

- `AFPSCharacterBase.HeadCamera` 的统一默认 FOV 已设为 110；构造函数写默认值，BeginPlay 再写一次以覆盖角色 BP 可能保留的旧序列化组件值。
- 复用现有 Unreal 编辑器执行 Live Coding，日志确认 `Live coding succeeded`。
- PIE 运行时读取 `BP_MaintenanceWorker_C_0.HeadCamera.FieldOfView=110.0`，随后正常停止 PIE。
- 完整 Development Editor 构建尝试被当前启用的 Live Coding 拦截，属于编辑器运行状态限制而非编译错误；未关闭或重启用户现有编辑器。

## Session111 handoff - 相机偏移方案待明日继续（2026-07-29）

- 今天不再修改相机结构或蓝图；FOV 继续保持 110。
- 明天继续时不添加 SpringArm，保留 `Capsule -> HeadCamera -> ViewmodelRoot -> ArmsViewMesh`：在 `HeadCamera.RelativeLocation` 调真实视点，在 `ViewmodelRoot.RelativeTransform` 做反向补偿和最终手臂/武器构图。
- 手臂仍完整继承 HeadCamera 旋转；`ArmsViewMesh.RelativeTransform` 只用于模型导入轴向/骨架原点校正，不用来做日常画面构图。
- 当前工作区中的 `BP_MaintenanceWorker.uasset` 为脏状态，可能包含用户在编辑器内的调整；下次操作前先只读检查，不得盲目覆盖或纳入自动 checkpoint。
- 本轮未执行 Git 提交，所有 FOV、文档与可能的用户蓝图改动仍保留在工作区。
