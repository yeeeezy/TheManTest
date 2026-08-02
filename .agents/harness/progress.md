# 进度日志

## 当前状态

**最后更新：** 2026-08-02-session153

**当前功能：** FEAT-074 — VFXPack第一人称动画替换、HeadBob、武器摆动与RepairGun射击震屏

**状态：** in_progress

## 本轮完成

- 第一阶段 HeadBob、Viewmodel sway 与 RepairGun CameraShake 已完成并保存在检查点 `b7ff993`。
- FPSShooter1 中完成 68 骨逐骨/参考姿势一致性验证；无 IK 重定向，VFXPack Skeleton 已成为 MaintenanceWorker 身体、腿与动画统一 Skeleton。
- RepairGun 不再保留原生 Walk；2D BlendSpace 精确复刻 VFXPack 的 Idle/Run 混合曲线（Speed 0/280/420/700，RateScale 0.8/0.5/1.0/1.5），按三个 Direction 档共 12 个有效样本。
- 撤销 session145 的错误动画验收结论：用户截图确认 ArmsViewMesh 为完整 T-Pose。
- 已定位根因是 `ABP_MaintenanceWorker + RepairGun Linked Anim Layer` 对 ArmsViewMesh 的最终输出；同 Mesh 手动播 Idle、以及直挂 VFXPack 原版 AnimBP 均能正常弯臂。
- 已将精简后的原版状态机整理为 `ABP_VFXPack_FirstPerson` 并持久化到 ArmsViewMesh；冷启动 PIE 的 Idle 与 420 cm/s 移动状态均脱离 T-Pose，正式资产验证通过。
- 供应商目录已清理；删除时出现一次 handled ensure，最终仍需冷重启日志复核。
- 第一人称构图按原 VFXPack 拾枪截图重新校准：`ViewmodelOffsetLocation` 从错误的 `(302.4,100,-210)` 调整为 `(100,75,-200)`；Idle/Run 均位于右下持枪区域。
- 影子已恢复非 T-Pose 持枪姿势；CharacterMesh0 继续使用主 ABP，ShadowBodyMesh/LegsMesh 正确跟随 CharacterMesh0，第一人称 ArmsViewMesh 独立使用 VFX AnimBP。
- 撤销 session147 的错误影子判断；用户截图与骨骼采样证实当时仍是 T-Pose。
- 已定位并修复 `TABP_BodyLocomotion` 的故障 WeaponUpperBody/AimOffset 覆盖链；身体改由稳定的 Locomotion Pose 直接进入 UpperBodySlot，Idle/420 cm/s Run 骨骼值与截图均不再 T-Pose。
- 第一人称 ArmsViewMesh 跨帧手骨变化且 Run 时 `Is_Moving=True`，确认 VFX AnimBP 确实在播放。
- 临时 Retarget、TempCharacter 与供应商目录已清场；四个相关蓝图编译保存成功，Development Editor 编译成功。

## 待办

- 冷重启编辑器后再次复核 Idle/Run、蓝图编译和日志，确认供应商目录删除时的 handled ensure 没有留下损坏引用。
- 前台实际输入复核走/跑/跳/开火观感；当前自动化已用持续 420 cm/s 速度验证 `Is_Moving=True`，但仍需真实 Enhanced Input 复核。
- 确认后决定是否把 VFXPack Jump/Fire/Recoil 继续接到现有状态机/蒙太奇；当前资产已整理但未强行覆盖本项目 RepairGun 装备/开火动作。
- VFX 动画按素材包枪型制作，RepairGun 前握把与左手尚未完全贴合；后续若要求精确贴握，需要外部项目制作专属最终动画或另行确认程序化左手 IK。

## 2026-08-02 session152 交接

- RepairGun 已换为 VFXPack `BP_Weapon_Rifle_Physical_01_Child` 使用的 Rifle 01 静态枪模，资产已归档到 `/Game/Weapons/RepairGun/`；旧骨骼枪模已清空隐藏。
- 第一人称 ArmsViewMesh 直接使用 `ABP_VFXPack_FirstPerson`，恢复原版上半身 Idle/Run 姿态与速度；下半身速度未调整。
- 最终 Viewmodel 为 `Location=(-18.107912,41,-150.00795)`、`Rotation=(-3,-15,-1)`；确定性 1920×1080 截图为 `Saved/Screenshots/PlayerFramingCurrent.png`。
- 静态枪模枪口回退已落入 `AFirearm::GetMuzzleWorldTransform()`；Development Editor / Win64 冷构建与 `TheManTest.Player.Viewmodel.FramingCapture` 均成功。

## 2026-08-02 session153 交接

- 已停用鼠标旋转枪械滞后、自创方向移动偏移与 gameplay 相机走跑 CameraShake；VFXPack 动画负责主要姿态，C++ 原参数波形只作用 ViewmodelRoot。
- Rifle Outline 作为原版附加描边壳与实体枪组合使用，不再单独替代实体枪。
- 影子上半身根因已修：ShadowBodyMesh Leader=ArmsViewMesh；运行时 spine_03/hand_r/hand_l 组件空间 Pose 完全一致。LegsMesh 仍跟随 CharacterMesh0，下半身速度未调整。
- 冷编译和 FramingCapture 均成功；最终截图 `Saved/Screenshots/PlayerFramingCurrent.png`。

## 工作区边界

- FEAT-073 安全检查点：`34fbfaf`。
- 本轮修改 MaintenanceWorker 最终 Skeleton/身体动画、RepairGun 第一人称 locomotion、既有 Camera/Viewmodel/开火反馈与 Harness；不包含 IK Rig/Retargeter 或供应商工作目录。
## 2026-08-01 session149

- 撤销第一人称独立 AnimBP 与身体绕过武器层的临时方案；CharacterMesh0/ArmsViewMesh 重新统一使用 ABP_MaintenanceWorker，并在两边链接同一 RepairGun Anim Layer。
- 修复 TABP_BodyLocomotion 的 DefaultSlot 基础 Pose 断线；RepairGun Idle 指定 AS_Rifle_A_Idle，WalkRun 改为直接播放已验证有效的 AS_Rifle_A_Run（0.5×），避免 BlendSpace Player 运行时返回参考姿势。
- PIE 真实 W 输入验证：主 ABP 与 Linked Layer 两边 Speed=100；第一人称、身体、影子均为有效 Run 持枪 Pose。Idle 时三者 hand_r 组件空间 Pose 完全一致。
- 截图：TMT_UnifiedABP_Idle_Final.png、TMT_UnifiedABP_RunSequence.png。

## 2026-08-02 session150

- 撤销 `(100,75,-200)` 已匹配 VFXPack 的错误构图结论；此前截图只证明动画有效，未证明视角一致。
- 以 110° FOV、1920×1080 的确定性 SceneCapture 与 VFXPack 参考图逐项对照，四轮校准后采用 `ViewmodelOffsetLocation=(90,80,-185)`、`ViewmodelOffsetRotation=(0,-13,0)`。
- 当前枪口位于参考图对应的中心偏右区域，枪身轴线为左上到右下；RepairGun 比参考长步枪短，不能用相同轮廓长度作为验收条件。
- Live Coding、Development Editor / Win64 构建和 `TheManTest.Player.Viewmodel.FramingCapture` 自动化均成功；截图为 `Saved/Screenshots/PlayerFramingCurrent.png` 与 `TMT_VFXPack_Reframed_Idle.png`。

## 2026-08-02 session151

- 用户指定桌面 `微信图片_20260802100122_109_52.png` 和原项目 `UE389_MuzzleSource/.../VFXPack` 为唯一正确参考；session150 的参考图与 `(90,80,-185)` 结论作废。
- 按正确 1059×597 参考图量化枪口目标 `(58.5%,58.1%)`，使用同宽高比的 1920×1080 SceneCapture 迭代。
- 最终采用 `ViewmodelOffsetLocation=(0,41,-155)`、`ViewmodelOffsetRotation=(0,-13,0)`；实测枪口约 `(57.9%,57.2%)`，RepairGun 主体按参考尺度延伸并裁出右下边界。
- 确定性截图为 `Saved/Screenshots/PlayerFramingCurrent.png`；普通 PIE 截图 `TMT_CorrectVFXReference_Idle.png` 因当前内嵌面板为 1567×428 超宽比例，仅用于可见性检查，不参与 16:9 构图验收。
- 自动化断言已同步；Live Coding、FramingCapture、Development Editor / Win64 冷构建均成功。重启后 BP_MaintenanceWorker 资产验证与最终 Transform 冷回读通过。
