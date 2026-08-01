# FEAT-074 — VFXPack移动HeadBob、武器摆动与RepairGun射击震屏

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
