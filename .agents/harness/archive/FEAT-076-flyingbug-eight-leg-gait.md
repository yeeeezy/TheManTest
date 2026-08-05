# FEAT-076 — FlyingBug2 六足交替步态修正

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
