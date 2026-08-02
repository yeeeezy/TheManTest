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
