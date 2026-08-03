# FEAT-074 — VFXPack第一人称动画替换、HeadBob、武器摆动与RepairGun射击震屏

**状态：** in_progress

**创建：** 2026-08-01

## 来源审计

- VFXPack 跑步观感不是纯动画：`FirstPerson_AnimBP` 的 `Walk_Run_1D/FirstPerson_Run`（RateScale 1、无 Root Motion）叠加 Running HeadBob 与角色蓝图 Body Sway。
- `CamShake_HeadBob_Running`：Duration 10、Blend 0.1/0.2；Pitch 0.75@12Hz、Yaw 0.2@16Hz，Single Instance。
- `CamShake_HeadBob_Walking`：Duration 1、Blend 0.5/0.5；Pitch/Yaw/Roll 0.2@2Hz，Single Instance。
- Scout 的 `CamShake_Fire_Medium`：Duration 0.3、Blend 0.05/0.2；Pitch/Yaw/Roll 0.2@30Hz；XYZ 1@5Hz；FOV 0。
- 当前 TheManTest 已有 `ViewmodelRoot` 视觉隔离层和被临时禁用的视角 Sway 代码，适合在原链路恢复扩展。

## 确认方案

- 精确迁移三套 Camera Shake，只保留必要资产并按项目语义目录归档。
- 通用走/跑 HeadBob 放 `/Game/Characters/_Shared/Effects/Camera/`。
- RepairGun 射击震屏放 `/Game/Weapons/RepairGun/Effects/Camera/`。
- 移动/视角武器摆动仅作用于第一人称 viewmodel，不改变 gameplay 相机、弹道或碰撞。
- 用户于 2026-08-01 明确确认完整方案。
- 写入前安全检查点：`34fbfaf`。

## 第一阶段完成

- 三套 Camera Shake 已迁入项目语义目录；角色走/跑 HeadBob、程序化 Viewmodel 移动/视角摆动、RepairGun 开火震屏已经接入。
- Development Editor 编译、相关资产与蓝图编译验证通过；PIE 已确认移动时 ViewmodelRoot 产生预期视觉偏移。
- 第一阶段检查点：`b7ff993`。

## 扩展范围（用户已确认）

- 采用 VFXPack 的第一人称手臂骨架、手臂网格和动画观感替换 MaintenanceWorker 当前第一人称表现，但保留本项目 RepairGun、装备系统、GAS、枪口弹道和碰撞。
- VFXPack 的 `SK_Mannequin_Arms_Skeleton` 经审计实际含 pelvis、完整双腿及 IK 骨链；`SK_Mannequin_Arms` 网格几何仍仅为手臂，不能替代全身/腿网格。
- 所有骨架适配、动画重定向或换骨架副本只允许在 `D:\Unreal Projects\FPSShooter1` 的 `/Game/CodexRetargeting/VFXPackPlayer/` 隔离目录中完成和验证。
- TheManTest 只接收最终手臂 Mesh、目标 Skeleton 与最终动画，不迁入 IK Rig、IK Retargeter、源骨架、示例角色或工作目录。
- 最终继续使用现有主 ABP + RepairGun Linked Anim Layer 架构，换骨架后重新验证腿部 locomotion、第一人称动画、HeadBob、Viewmodel sway 与射击 CameraShake。

## 2026-08-01 session145 实施记录

- 在 FPSShooter1 隔离目录完成 VFXPack Skeleton 与现有玩家 Skeleton 的逐骨比对：68 根骨名称/顺序一致，参考姿势逐骨完全一致，因此采用无损 Skeleton 资产合并，不创建 IK Rig/IK Retargeter。
- VFXPack Skeleton 已成为 MaintenanceWorker 的统一 Skeleton；全身 Mesh、下半身 Mesh 与 20 套现有身体 locomotion 动画均冷启动回读为该 Skeleton。
- RepairGun 的 Idle/Run 已替换为 VFXPack `FirstPerson_Idle`/`FirstPerson_Run`；不再保留项目原有 Walk，而是完整复刻 VFXPack 原始混合：Speed 0=Idle×0.8、280=Run×0.5、420=Run×1.0、700=Run×1.5，权重插值速度 10。
- `BS_WalkRun_RepairGun` 保持现有 AnimBP 所需的 2D 类型，并把上述 4 档速度复制到 Direction -180/0/180，共 12 个有效样本；强制重建后相关 4 个蓝图编译保存成功。
- 其余 VFXPack Fire/Still/JumpStart/JumpLoop/JumpEnd/Recoil/原始 WalkRun 保存在 `/Game/Characters/MaintenanceWorker/Animations/VFXPackFirstPerson/`，供后续状态机/蒙太奇接线。
- VFXPack 手臂 Mesh 在当前第一人称三件套渲染链的 PIE 截图中不可见；按“VFX骨架和动画 + 本项目武器保持不变”的用户边界，恢复原项目手臂几何，保留 VFX Skeleton/动画。骨架参考姿势完全一致。
- 清理完成：TheManTest 内 `/Game/CodexRetargeting`、`TempCharacterArm`、`TempCharacterBody`、VFXPack 供应商目录均为 0 资产；未迁入 IK Rig/IK Retargeter。
- 蓝图编译通过：`ABP_MaintenanceWorker`、`ABP_RepairGun_AnimLayer`、`BP_MaintenanceWorker`、`BP_RepairGun`。
- Development Editor / Win64：Succeeded（Target is up to date）。
- PIE 冷启动确认 CharacterMesh0/ArmsViewMesh/LegsMesh 均加载 MaintenanceWorker 主 ABP，RepairGun Linked Anim Layer 同时存在于身体与 ArmsViewMesh；HeadBob、Viewmodel sway、RepairGun CameraShake 第一阶段配置保留。

## 2026-08-01 可见性回归修复

- 用户前台复核发现第一人称手臂与 RepairGun 均不可见。运行时定位并非 BlendSpace/Linked Layer 未加载，而是 VFX Skeleton 合并后项目原 `Grip_Point` Socket 丢失，RepairGun 实际回落到 ArmsViewMesh 组件原点；同时旧 Viewmodel 构图变换不适用于 VFX 手臂 Mesh。
- 写入前建立检查点 `3970105`。用户补充确认允许直接使用 VFXPack 原版第一人称 Mesh。
- 从 FPSShooter1 的最终资产迁入 `SKM_VFXPack_FirstPersonArms`，整理到 `/Game/Characters/MaintenanceWorker/FirstPersonArms/Mesh/`，并合并回项目统一 Skeleton；未保留迁移 Skeleton、供应商 PhysicsAsset、CodexRetargeting 或供应商目录。
- VFX 原版 `GripPoint` 保留；项目原 `Rifle_A` 与 `Grip_Point` 按修改前精确骨骼/变换恢复，其中 RepairGun 再次正确附着 `Grip_Point`。
- Viewmodel 采用 VFX Mesh 原版旋转，并针对本项目 110° 相机及 RepairGun 尺寸校准构图。TestMap PIE 截图确认手臂、RepairGun、移动 HeadBob 与移动动画均可见；相关 4 个蓝图编译保存、目标资产验证通过。
- 已知观感项：VFX 动画原本按素材包枪型制作，RepairGun 前握把几何不同，左手尚未精确贴合 RepairGun；如需完全贴握，应在外部动画资源项目制作 RepairGun 专属最终动画或另行确认程序化左手 IK。

## 2026-08-01 session146 T-Pose 根因修正

- 用户前台截图确认主项目第一人称手臂实际为完整 T-Pose；session145 关于“移动动画可见”的结论撤销，不能作为验收证据。
- PIE 隔离验证：同一 `SKM_VFXPack_FirstPersonArms` 直接播放 `AS_Rifle_A_Idle` 能正常弯臂；将 ArmsViewMesh 从 `ABP_MaintenanceWorker + RepairGun Linked Anim Layer` 临时切换为 VFXPack 原版 `FirstPerson_AnimBP` 后也立即脱离 T-Pose。根因确定为旧主 ABP/Linked Layer 最终输出，而非 Mesh、Skeleton、播放速度或 Viewmodel Offset。
- 将原版 AnimBP 整理为 `/Game/Characters/MaintenanceWorker/Animations/VFXPackFirstPerson/ABP_VFXPack_FirstPerson`，删除其示例工程专属 `FirstPersonCharacter` 侧倾/ADS/后坐力分支，仅保留 Pawn Velocity、Falling 和原版 Idle/Run/Jump 状态机；Idle、Run 与现有最终 Jump/Still/BlendSpace 资产统一引用项目语义目录及统一 Skeleton。
- `BP_MaintenanceWorker.ArmsViewMesh` 已持久化改用 `ABP_VFXPack_FirstPerson_C`；身体/腿仍保留 `ABP_MaintenanceWorker`，RepairGun/GAS/弹道未替换。
- 冷启动 PIE 证据：Idle 时 ArmsViewMesh 脱离 T-Pose；持续 420 cm/s 速度驱动时 `Is_Moving=True` 且进入移动状态。截图：`Saved/Screenshots/WindowsEditor/TMT_PersistedVFXAnimBP_Idle.png`、`TMT_PersistedVFXAnimBP_Run.png`。
- 迁移产生的供应商目录在删除前为 43 个剩余资产、0 个外部引用，已通过 Unreal 删除；正式 AnimBP 与角色蓝图资产验证通过。删除时引擎产生一次 `CurObject` handled ensure，编辑器仍响应，需在最终冷重启验证中继续检查日志。

## 2026-08-01 session147 构图与影子回归

- 用户指出 session146 虽解除 T-Pose，但第一人称模型仍离相机过远，且影子仍为 T-Pose；继续处理而非按“已知问题”停止。
- 原错误 `ViewmodelOffsetLocation=(302.4,100,-210)` 经端点测试确认 X 将模型推得过远；`X=80` 会贴脸过大。以原 VFXPack 拾枪截图为构图参考，最终收敛为 `(100,75,-200)`，使 RepairGun/手臂占据右下区域。由于本项目保留 RepairGun 而非素材包长步枪，几何轮廓和占屏宽度不会完全相同。
- PIE Idle/Run 截图：`Saved/Screenshots/WindowsEditor/TMT_VFXAligned_Idle.png`、`TMT_VFXAligned_Run.png`。420 cm/s 驱动时第一人称 AnimBP `Is_Moving=True`。
- 影子复核已恢复非 T-Pose 持枪姿势；运行时链为 CharacterMesh0=`ABP_MaintenanceWorker`，ShadowBodyMesh/LegsMesh 的 LeaderPose 均为 CharacterMesh0，ArmsViewMesh 独立使用 `ABP_VFXPack_FirstPerson`。

## 2026-08-01 session148 身体 T-Pose 最终修复

- 用户最新截图证明 session147 的影子判断仍错误；骨骼连续采样确认 CharacterMesh0 双臂旋转固定为 Skeleton 参考姿势，而 ArmsViewMesh 的双手在 300ms 内持续变化，第一人称 Idle 实际有播放。
- 隔离结果：解除 RepairGun Linked Layer、强制 `WeaponTransitionAlpha=0` 均无法解除身体 T-Pose；同 Mesh 直接播放 `RTG_MM_Idle` 立即正常，排除 Mesh、Skeleton 和动画序列。
- 根因定位为 `TABP_BodyLocomotion` 的 WeaponUpperBody Linked Layer 支路：`UpperBodyInPose` 原本无输入，且该层稳定输出参考姿势并以权重 1 覆盖 spine_01 以上。补接基础 Pose 后只在过渡期短暂正常，移动后仍回 T-Pose。
- 最终让第三人称身体从故障 WeaponUpperBody/AimOffset 支路旁路，`DefaultSlot.Pose` 直接进入 `UpperBodySlot.Source`；第一人称 RepairGun 继续由独立 `ABP_VFXPack_FirstPerson` 驱动。
- 冷启动后 Idle 与 420 cm/s Run 均验证：CharacterMesh0 hand_r 不再等于参考姿势，ArmsViewMesh `Is_Moving=True` 且双手跨帧变化。截图：`TMT_ShadowPoseFixed_Idle.png`、`TMT_ShadowPoseFixed_Run.png`。
## 2026-08-01 session149 — 统一主 ABP 与持枪姿势最终修复

- session146~148 中“第一人称独立 VFX AnimBP、身体绕过武器层”的方案已撤销；它违背项目既有架构，不能作为最终实现。
- `CharacterMesh0` 与 `ArmsViewMesh` 现均使用 `ABP_MaintenanceWorker`；`ShadowBodyMesh` 与 `LegsMesh` 继续通过 Leader Pose 跟随 `CharacterMesh0`。RepairGun 在身体和第一人称两边均链接同一个 `ABP_RepairGun_AnimLayer`。
- `TABP_Firearm_UpperBodyBase` 的 Idle 节点已指定 `AS_Rifle_A_Idle`；`TABP_BodyLocomotion` 已恢复 `DefaultSlot.Pose -> Layered Blend Per Bone.BasePose` 基础姿势链。
- 2D/1D BlendSpace Player 在当前模板继承链的 PIE 运行时持续返回参考姿势，尽管样本、Skeleton、Speed 输入及 Linked Layer 实例均有效。WalkRun 状态最终改为直接播放已验证有效的 `AS_Rifle_A_Run`，PlayRate=0.5；状态切换仍由武器层 Speed 驱动。
- PIE 真输入验证：移动时主 AnimInstance 与 RepairGun Linked Layer 在 `CharacterMesh0` / `ArmsViewMesh` 两边 Speed 均为 100；两边手骨输出有效 Run Pose，影子通过 Leader Pose 同步。Idle 时 `CharacterMesh0`、`ArmsViewMesh`、`ShadowBodyMesh` 的 hand_r 组件空间 Pose 完全一致。
- 证据截图：`TMT_UnifiedABP_Idle_Final.png`、`TMT_UnifiedABP_RunSequence.png`。五个相关 Blueprint 均已编译保存。

## 2026-08-02 session150 — VFXPack 视角构图重新标定

- 用户指出 session147~149 只验证了非 T-Pose 和动画输出，实际第一人称构图仍与 VFXPack 参考明显不一致；此前将 `(100,75,-200)` 记为“校准完成”的结论撤销。
- 使用 `TheManTest.Player.Viewmodel.FramingCapture` 从 `HeadCamera`、110° FOV 生成确定性 1920×1080 截图，并直接与用户的 VFXPack 参考截图对照枪口位置、枪身轴线、屏幕占比和手臂裁切。
- 四轮定量迭代后，MaintenanceWorker 最终采用 `ViewmodelOffsetLocation=(90,80,-185)`、`ViewmodelOffsetRotation=(0,-13,0)`：枪口从过低过右位置移到参考图的中心偏右区域，枪身恢复左上到右下的 VFXPack 轴线；RepairGun 几何短于参考长步枪，因此轮廓长度不作为伪一致性证据。
- 自动化断言同步更新；Live Coding 成功，`TheManTestEditor Win64 Development` 构建成功，`TheManTest.Player.Viewmodel.FramingCapture` 最终为 Success。确定性截图：`Saved/Screenshots/PlayerFramingCurrent.png`；实际 PIE 截图：`TMT_VFXPack_Reframed_Idle.png`。

## 2026-08-02 session151 — 按用户指定原项目与正确截图重做构图

- 用户指定唯一正确参考图为桌面 `微信图片_20260802100122_109_52.png`，参考项目为 `D:\Unreal Projects\UE389_MuzzleSource\VFX Pack - Stylized FPS Muzzle and Impacts Effects 5.1\VFXPack`；session150 使用了错误参考图，其 `(90,80,-185)` 结论作废。
- 正确参考图为 1059×597，和 1920×1080 确定性 SceneCapture 宽高比一致。量化目标：枪口约位于屏幕 `(58.5%,58.1%)`，武器主体从该位置延伸并裁出右侧和底部边界。
- 逐步把 Viewmodel 前向距离从 90 降至 60、30、0，并同步补偿横向和高度；最终采用 `ViewmodelOffsetLocation=(0,41,-155)`、`ViewmodelOffsetRotation=(0,-13,0)`。
- 最终 1920×1080 截图中枪口约为 `(57.9%,57.2%)`，RepairGun 主体延伸到右下边界，和正确参考的构图尺度与轴线一致；枪械几何仍为本项目 RepairGun，不伪装成参考项目长步枪。
- 正确参考截图已用系统查看器打开；确定性结果写入 `Saved/Screenshots/PlayerFramingCurrent.png`，实际 PIE 截图为 `TMT_CorrectVFXReference_Idle.png`。当前编辑器内嵌 PIE 面板为 1567×428 的异常超宽比例，因此不用于和 16:9 参考做像素位置验收。
- 自动化断言已同步为最终 Transform；Live Coding 成功，`TheManTest.Player.Viewmodel.FramingCapture` 为 Success，`TheManTestEditor Win64 Development` 冷构建成功。重启编辑器后蓝图资产验证通过，CDO 冷回读仍为 `(0,41,-155)` / `(0,-13,0)`。

## 2026-08-02 session152 — Rifle Physical 01 与原版第一人称上半身

- 用户要求暂停下半身速度调整，改用 `BP_Weapon_Rifle_Physical` 实际引用的 Rifle 01 Mesh，并优先保证玩家视角上半身与 VFXPack 一致。
- 审计确认无编号重定向器最终指向 `BP_Weapon_Rifle_Physical_01_Child`；通过 Unreal AssetTools 迁入 `SM_Weapon_Ballistics_Rifle_01` 及 4 个必要材质/函数/纹理依赖，并整理为 `/Game/Weapons/RepairGun/` 语义路径，主 Mesh 命名为 `SM_RepairGun_Rifle`。
- 原项目实测 Rifle 组件相对变换为 `(0,-16.757669,3.554176)` / 零旋转，MuzzleFlashLoc 为 `(0,58.509277,4.953239)` / Yaw 90；RepairGun 已按该值配置 StaticMesh，旧 SkeletalMesh 清空隐藏。
- `AFirearm::GetMuzzleWorldTransform()` 新增 Skeletal Socket → Static Socket → `MuzzleLocalTransform` 回退链；`UGA_Shoot` 统一使用该结果，保留弹体、Niagara、音效和调试射线的真实枪口位置。
- 第一人称 `ArmsViewMesh` 使用已迁入并精简的 `ABP_VFXPack_FirstPerson`，直接恢复 VFXPack Idle/Run 状态机及 0.8×/0.5×/1.0×/1.5×速度逻辑；身体与下半身仍维持项目现状。
- 原项目 `SK_ArmMesh` 相机变换实测为 `(-18.107912,18.852108,-150.00795)` / `(-3,-15,-1)`；因相机/画幅差异最终横向补偿为 41，即 `(-18.107912,41,-150.00795)` / `(-3,-15,-1)`。
- `TheManTestEditor Win64 Development` 冷构建成功；冷重启后 `TheManTest.Player.Viewmodel.FramingCapture` 为 Success。1920×1080 证据为 `Saved/Screenshots/PlayerFramingCurrent.png`，枪口约位于参考要求的中心偏右区域，枪体延伸并裁出右下边界。

## 2026-08-02 session153 — 原版移动反馈、Outline 组合与影子修正

- C++ 移除鼠标旋转滞后、自创方向移动偏移及 PlayerCameraManager 走跑 CameraShake；保留 VFXPack 动画主姿态，并按 Walking 0.2°@2Hz/0.5s 淡入、Running Pitch 0.75°@12Hz + Yaw 0.2°@16Hz/0.1s 淡入生成仅作用于 ViewmodelRoot 的波形。
- 迁入 `SM_Weapon_Ballistics_Rifle_01_Outline`，归档为 `SM_RepairGun_Rifle_Outline`。审计确认它是描边壳而非实体枪，因此按原版组合：实体 `SM_RepairGun_Rifle` + 无碰撞/无投影 `StaticMeshOverlay` 描边壳。
- 动画框架运行时逐骨审计定位影子朝向根因：Shadow 原先忠实复制 CharacterMesh0，但后者不是 VFXPack 第一人称持枪 Pose。Shadow Leader 改为 ArmsViewMesh 后，spine_03/hand_r/hand_l 组件空间 Pose 精确一致；LegsMesh 仍跟随 CharacterMesh0。
- Development Editor / Win64 冷构建成功；冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 成功，最终截图仍为 `Saved/Screenshots/PlayerFramingCurrent.png`。

## 2026-08-02 session154：按 VFXPack 原版逻辑重做移动动画与 HeadBob

- 撤销 session153 的自创 Viewmodel 正弦波方案；移动反馈直接使用已迁入的原版 `CS_Player_HeadBob_Walk` / `CS_Player_HeadBob_Run` CameraShake 资产。调用条件、频率与原蓝图一致：真实速度 `Size > 0` 时每帧 Start（依赖资产 Single Instance），Walk Scale=0.5、Run Scale=1.0、CameraLocal；退出状态立即 Stop。
- 撤销 session153 的错误影子结论：`ShadowBodyMesh` 与 `LegsMesh` 都必须跟随 `CharacterMesh0`，绝不能让全身影子跟随第一人称 `ArmsViewMesh`。该错误正是用户最新截图中身体/手臂错位的根因。
- `ABP_VFXPack_FirstPerson` 原状态机和 BlendSpace 保留；C++ 仅复刻原蓝图变量更新：`Is_Moving=Velocity.Size()>0`、`Is_InAir=IsFalling()`、`Character_Speed=Velocity.Size()`。
- MaintenanceWorker 运行时固定使用原示例参数：Walk=550、Sprint=750、MaxAcceleration=2000、BrakingDecelerationWalking=750，修复 BP 旧序列化 100/300 导致走路接近 Idle、看似不播放动画的问题。
- 鼠标左右旋转枪械滞后和自创方向移动偏移保持停用；RepairGun 使用 Rifle 实体网格叠加 Outline 壳。
- 冷构建成功；冷启动构图自动化第 3 次运行 Success。真实 W 输入验证 Idle→550 cm/s Run→Idle，AnimBP 的 `Is_Moving/Character_Speed` 正确切换且 `hand_r` 跨帧 Pose 持续变化；长按 2 秒与 5 秒采样均确认循环动画持续播放。运行时 Shadow leader 为 `CharacterMesh0`。动态截图：`Saved/Screenshots/WindowsEditor/TMT_ExactVFX_Walk.png`。

## 2026-08-02 session155：原版资产链替换与冷启动复核

- 撤销“精简重建 AnimBP 等同原版”的错误结论；从指定 VFXPack 原项目直接迁入原版 `FirstPerson_AnimBP`、全部状态机动画、`SK_Mannequin_Arms`、Skeleton、PhysicsAsset 与手臂材质。
- 原版 EventGraph 对示例 `FirstPersonCharacter` 的硬 Cast 会递归带入示例武器、HUD、Widget 和无关 VFX；保留原版 AnimGraph、状态机、Transition 与动画序列，删除该 EventGraph，由现有 C++ 每帧写入完全同名的 `Is_Moving`、`Is_InAir`、`Character_Speed`。
- 13 个最终资产通过 AssetTools 整理至 `/Game/Characters/MaintenanceWorker/FirstPerson/`；供应商目录已删除。删除过程出现 `CurObject != nullptr` handled ensure，但随后冷重启确认全部最终资产、角色 BP 与 RepairGun BP 可加载和编译，供应商目录不存在。
- RepairGun 插槽由错误的 `Grip_Point` 改为原版精确名称 `GripPoint`，并编译蓝图使 GeneratedClass/CDO 真正刷新；自动化新增 Mesh Socket、装备声明和运行时实际挂载三重断言。
- 冷启动运行时移动复核：速度 `550.0`、AnimClass=`ABP_MaintenanceWorker_FirstPerson_Original_C`、`Is_Moving=True`、`Is_InAir=False`、`Character_Speed=550.0`，`GripPoint` 存在；截图 `Saved/Screenshots/WindowsEditor/TMT_OriginalAnimBP_Walk_Cold.png`，活动相机 FOV=77。
- Development Editor / Win64 冷构建成功；冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 通过（1/1 Success）。确定性截图更新为 `Saved/Screenshots/PlayerFramingCurrent.png`。

## 2026-08-02 session156：统一第一/第三人称动画源与原版侧移 Lean

- 用户截图 `屏幕截图 2026-08-02 123929.png` 证明影子上半身仍与第一人称分叉。恢复项目原本的同源架构：`CharacterMesh0` 与 `ArmsViewMesh` 均使用 `ABP_MaintenanceWorker_FirstPerson_Original_C`；`ShadowBodyMesh`、`LegsMesh` 继续 Leader=`CharacterMesh0`。
- 审计原 VFXPack 后确认 `Walk_Run_1D` 是一维速度 BlendSpace，不包含左右方向轴。原版左右姿态来自角色 `Body_Sway`：MoveRight Clamp 到 `[-1,1]`，步行以 2、冲刺以 8 插值，再写入 AnimBP 的 `Lean_Sides_Amount`；前后输入同样写入 `Look_Up_Down_Amount`。按用户此前要求，未恢复 MouseX 枪械滞后。
- C++ 现在向第一/第三人称两个原版 AnimInstance 同步写入 `Is_Moving`、`Is_InAir`、`Character_Speed`、`Lean_Sides_Amount`、`Look_Up_Down_Amount`。
- PIE 右移实测 Lean=`+0.9782058`，左移=`-0.9782051`；两侧速度均 550。两个 Mesh 的 AnimClass 相同，`hand_r` 组件空间旋转/位移逐项一致（仅浮点百万分位误差）；Shadow Leader 冷回读为 `CharacterMesh0`。
- 证据截图：`Saved/Screenshots/WindowsEditor/TMT_VFXPack_StrafeRight.png`、`TMT_VFXPack_StrafeLeft.png`。Development Editor / Win64 冷构建成功；FramingCapture 1/1 Success；三个相关蓝图编译并保存，全部 Dirty Package 已保存。

## 2026-08-02 session157：修复移动倾斜驱动

- 用户前台确认 A/D 没有可见倾斜，撤销 session156 仅凭 `Lean_Sides_Amount` 数值就判定视觉有效的结论。
- 重新导出原版与主项目正式 AnimBP：原版通过 `spine_03` Additive Roll/Pitch 和 `hand_l` 0.5× Additive Roll 实现枪械随手臂倾斜；正式资产中的节点与连接仍存在。
- C++ 不再从 CharacterMovement 速度反推方向输入，改为缓存 Enhanced Input 原始二维移动轴；普通移动/冲刺仍按原版 2/8 的 `FInterpTo` 速度，输入 Completed/Canceled 时清零并平滑回正。
- 修正前后倾斜变量名：错误的 `Look_Up_Down_Amount` 改为 AnimBP 实际读取的 `Look_Up_Amount`。
- `TheManTestEditor Win64 Development` 冷构建成功。由于最终可见倾斜属于前台观感，仍待用户 PIE 复核 A/D/W/S；不得再仅以变量数值作为验收证据。
## 2026-08-02 session158

- 运行时探针确认 A 输入期间两个第一人称相关 AnimInstance 的 `Lean_Sides_Amount=-1.0`，原 `Modify Bone` 链确实改变 `spine_03/hand_l/hand_r`，此前“完全没倾斜”的视觉结果来自原骨骼修正只有最大 1°。
- 复核原角色 `BodyRotator` 后确认其 Timeline 是冲刺/收枪过渡，不是左右移动倾斜来源。
- 在保留原 AnimBP 细微骨骼修正、原 2/8 插值速度和输入回正逻辑的基础上，将 `CurrentVFXLeanSides` 同时应用到 `ViewmodelRoot` Roll，满输入幅度 6°，以匹配当前 RepairGun/FOV 下可辨识的枪械侧倾。
- `FPSCharacterBase.cpp` 编译成功；Development Editor 冷构建链接阶段因当前 UnrealEditor 占用 `UnrealEditor-TheManTest.dll` 失败，关闭编辑器后需重跑冷构建并用 A/D 前台确认方向和幅度。
## 2026-08-02 session159

- 根据用户前台观感反馈，将移动枪械根节点最大 Roll 从 6° 下调为 3°；不改变原 2/8 插值速度、输入回正或 AnimBP 骨骼细微修正。
- 当前 UnrealEditor 正在运行，需关闭后冷构建加载新幅度。
## 2026-08-02 session160

- 直接读取已迁移原版 CameraShake CDO：Walk RotOscillation Pitch/Yaw/Roll 均为 0.2°、原调用 Scale=0.5；Run Pitch=0.75°、Yaw=0.2°、Roll=0°。CameraShake 不是 A/D 定向侧倾的主要来源。
- 原版 A/D 定向侧倾仍以 AnimBP 的 `spine_03` 最大 1° Roll 为主，`hand_l` 为 0.5 倍；`BodyRotator` 仅处理冲刺/收枪，Run 动画只提供周期摆动。
- 将为当前 RepairGun/FOV 补可见性的额外 `ViewmodelRoot` 最大 Roll 从 3° 下调到 1°，避免明显超过原版。
## 2026-08-02 session161

- 用户确认 1° 补偿偏弱，指定将额外 `ViewmodelRoot` 最大 Roll 调整为 2°；其余侧倾链路参数不变。
## 2026-08-02 session162

- 复核原 AnimBP 导出：定向倾斜确实由 `spine_03` Additive Roll 驱动，随后 `hand_l` 叠加 0.5 倍 Roll；两节点均未覆盖 `RotationSpace`，采用 Modify Bone 默认 Component Space。当前原节点仍完整运行。
- 当前额外 2° `ViewmodelRoot` Roll 因轴心在相机子级根部会产生可见弧形位移；按用户要求保留，并与原骨骼旋转同时叠加。
- 将冲刺视觉选择从 `bIsSprinting`（Shift 意图）改为实际 `Velocity.Size2D()`：在 `WalkSpeed..SprintSpeed` 间计算连续 `SprintVisualAlpha`，Body Sway 插值速度 2..8 连续变化，Running CameraShake 于 alpha>=0.5 切换。移动速度未达到阈值时，即使按住 Shift 也不进入冲刺视觉。
## 2026-08-02 session163

- 用户复核发现 session162 后 Shift 无压枪反应；根因是此前只将 Body Sway 插值与 CameraShake 改为速度驱动，未在当前项目恢复原版 `BodyRotator` 冲刺旋转。
- 从原 `FirstPersonCharacter` 图中提取精确参数：`Timeline_2` 输出 Alpha，`RLerp` 从 Identity 到 `Rotator(Pitch=-12.5°, Yaw=0, Roll=0)`，再写入 BodyRotator Relative Rotation。
- 当前等价实现将 `SprintLoweringRotation.Pitch=-12.5° * SprintVisualAlpha` 与现有 ViewmodelOffset、左右 2° Roll 相加；alpha 由实际 `Velocity.Size2D()` 在 550..750 映射，原地 Shift 不压枪，加速/减速连续过渡。
## 2026-08-02 session164

- 架构按原版职责重排：C++ 只向 AnimBP 传 `Lean_Sides_Amount`、`Look_Up_Amount` 和速度；删除 C++ 对 `ViewmodelRoot` 的移动 Roll 与 Sprint Pitch 直接旋转。
- 冲刺速度比例产生的 -12.5° 压枪值并入 `Look_Up_Amount`，由现有原版 `spine_03` Additive Modify Bone 在 AnimBP 内执行；A/D 原版 Roll 链保持不变。
- 用户认可的额外“左右偏移”作为项目补充保留，但改为 C++ 在动画 Pose 后叠加 `ViewmodelRoot` Y 方向最大 5cm 位置偏移，不再使用组件 Roll。

## 2026-08-03 session165：取消额外横向平移

- 用户确认暂不使用项目补充的 5cm `ViewmodelRoot` Y 向横移；删除该常量及 Tick 中的平移叠加。
- `ViewmodelRoot` 恢复只使用确定性构图 Transform；A/D 姿态完全由原版 AnimBP 的 `spine_03` 与 `hand_l` Additive Roll 链负责，原输入 Clamp、2/8 插值和回正逻辑不变。
- `TheManTestEditor Win64 Development` 完整编译与链接成功；当前编辑器进程仍在运行，需重启后再做前台 A/D 观感复核。
- 相机基础 FOV 按用户要求从 77° 改回 110°，并删除 `BeginPlay` 的硬编码覆盖；后续角色蓝图可直接调整 HeadCamera 组件原生 `Field Of View`。构图截图测试改为读取运行时 Camera FOV，不再另写死捕获角度。
- FOV 与横移调整后的 `TheManTestEditor Win64 Development` 完整编译链接成功。

## 2026-08-03 session166：恢复原版完整 Body Sway 与冲刺 BodyRotator

- 重新导出并核对原 `FirstPersonCharacter.Body_Sway`：侧向目标为 `Clamp(MoveRight + MouseX, -1, 1)`，前后/俯仰目标为 `Clamp(-MoveForward - 10×LookUp, -1, 1)`；两者均以 Walk=2 / Sprint=8 的 `FInterpTo` 速度写入 AnimBP。
- 普通 A/D 最终仍使用原 AnimBP 的 `spine_03` Additive Component-Space Roll 与 `hand_l` 0.5 倍 Roll；不叠加 `ViewmodelRoot` 侧向位移或 Roll。可见观感还包含原 Run 动画、HeadBob 与鼠标 Body Sway，不能只用单个 1°数值解释。
- 原冲刺由按键意图驱动两条 0.2s 可逆 Timeline：MaxWalkSpeed 550→750；`BodyRotator` 用 `RLerp(Identity, Pitch=-12.5°)` 整体旋转手臂/武器。当前实现使用同一线性 alpha 同步驱动速度与 `ViewmodelRoot` 等价枢轴，松键从当前位置反向播放；空中拒绝开始冲刺。
- 取消 session162–164 的速度阈值驱动与“把 -12.5° 写入 `Look_Up_Amount`”的非原版路径；Running CameraShake 也恢复按 sprint 状态选择。
- 自动化首轮暴露 `BP_MaintenanceWorker` 仍序列化 HeadCamera FOV=100°；已直接改为 110°，使蓝图 Camera 原生默认值真正生效。`TheManTestEditor Win64 Development` 编译成功，`TheManTest.Player.Viewmodel.FramingCapture` 复跑 1/1 Success；尚需前台 PIE 对比普通 A/D、鼠标转向和冲刺压枪的最终观感。

## 2026-08-03 session167：强化普通移动的可见旋转

- 用户前台复核确认仅有原 AnimBP `spine_03=±1°` / `hand_l=±0.5°` 时仍观察不到旋转；不再把变量或骨骼 Pose 数值变化当作可见效果验收。
- 保留原 Body Sway 与 AnimBP Modify Bone 链，同时把 `CurrentVFXLeanSides` 以最大 ±6° 纯 Roll 应用到 `ViewmodelRoot`；与原版冲刺的最大 -12.5° Pitch 在同一枢轴合并。位置仍固定为 authored Transform，没有恢复 5cm 横向平移。
- Development Editor / Win64 完整编译和链接成功。

## 2026-08-03 session168：修正侧倾旋转枢轴

- session167 将 Roll 施加在相机子级 `ViewmodelRoot`，由于枢轴距枪械过远，屏幕观感仍是圆弧横移，与原版围绕胸口/持枪骨骼的倾斜不符。
- 撤掉 `ViewmodelRoot` 的 A/D Roll；该节点只保留原 `BodyRotator` 冲刺 Pitch。可见性 Roll 改在 `ArmsViewMesh` 相对旋转上与 authored `BaseArmsRotation` 合并，使枢轴靠近骨架根/胸口；无位置变化。
- Development Editor / Win64 完整编译链接成功。

## 2026-08-03 session169：删除所有方向倾斜补偿

- 用户明确要求既然 Skeleton、Mesh、AnimBP 与动画均来自原版，应直接复制原逻辑，不再进行任何项目侧可见性补偿。
- 撤销 session167–168 的 ±6° 组件 Roll；`ViewmodelRoot` 和 `ArmsViewMesh` 均不再从 A/D 直接获得旋转。方向摆动仅使用原 Body Sway 的 `Lean_Sides_Amount` / `Look_Up_Amount` 与 AnimBP Modify Bone 输出。
- 冲刺仍仅使用原 `BodyRotator` 等价层：0.2s 可逆 alpha 驱动 Pitch 0→-12.5°。Development Editor / Win64 完整编译链接成功。

## 2026-08-03 session170：恢复 AnimBP 遗漏的原版 Body Sway 倍率

- 对原/现 AnimBP 做 T3D 节点级对比：4 个 Modify Bone、`spine_03`、`hand_l`、变量 GUID 与输出连接均完整，资产迁移未丢节点。
- 真正缺失位于被删除的原 AnimBP EventGraph：它在从示例角色复制 `PlayerLeanAmount` / `PlayerLookUpAmount` 后，还会分别乘以 `Lean_Sides_Offset` / `Look_Up_Offset` 再写回。直接读取原版 CDO 确认精确默认值为 8.0 / 2.0。
- C++ 现按原顺序写入 `Lean_Sides_Amount=CurrentVFXLeanSides×8` 与 `Look_Up_Amount=CurrentVFXLookUpDown×2`；未添加任何组件 Roll 或位移。
- 运行时探针强制真实 A/D 入口与 Pose 评估：修复前 `hand_r` / RepairGun 角差约为 2.15° / 2.05°；修复后为 6.61° / 10.24°，证明原版明显旋转链已恢复并传递到最终武器。

## 2026-08-03 session171：原版 FOV 与武器挂点最终变换

- 修改 VFXPack `MainScene` 中最近出生点的 `BP_Weapon_Rifle_Physical_01_Child` 位置，使其可在开局附近按 E 拾取；实机采集 1280×720 Idle 与 Shift+W 参考图。
- 原项目 HeadCamera FOV 为 77°。当前 C++ 基础值和自动化断言已恢复 77°，不在 BeginPlay 硬覆盖蓝图 Camera 属性。
- 原武器实际挂载链为 `GripPoint -> Actor Root -> RootOffset(Y=+11.660166) -> WeaponGripLoc -> Weapon_MainMesh(Y=-16.757669,Z=3.554176)`，最终网格位置为 `(-0.000656,-5.097503,3.554176)`。项目先前只复制了末级 Mesh 变换，漏掉 RootOffset，造成持枪轴向偏差 11.66cm；`BP_RepairGun.StaticMesh` 已恢复为原版最终值。
- 原 `BodyRotator` 冲刺 0.2s / Pitch -12.5° 保持不变。Development Editor 编译成功，RepairGun 蓝图编译保存并冷回读正确。
