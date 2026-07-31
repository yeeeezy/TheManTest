# FEAT-061 — Phantom 翻滚找掩体与战斗决策

**状态：** done
**完成：** 2026-07-30

Phantom 专属翻滚找掩体和战斗组合；通用行为树不直接依赖 Phantom 类，缺失动作/特效使用显式可配置空槽。

## 实现与验证

- 通用 TakeCover 能力查询最佳掩体并 MoveTo StandPoint；RollMontage 为显式可配置空槽。
- Rifle_01 未找到 Roll 动画，按约定保留空槽，不伪造不兼容动作。
- TakeCover 与 Burst/SuppressiveFire/Reload 通过阶段技能数据注入；公共 BT 不引用 Phantom 类型。
- Blueprint 编译、引用审计、掩体自动化和真实 PIE spawn 通过。
