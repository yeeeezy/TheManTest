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
