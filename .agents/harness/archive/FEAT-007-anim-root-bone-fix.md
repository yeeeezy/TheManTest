# [FEAT-007] 动画根骨骼位置修复

**创建日期：** 2026-06-07
**状态：** planned（待优化）
**Archive 文件：** `archive/FEAT-007-anim-root-bone-fix.md`

---

## 问题描述

`ArmsMesh` 使用的动画资产根骨骼（root bone）不在原点，导致动画播放时整个骨骼链偏移，`HeadCamera`（挂在 `head` 骨骼上）位置错误，相机视角偏移。

---

## 临时方案（当前生效）

在角色蓝图中手动调整 `HeadCamera` 的 Relative Transform 做偏移补偿，使相机视角位置看上去正确。**这是 workaround，不是根本修复。**

---

## 根本修复方案

需要美术介入，二选一：

**方案 A（骨骼编辑器）：**
1. 打开骨骼资产（SK_Mannequin 或对应骨骼）
2. 找到 `root` 骨骼 → `Translation Retargeting` 改为 `Skeleton`
3. 动画里的根骨骼平移被忽略，以参考姿势为准

**方案 B（修正动画资产）：**
1. 在 DCC 工具（Maya/Blender）中将动画根骨骼归零
2. 重新导入动画资产

---

## 相关上下文

- 发现于 FEAT-006（FPS 动画架构）搭建过程中
- `ABP_FPSArms` 的 `Root Motion Mode` 应同步设为 `No Root Motion Extraction`
- `head` 骨骼位置依赖根骨骼，根骨骼偏移会连锁影响相机位置

---

## 完成标准

- [ ] HeadCamera 无需手动偏移，位置自动对齐
- [ ] PIE 测试：相机视角位置正确
- [ ] 蓝图中 HeadCamera Transform 恢复默认值（无补偿偏移）

---

**完成标准全部满足日期：** —
**功能关闭日期：** —
