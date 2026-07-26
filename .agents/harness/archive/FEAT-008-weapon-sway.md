# [FEAT-008] 武器摇摆（Weapon Sway）

**创建日期：** 2026-06-07  
**状态：** done  

---

## 功能概述

纯 C++ 武器惯性摇摆，不改动任何动画蓝图。

胶囊体通过 `bUseControllerRotationPitch = true` / `bUseControllerRotationYaw = true` 处理主旋转。ArmsMesh 挂在胶囊根节点，`SetRelativeRotation` 设置的是相对于胶囊的偏移，两者完全独立不冲突。

---

## 实现方案

- `Look()` 中缓存每帧鼠标 delta 到 `LastMouseInput`
- `Tick()` 中用 `FMath::RInterpTo` 平滑插值 `CurrentSway`
- 将 `CurrentSway` 以 `SetRelativeRotation` 叠加到 `ArmsMesh`
- 鼠标停止时 delta = 0，Target = 零旋转，自动平滑回中

## 参数

| 变量 | 类型 | 默认值 | 说明 |
|---|---|---|---|
| `SwayIntensity` | float | 2.0 | 摇摆幅度（角度倍率） |
| `SwayInterpSpeedX` | float | 8.0 | 水平轴（Yaw/Roll）回正速度 |
| `SwayInterpSpeedY` | float | 5.0 | 垂直轴（Pitch）回正速度 |

---

## 完成标准

- [x] C++ 编译无错误无警告
- [x] PIE：鼠标移动时手臂轻微反向偏转，停止后平滑回正
- [x] SwayIntensity / SwayInterpSpeedX / SwayInterpSpeedY 蓝图可调

---

## 实现日志

### 2026-06-07 — 实现

- `FPSCharacterBase.h` 新增变量：`SwayIntensity`、`SwayInterpSpeedX`、`SwayInterpSpeedY`（X/Y 独立速度）、`CurrentSway`、`LastControlRotation`
- `FPSCharacterBase.cpp` BeginPlay() 初始化 `LastControlRotation`，Tick() 计算控制器旋转 delta，X/Y 轴独立 FInterpTo 插值，`SetRelativeRotation` 应用到 ArmsMesh
- 方案从"缓存原始鼠标 delta"升级为"控制器旋转 delta"，效果更稳定
- Yaw 分量取反修复方向（枪滞后于镜头而非超前）
