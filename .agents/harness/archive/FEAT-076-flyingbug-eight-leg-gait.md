# FEAT-076 — FlyingBug2 六足交替步态修正

### 2026-08-21 session207 — MaintenanceWorker 第一人称动画 root 方向修复

- 用户截图证明当前迁入序列 root 约-90°、原资产为0°，session205 的“恢复原始-90°”结论作废。
- 在外部批准项目 FPSShooter1 确认7条最终动画首尾 root Yaw=0°并导出，TheManTest 仅导入最终动画；最终7/7回读为0°。Still 因0.066秒帧边界问题启用最近帧吸附后成为3采样键。
- 相关ABP、RepairGun Layer、BP_MaintenanceWorker编译保存，资产加载验证通过；FramingCapture在冷启动环境未注册，前台PIE视觉验收仍待用户完成。

### 2026-08-21 session206 — MaintenanceWorker 第一人称资产正式目录整理

- 经用户确认，只整理路径、不修改动画内容：使用 Unreal AssetTools 将 `Legacy/VFXPackFirstPerson` 的10个资产迁至正式 `FirstPerson/Locomotion`、`FirstPerson/Actions` 与 `FirstPerson/Logic`。
- 旧目录 Asset Registry 回读为0；相关 AnimBP、RepairGun Layer、BP_MaintenanceWorker 编译保存，目标资产加载验证通过。首次漏删磁盘空目录，用户指出后已删除空 `VFXPackFirstPerson` 与变空的 `Legacy` 父目录，并将双重清场要求写入规则。用户将在新目录手动修复动画。

### 2026-08-06 session205 — 撤销错误动画根旋转并恢复参考构图

- 用户将于次日继续做主观视觉复核；本轮不得标记为用户验收通过。后续以桌面 `微信图片_20260802100122_109_52.png` 和 `TMT_RestoredVFXPack_ReferenceCheck.png` 为对照继续调整。
- 撤销 session204 的 7 条第一人称动画 root `+90°` 烘焙；该方案会把游戏内手臂和枪整体转错，不能用于解决蓝图预览差异。
- 从修改前备份恢复 7 条序列，冷审计首尾 root Yaw 均为原始约 `-89.999977°`。错误版本另存于 `Saved/Backups/VFXPackFirstPerson_BadRootPlus90_20260806_2010`，可回退。
- PIE 截图 `Saved/Screenshots/WindowsEditor/TMT_RestoredVFXPack_ReferenceCheck.png` 与桌面参考图复核通过；BP CDO 与 PIE 的相机、ViewmodelRoot、手臂、身体、腿部相对 Transform 逐项一致，未使用 C++ Tick 锁定。
- Development Editor 构建成功；保存并正常关闭编辑器。

### 2026-08-06 session204 — VFXPack 第一人称动画基础方向修复

- 遵守 destination-only 边界，在批准的外部项目 `FPSShooter1` 完成方向审计和关键帧修复；TheManTest 内没有创建 IK Rig、IK Retargeter、源骨架或重定向工作目录。
- Fire、Idle、JumpStart、JumpLoop、JumpEnd、Run、Still 共 7 条序列的 root 原始首尾 Yaw 均约 `-89.999977°`；对 root 的全部采样关键帧烘焙逆时针 `+90°` 后迁入最终资产。
- TheManTest 冷启动全量审计 7/7 首尾 root Yaw=`0.0°`，Montage 与 BlendSpace 自动继承修正序列；Development Editor 构建成功。
- 备份：外部 `D:/Unreal Projects/FPSShooter1/Saved/Codex/Backups/VFXPackFirstPerson_20260806_194749`；目标 `Saved/Backups/VFXPackFirstPerson_20260806_195138`。

**创建日期：** 2026-08-04
**状态：** in_progress
**Archive 文件：** `archive/FEAT-076-flyingbug-eight-leg-gait.md`

---

## 功能概述

修正 FlyingBug2 虽登记八个 Locomotor 足端，但运行画面像少数腿同步颠动的问题。保留 Locomotor + Control Rig + FullBodyIK 架构，逐腿校正映射、相位和验证证据。

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/Private/Core/Tests/PhantomFeatureTests.cpp`
- 仅在运行时驱动确有必要时修改 `NightmareFlyingBug.h/.cpp`

**涉及蓝图资产：**
- `Content/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor`

**完成标准：**
- [ ] 八条腿的骨骼链、足端和 Locomotor Leg 映射逐项正确
- [ ] 八个足端使用蜘蛛式交错相位，移动时不会退化为少数腿同步颠动
- [ ] 自动化逐腿断言每个足端均存在有效位移、抬起与落地阶段
- [ ] Development Editor 冷构建、Control Rig/蓝图编译及平地与起伏路线 PIE 验证通过

## 实现日志

### 2026-08-06 session199 — 撤销玩家三视图验收并解除 C++ Transform 锁定

- 用户最新截图确认 `ArmsViewMesh` 上半身在蓝图正交视图中侧向分离，session198 的“三视图通过”结论作废。
- 根因之一是 `OnConstruction/BeginPlay` 的 `ApplyViewmodelFraming()` 与 Tick 每帧重写 `BodyRoot/ViewmodelRoot/ArmsViewMesh`，导致蓝图 Transform 修改进 PIE 后立即被 C++ 覆盖。
- 已删除静态 Transform 强制覆盖；BeginPlay 读取蓝图序列化的 ViewmodelRoot/ArmsViewMesh Transform 作为基线，Tick 只叠加冲刺旋转和移动惯性。Development Editor / Win64 冷构建 Success。
- 冷读回确认 BodyRoot 不再使用绝对旋转。尝试用组件 Yaw 强行把独立第一人称 Arms 拼到完整身体后，PIE 出现手臂消失或严重构图错位，说明剩余问题是第一人称资产几何/相机空间与完整身体空间的架构冲突，不得继续用 C++ 或蓝图位置常量伪造通过；实验 Transform 已撤销并恢复原可用构图。

### 2026-08-06 session200 — 保留独立 viewmodel，蓝图直接校正

- 用户明确要求保留独立 `ArmsViewMesh`，通过组件 Transform 校正，不改为完整身体第一人称。
- session203：按用户指定架构撤销 RepairGun 的 FP/Body Linked Layer 分流。旧 `AS_Rifle_A_Idle/Run` 全部引用合并到 `FPSShooter1` 外部生成并迁入的 `AS_VFXPack_FP_Idle/Run`；删除零引用 `ABP_RepairGun_BodyAnimLayer`，C++ 删除 `BodyEquipmentAnimLayerClass`。ArmsViewMesh 与 CharacterMesh0 现共享 `ABP_CharacterBase_Body + ABP_RepairGun_AnimLayer`，PIE 关键骨骼组件空间旋转一致。
- 蓝图最终写入 `ArmsViewMesh Location=(200,18.852108,-165)`、Rotation Yaw=`-90°`；身体、腿与 Arms 使用同一 Yaw 基准。
- 蓝图编译保存成功，PIE 实际截图 `Saved/Screenshots/WindowsEditor/TMT_IndependentArms_YawAligned_Raised_PIE.png`。基础 Transform 仅保存在蓝图，C++ 不再强制覆盖，等待用户前台观感确认。

### 2026-08-06 session201 — Bounds 三视图与 PIE 一致性复验

- 旧检查只对比组件原点，无法证明实际几何重合；现生成未保存的编辑器临时实例并读取 `SystemLibrary.GetComponentBounds`，逐轴比较 `ArmsViewMesh` 与 `CharacterMesh0` 的真实 Bounds 中心。
- 最终蓝图值：`HeadCamera=(-201.886,-121.126,77)`；`ArmsViewMesh=(200,120,-206.36)`、Yaw `-90°`。相反的相机/手臂平移保留独立 viewmodel 的相对构图，同时使编辑器世界空间几何中心重合。
- 最终误差：`X=-0.0001cm`、`Y=0.0003cm`、`Z=-0.0203cm`。PIE Runtime Report 确认 Arms 与身体世界 X/Y 完全同点，世界 Yaw 相同；最终截图 `Saved/Screenshots/WindowsEditor/TMT_Player_BPAndPIEAligned_Final.png`。

### 2026-08-06 — 玩家手臂、下半身与权威影子同轴

- 修正 BP 编辑器与 PIE 不一致：蓝图中清空已弃用的 `ShadowBodyMesh` / `ShadowUpperBodyMesh` 资产，唯一完整影子继续由隐藏的 `CharacterMesh0` 投射。
- 保留 VFXPack 手臂的前后与高度构图；`ArmsViewMesh.Y=+18.852108`，`HeadCamera.Y=-18.852108`，使手臂组件世界横向位置与 CharacterMesh0/影子严格同轴，冷读回误差 `0.000000cm`。
- `BodyRoot` 归零；`LegsMesh` 与 `CharacterMesh0` 均为 Location `(0,0,-90)`、Yaw `-90°`，保证下半身与脚底影子起点重合。
- 新增 PIE 横向轴线与腿根重合断言，并增加蓝图组件编辑器 Front/Side/Top 正交视图辅助接口用于可见验收。

### 2026-08-04 — 功能规划

- **背景：** 用户指出被配置的 `tent_low*` 实为头部而非脚。骨骼层级和参考姿势全局高度复核证明 FlyingBug2 有六条真实接地腿。
- **设计决策：** 保留现有解算架构，先逐腿审计实际输出，再校正相位；验收改为每条腿独立通过。
- **安全检查点：** `3f069cd`。

### 2026-08-04 — 教程结构复核与四相写入

- 用户截图确认教程链为基础 Idle → Control Rig；Rig 内为四个 Foot Set → 八个 FeetTransform 数组索引 → 八个 Full Body IK Effector。
- 原 Rig 虽有八个足端与八个 Effector，却错误按“左侧四足 Phase 0 / 右侧四足 Phase 0.5”分为两组，造成整侧同步颠动。
- 已清除误配的头部 `tent_low*` 与多余 GetFoot6/7，改为六条真实接地腿的两组三足组，PhaseOffset=`0/0.5`，重编译并保存。
- 逐腿自动化改为分别断言六条真实接地腿的 component-space travel、Control Rig travel 和垂直抬落范围，并生成连续相位证据图。
- 当前 DebugGame 编辑器仅约 3 FPS，编辑器内自动化等待交互帧率；已停止该轮排队测试。需关闭 DebugGame 编辑器后用 Development Editor 冷启动复跑。

### 2026-08-04 — 两轮冷启动逐腿验证

- Development Editor / Win64 冷构建 Success。
- 独立冷回读确认两个三足 Foot Set、`0/0.5` 相位、六个 FBIK Effector 与 GetFoot0-5 已写盘且无断链。
- `LocomotorCrawlEvidence` 连续两轮 Success；八个足端分别通过 component-space travel、Control Rig rig-space travel 和垂直抬落范围断言。
- `LocomotorSlopeEvidence` 连续两轮 Success。
- session188 的八足日志与结论作废；有效日志为 `Saved/Logs/SixLegGaitColdRound1.log`、`Saved/Logs/SixLegSlopeColdRound1.log`。
- 自动验证完成；功能保留 `in_progress`，等待用户前台确认最终运动观感。

## Bug 记录

### BUG-076-001 — 头部触须被误识别为足端

**发现日期：** 2026-08-04
**严重程度：** 高
**状态：** fixed，待冷启动回归

**根本原因：** 自动化仅累计全部足端位移并检查总阈值，没有逐腿活动、抬脚、落地与相位断言；Rig 同时错误把左右整侧分别放入两个 Foot Set。

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | 2026-08-04 | Success | 冷构建 |
| Control Rig/蓝图编译 | 2026-08-04 | Success | 当前编辑器重编译保存，随后独立冷回读 |
| PIE 平地六足逐腿验证 | 2026-08-04 | Success | 六条真实接地腿分别断言 |
| PIE 起伏路线六足验证 | 2026-08-04 | Success | 冷启动坡地验证 |

### 2026-08-04 session190 — Walk Source Pose 混合

- 按用户最新截图建立 `Anim_Nightmare_bug2_walk1 -> Control Rig -> Output Pose` 专用 AnimBP，撤销外置 ControlRigComponent 完整骨架覆盖。
- 平地冷启动验证六足继续抬落，头部触须 `tent_low1_left3` 累计运动 208.8cm；坡地验证 Success。
- 为避免 FBIK 翻过原 Walk 的自然弯曲面，六个主弯曲关节已配置 AngularStiffness=0.78 并成功写盘。六个 Effector 的 RotationAlpha 降为 0：Locomotor 只修正位置，尖足和关节朝向由原 Walk Source Pose 保留。平地/坡地冷启动均 Success。

### 2026-08-04 session191 — 蟹形支撑姿态严格复审

- 用户截图揭示 session190 的通过标准不足：运行时最低仅 2/6 足端处于低位，瞬时高差 162.1cm，前支撑腿被 FBIK 拉到约 124cm 的异常垂直范围。
- AnimBP 改为双 Walk Source：头、颈、躯干与非支撑附肢循环原 Walk；六条支撑链取 Walk 的全足低位帧，再交给 Control Rig，避免 authored 抬腿与 Locomotor 二次抬腿叠加。
- 六个足端仍全部登记在 Locomotor/FBIK。前支撑对 PositionAlpha=0.2，避免完全静止同时抑制旧 FBIK 拉飞；后四腿 PositionAlpha=1.0。步高降为4、空中占比0.22、MaxCollisionHeight=18、BobOffset=-35；RotationAlpha 保持0，保留原 Walk 朝向。
- 新增逐帧蟹形断言：每帧至少一组三足处于尖足低位支撑带、六足最大高差小于60cm、三对左右足不得交叉；继续逐腿检查独立运动与头部混合。
- `GroundedCrabFinalCrawl2.log` 与 `GroundedCrabFinalCrawlRepeat.log` 连续 Success：最低低位足3/6、最大高差56.0/56.5cm、头触须运动203.9/203.8cm；`GroundedCrabFinalSlope2.log` Success。亮场截图 `Saved/Screenshots/WindowsEditor/TMT_NightmareLocomotor_Crawl.png` 已人工复核，仍待用户前台主观确认。

### 2026-08-04 session192 — 三组腿对结构复刻

- 用户确认模型只有六条真实可接地腿，因此将教程四组/八足结构按模型等比例落实为三组/六足，而不是继续使用两组三足：前腿对 Phase 0、中腿对 Phase 0.333、后腿对 Phase 0.667。
- 六个 `GetFoot0..5` 数组索引与六个 FullBodyIK Effector 冷回读全部一一连通。Stepping 按教程截图写为 PercentOfStrideInAir 0.35、StepHeight 6、MaxCollisionHeight 1。
- 关闭占用资产的 DebugGame 编辑器后冷写盘成功；Control Rig 初始姿势审计显示六足端 Z 为 -3.73~5.31cm，三对左右分离并处于接地基础带。
- `ThreePairCrawlAudit1.log`、`ThreePairCrawlAuditRepeat2.log` 连续 Success：最低低位足均6/6，最大高差25.4/25.5cm，头触须运动170.6/169.8cm；`ThreePairSlopeAudit1.log` Success。第一次重复启动因 UE WebBrowser/CEF RHI 断言中断，单独冷重跑成功，非姿态失败。

### 2026-08-05 session193 — 位移、步频与步幅定量校准

- 从 UE 5.7 Locomotor 源码确认 `PhaseSpeed` 单位为 cycles/s，且 `StrideLength=CurrentSpeed/CurrentPhaseSpeed`。旧设置在运行速度120cm/s时为3.43 cycles/s、约35cm步幅，视觉上明显原地快速倒腾。
- 实测否决两档过慢方案：0.633 cycles/s 导致约190cm不可达步幅，四条中后腿不抬；1.129 cycles/s/约106cm步幅仍有三条腿拖地。未把失败参数留在资产中。
- 最终 `PhaseSpeedMin=0.8`、`PhaseSpeedMax=2.1`、`MinimumStepLength=12`；120cm/s时约1.852 cycles/s、65cm步幅，较旧版降频约46%，同时保持六相错峰完整抬落。
- `CadenceTuneCrawl3.log` 与 `CadenceTuneCrawlRepeat.log` 连续 Success：六足全部独立移动与抬落，中后足垂直范围3.2~3.6cm、前足29.7~31.3cm，最低6/6低位支撑，最大高差25.8cm；`CadenceTuneSlope.log` Success。四张时相截图已复核，无腿链反折或左右交叉，原Walk头颈混合保持运动。

### 2026-08-05 session194 — 前腿可见链驱动与最终投影回归

- 用户前台指出前腿几乎不动；根因是前腿 FBIK `PositionAlpha=0.2`，旧验收只看足端数值位移，没有保证整条五关节链肉眼可见。现六条腿统一 `PositionAlpha=1.0`。
- 四张连续时相图人工复核，两条前腿整链在支撑/摆动姿势间明显切换；六足垂直范围均3.3~3.4cm，平地与坡地测试均 Success。日志：`VisibleFrontLegCrawl.log`、`VisibleFrontLegSlope.log`。
- 同轮修复玩家影子空间分叉：shadow-only 枪体不再复用第一人称相机空间旋转，位置/缩放保留，方向交给 `CharacterMesh0.GripPoint`。`UpperBodyEvidence` Success，最终截图 `TMT_ShadowUpperBody_Runtime.png`已人工复核。

### 2026-08-05 session195 — 撤销错误朝向验收并在编辑器内复验

- session194 的“最终投影回归通过”不足以证明完整人物朝向正确，现撤销该结论。旧自动化只验证组件/枪体附着，没有验证身体模型前向，也没有验证 FlyingBug 的视觉头部是否跟随位移。
- FlyingBug2 参考姿势审计确认视觉前方是 Mesh 局部 `+Y`；地表对齐由 `MakeFromXZ` 改为 `MakeFromYZ`，Actor Yaw 直接取实际移动 Forward。`LocomotorCrawlEvidence` 新增 Mesh `RightVector`（局部 `+Y`）与实际位移方向点积 `>0.9` 的断言。
- 玩家第一人称与完整身体的装备动画层拆分：`EquipmentAnimLayerClass` 仅链接 `ArmsViewMesh`，新增 `BodyEquipmentAnimLayerClass` 链接 `CharacterMesh0`。RepairGun 身体层 `ABP_RepairGun_BodyAnimLayer` 使用第三人称瞄准 Idle/Run，避免第一人称手臂姿势覆盖完整人物。
- `CharacterMesh0`、`ShadowBodyMesh`、`LegsMesh` 正式统一为零相对旋转；在实际 Unreal Editor 中打开 `BP_MaintenanceWorker` 并冷读回三者 Rotation=`0/0/0`。编辑器证据：`Saved/Screenshots/TMT_BP_MaintenanceWorker_Viewport_Final3.png`。
- 运行时截图 `TMT_PlayerBody_ForwardViewport.png`、`TMT_ShadowUpperBody_Runtime.png`、`TMT_NightmareLocomotor_Crawl.png` 已人工复核。冷构建 Success；`FinalOrientationEvidence.log` 中 Crawl、Slope、UpperBodyEvidence 全部 Success；`FinalViewmodelFraming.log` 中 FramingCapture Success，证明第一人称构图未回归。

### 2026-08-05 session196 — 以 Actor 箭头纠正玩家身体 Yaw

- 用户指出 session195 编辑器箭头仍不一致，因此撤销该轮玩家身体“零旋转通过”的结论。MaintenanceWorker 身体资产与 FlyingBug 一样以 Mesh 局部 `+Y` 为视觉前方，必须通过蓝色 Z/Yaw `-90°` 对齐 Actor `+X` 箭头。
- 修正 Python 写盘参数：旧 `unreal.Rotator(0,-90,0)` 实际写入绿色 Pitch；现使用 `unreal.Rotator(0,0,-90)`。真实 Unreal Editor 冷读回 `CharacterMesh0`、`ShadowBodyMesh`、`LegsMesh` 均为 Pitch `0°`、Yaw `-90°`，证据 `TMT_BodyYawMinus90_Editor.png`。
- `UpperBodyEvidence` 改为断言 Mesh `RightVector`（资产局部 `+Y`）与 Actor Forward 箭头点积 `>0.99`。错误 Pitch 版本实际 Fail，正确 Yaw 版本 Success；最终人物正向与影子截图重新生成并人工复核。`FramingCapture` 同轮 Success。

### 2026-08-05 session197 — 箭头同框验收与交叉三足自然步态

- 在真实关卡编辑器视口生成临时 MaintenanceWorker，隐藏第一人称/影子重复 Mesh，只显示权威 `CharacterMesh0`；红色箭头表示 Actor `+X`，绿色箭头表示 Mesh 视觉 `+Y`。截图 `TMT_Player_Model_WithAlignedArrows.png` 中两者平行同向，编辑器计算点积=`1.0`。临时 Actor 未写入关卡并已清理。
- 用户指出爬行仍不自然。根因是 session192 的前/中/后三组左右腿对让每排两腿同相抬起，产生机械式三排摇摆。现改为标准交叉三足：左前+右中+左后 Phase0；右前+左中+右后 Phase0.5。
- 新配置冷写盘成功。18秒平地四张连续时相显示两组三角支撑交替而非横排同步；`LocomotorCrawlEvidence` Success，`LocomotorSlopeEvidence` Success，人物 `UpperBodyEvidence` Success。
