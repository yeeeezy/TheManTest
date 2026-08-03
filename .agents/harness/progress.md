# 进度日志

## 当前状态

**最后更新：** 2026-08-03-session171

**当前功能：** FEAT-074 — VFXPack第一人称动画替换、HeadBob、武器摆动与RepairGun射击震屏

**状态：** in_progress

## 本轮完成

- session171：按原 VFXPack 实机 1280×720 截图重新对比 Idle/按 Shift+W 冲刺。相机恢复原版 HeadCamera `FieldOfView=77°`，保留原 `BodyRotator` 0.2s / Pitch -12.5°。另定位 RepairGun 漏复制原武器父节点 `RootOffset.Y=+11.660166`：原版网格相对 GripPoint 最终为 `(-0.000656,-5.097503,3.554176)`，而当前误为 `(0,-16.757669,3.554176)`。已将 `BP_RepairGun.StaticMesh` 恢复到原版最终变换并冷回读；Development Editor 编译成功。
- session170：找到原版枪械倾斜明显、当前项目几乎无反应的真正根因：迁移 AnimBP 时删除了对示例 `FirstPersonCharacter` 的硬 Cast EventGraph，C++ 只复制了 `PlayerLeanAmount/PlayerLookUpAmount`，遗漏原 EventGraph 随后使用的 `Lean_Sides_Offset=8.0` 与 `Look_Up_Offset=2.0`。现按原顺序在写入 AnimBP 前精确恢复 `Side×8` / `LookUp×2`，不添加任何组件位移或自创旋转。运行时探针实测 RepairGun A/D 前后旋转差由约 2.05° 恢复到约 10.24°，`hand_r` 由约 2.15° 恢复到约 6.61°。Development Editor 编译成功。
- session169：按用户要求撤销 session167–168 所有 A/D 可见性补偿。删除 `VFXMovementWeaponRollDegrees` 及任何 `ViewmodelRoot` / `ArmsViewMesh` 方向 Roll；普通移动严格只走原 Body Sway 变量和原 AnimBP `spine_03/hand_l` Modify Bone 链。`ViewmodelRoot` 只复刻原 `BodyRotator` 冲刺 0.2s / Pitch -12.5°，无位置变化。Development Editor 完整编译链接成功。
- session168：用户指出 session167 的 `ViewmodelRoot` Roll 仍然表现为横向平移。确认原因是该枢轴位于相机子级根部，旋转半径过大。现已将 A/D 可见性补偿的 ±6° Roll 移到 `ArmsViewMesh` 的骨架/胸口附近枢轴；`ViewmodelRoot` 只保留原版冲刺 0→-12.5° Pitch，位置始终固定。Development Editor 完整编译链接成功。
- session167：用户前台确认原 AnimBP 骨骼倾斜仍无可见旋转。原因是实际定向修正只有 `spine_03=±1°` 和 `hand_l=±0.5°`，在 RepairGun/110° FOV 构图下不可辨识。保留原 AnimBP 链的同时，现将同一 `CurrentVFXLeanSides` 以最大 ±6° 纯 Roll 应用到 `ViewmodelRoot`，与冲刺 0→-12.5° Pitch 合并；仍无任何横向位移。Development Editor / Win64 完整编译链接成功。
- session166：按用户确认重新深查 VFXPack 原角色蓝图。恢复完整 Body Sway 目标：`Clamp(MoveRight+MouseX,-1,1)` 与 `Clamp(-MoveForward-10×LookUp,-1,1)`，仍由 AnimBP 的 `spine_03/hand_l` Additive Modify Bone 负责普通移动与视角摆动。冲刺改回按键意图驱动的 0.2s 可逆过渡，同步插值速度 550→750，并在 `BodyRotator` 等价枢轴 `ViewmodelRoot` 上整体压枪 Pitch 0→-12.5°；不再依赖实际速度阈值，不再错写入脊柱单骨，且保持无 5cm 横向平移。同时将 `BP_MaintenanceWorker` 序列化的 HeadCamera FOV 从 100°改为 110°。Development Editor / Win64 编译成功，FramingCapture 1/1 Success，待前台 PIE 观感复核。
- session165：按用户最新决定取消额外的 5cm `ViewmodelRoot` Y 向横移；第一人称构图根节点固定回到 authored Transform，A/D 只保留原版 AnimBP 的 `spine_03/hand_l` Additive Roll。相机基础 FOV 改回 110°，删除 BeginPlay 硬覆盖，后续直接使用蓝图 Camera 组件原生 Field Of View。原输入 Clamp、2/8 插值与回正逻辑不变；Development Editor / Win64 完整编译链接成功，当前编辑器需重启后进行前台观感复核。

- session164：按用户要求将原版旋转职责归还 AnimBP。C++ 不再直接叠加 A/D Roll 或冲刺 Pitch；A/D 继续只写 `Lean_Sides_Amount`，冲刺按实际速度计算后并入 `Look_Up_Amount`（最多 -12.5°），由原 AnimBP 的 `spine_03 Modify Bone` 执行。此前用户认可的附加效果改为 C++ 最后叠加最多 5cm 的纯 Y 向左右位置偏移，不再通过远轴心 Roll 制造假平移。

- session163：修正 session162 只切换 Body Sway/CameraShake、未恢复真正冲刺压枪的问题。原版 `BodyRotator Timeline_2` 已确认使用 `RLerp(A=Identity, B=Pitch -12.5°)`；现将 `-12.5° * SprintVisualAlpha` 叠到 `ViewmodelRoot` Pitch，按实际水平速度在 WalkSpeed 550 到 SprintSpeed 750 间连续压低/抬回。左右移动原骨骼链及额外 2° Roll 保留。

- session162：再次核对原版侧倾节点：`spine_03` 与 `hand_l` 均为 Additive Roll，`RotationSpace` 未覆盖、采用 Modify Bone 默认 Component Space；枪随 `hand_r` 后代链绕脊柱轴心旋转。保留该原链及当前 2° `ViewmodelRoot` 补偿。冲刺视觉从 Shift 意图状态改为实际水平速度驱动：`WalkSpeed..SprintSpeed` 连续映射 0..1，Body Sway 插值速度由 2 连续过渡到 8，Running CameraShake 在速度比例达到 0.5 后切换；原地按 Shift 不再触发冲刺视觉。

- session161：按用户前台观感，将额外 `ViewmodelRoot` 最大移动 Roll 从 1° 微调为 2°；原 AnimBP 骨骼修正、2/8 插值速度与回正逻辑均不变。

- session160：重新核查原版完整侧倾来源。原 AnimBP 的定向 A/D 修正为 `spine_03` Roll 最大 1°、`hand_l` 再取 0.5 倍；Walking CameraShake Roll 振幅 0.2° 且调用 Scale=0.5（实际约 0.1°），Running CameraShake Roll=0°；`BodyRotator` 是冲刺/收枪过渡，Run 动画摆动不区分左右方向。因此撤销过强的自定义幅度，将额外 `ViewmodelRoot` Roll 从 3° 收敛为 1°，只用于补足当前构图下原骨骼侧倾的可见性。

- session159：用户前台确认 6° 左右移动枪械倾斜过强；最大 `ViewmodelRoot` Roll 收敛为 3°，插值速度、松键回正与原 AnimBP 1° 骨骼修正保持不变。待关闭编辑器后冷构建复核。

- session158：PIE 实测确认 `Lean_Sides_Amount` 在 A 输入时达到 `-1.0`，`spine_03/hand_l/hand_r` Pose 均发生变化，证明原 AnimBP 链路有效；但原骨骼 Roll 最大仅 1°，在当前 RepairGun/FOV 构图下肉眼不可辨。保留原骨骼修正，并新增同一平滑侧移量驱动 `ViewmodelRoot` Roll（满输入 6°），使左右移动枪械倾斜清楚可见。C++ 编译通过，冷构建仅因运行中的 UnrealEditor 锁定 DLL 而在链接阶段失败，需关闭编辑器后重跑并前台复核。

- 修复 VFXPack 移动倾斜驱动：不再用 CharacterMovement 速度归一化近似输入，改为缓存 Enhanced Input 的原始 A/D/W/S 轴，普通移动/冲刺继续按原版 2/8 插值，Completed/Canceled 后平滑回正。
- 修正前后倾斜写入名：`Look_Up_Down_Amount` 改为原版 AnimBP 实际读取的 `Look_Up_Amount`。
- 只读导出正式 `ABP_MaintenanceWorker_FirstPerson_Original`，确认 `spine_03` Additive Roll/Pitch 与 `hand_l` 0.5× Roll Modify Bone 均保留且连接；Development Editor / Win64 冷构建成功。待用户前台确认 A/D/W/S 可见倾斜。

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

## 2026-08-02 session154 交接

- session153 的自创 Viewmodel 波形与 `ShadowBodyMesh=ArmsViewMesh` 结论均已撤销；影子和腿恢复跟随 `CharacterMesh0`，修复第一人称视野中全身错位及影子上半身朝向异常。
- 移动反馈直接复用 VFXPack 原版 Walking/Running CameraShake 资产与原蓝图调用语义；C++ 只代替原蓝图内部状态更新，不强行替代资产。
- `ABP_VFXPack_FirstPerson` 由真实 Velocity 驱动 `Is_Moving/Is_InAir/Character_Speed`；MaintenanceWorker 运行值为 Walk 550、Sprint 750、Acceleration 2000、Braking 750。
- 真实 W 输入已重复验证 Idle→Run→Idle，长按期间手骨 Pose 持续变化；冷构建、`git diff --check`、UTF-8 JSON 解析与 FramingCapture 第 3 次运行均成功。
- 动态验证截图：`Saved/Screenshots/WindowsEditor/TMT_ExactVFX_Walk.png`。仍需用户在自己的前台游戏窗口按参考图主观确认运动节奏与最终画面对齐。

## 2026-08-02 session155 交接

- 第一人称现已直接运行原 VFXPack `FirstPerson_AnimBP` 的完整 AnimGraph/状态机与原动画资产，不再使用此前重建版；仅删除会拖入整套示例工程的 EventGraph Cast，三个驱动变量由 C++ 等价写入。
- 原版手臂 Mesh/Skeleton/Physics/材质和动画共 13 个最终资产已整理到 `/Game/Characters/MaintenanceWorker/FirstPerson/`，供应商目录冷重启后确认不存在。
- 原版武器插槽精确名称已修正为 `GripPoint`，自动化直接检查 Socket 存在、装备声明和实际挂载三项。
- 冷构建及冷启动 FramingCapture 1/1 成功；真实移动时原版 AnimClass、550 速度与三个状态变量已回读。最新动态截图：`Saved/Screenshots/WindowsEditor/TMT_OriginalAnimBP_Walk_Cold.png`。

## 2026-08-02 session156 交接

- `CharacterMesh0` 与 `ArmsViewMesh` 已恢复使用同一个原版 VFXPack AnimBP；Shadow/Legs 继续跟随 CharacterMesh0，修复影子上半身与第一人称姿态分叉。
- 原版 `Walk_Run_1D` 确认为 1D 速度轴；左右偏移不是 BlendSpace Direction 轴，而是 `Body_Sway -> Lean_Sides_Amount`。已按原参数恢复：侧移 Clamp `[-1,1]`，Walk 插值 2、Sprint 插值 8；不恢复 MouseX 枪械滞后。
- PIE A/D 实测 Lean 分别为 `+0.9782/-0.9782`，两个 AnimInstance 的 AnimClass、速度和 hand_r Pose 一致；Shadow Leader=`CharacterMesh0`。
- 冷构建、FramingCapture 1/1、蓝图编译和全部资产保存完成。截图：`TMT_VFXPack_StrafeRight.png`、`TMT_VFXPack_StrafeLeft.png`。


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
