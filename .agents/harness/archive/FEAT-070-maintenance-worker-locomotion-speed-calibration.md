# FEAT-070 — MaintenanceWorker 移动动画步速标定

**状态：** in_progress

**创建：** 2026-08-01

## 目标

- 通过脚掌着地阶段的世界空间漂移测量 Walk/Jog 动画自然步速。
- 同步校准 `BP_MaintenanceWorker` 的 `WalkSpeed` / `SprintSpeed` 与 `BS_RunWalk_MaintenanceWorker`。
- 保持普通 CharacterMovement 驱动，不启用 locomotion Root Motion。

## 开始状态

- 安全检查点：`a03f30d`。
- 角色速度：Walk 250 cm/s，Sprint 550 cm/s。
- BlendSpace：Direction -180~180；Speed 0~550；Idle=0、Walk=250、Jog=550；27 个样本，RateScale 全部 1.0。
- Walk 前进循环时长约 1.1667 s；Jog 前进循环时长约 0.7 s。

## 测量方法与结果

- 对每条动画以 41~81 个时间点采样 `foot_l/foot_r`，沿 `foot -> calf -> thigh -> pelvis -> root` 合成本地姿势。
- 取脚掌高度位于该脚最低点 +2.75 cm（Jog +3 cm）以内的连续样本作为着地段，以着地段水平速度中位数估计自然步速。

| 方向 | Walk cm/s | Jog cm/s | 测算 RateScale@120 | 测算 RateScale@280 |
|---|---:|---:|---:|---:|
| F | 114.9 | 273.6 | 1.044 | 1.023 |
| FL | 119.4 | 317.5 | 1.005 | 0.882 |
| FR | 119.6 | 327.1 | 1.003 | 0.856 |
| L | 97.7 | 339.5 | 1.228 | 0.825 |
| R | 99.6 | 290.5 | 1.205 | 0.964 |
| BL | 95.3 | 310.7 | 1.259 | 0.901 |
| BR | 99.4 | 294.4 | 1.207 | 0.951 |
| B | 108.4 | 218.6 | 1.107 | 1.281 |

## 已实施与验证

- 首次写入后虽然工具报告保存/重建成功，但冷回读发现 BlendSpace 仍为轴 0~550、Walk=250、Jog=550，而角色已经是 120/280；Walk 运行时因此混入约一半 Idle，用户观察到脚步像慢放。该轮验证结论撤销。
- 修正方案不再改变各方向播放倍率：所有样本 RateScale=1.0，保持动画原速；利用现有 50 cm/s 网格将 Walk 放在100、Jog放在300，Speed轴收敛为0~300。
- `BP_MaintenanceWorker` 最终为 WalkSpeed=100、SprintSpeed=300，与样本网格点精确一致。
- 修正后冷回读：轴0~300/Grid6、前进Walk=(0,100)/RateScale1.0、前进Jog=(0,300)/RateScale1.0；`BP_MaintenanceWorker` 与 `ABP_MaintenanceWorker` 编译保存成功，三个目标资产验证通过。
