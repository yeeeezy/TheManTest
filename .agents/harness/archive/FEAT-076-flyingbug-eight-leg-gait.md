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
