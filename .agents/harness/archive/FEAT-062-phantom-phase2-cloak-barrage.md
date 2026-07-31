# FEAT-062 — Phantom 二阶段透明穿透与范围轰炸

**状态：** done
**完成：** 2026-07-30

二阶段保留一阶段技能，新增透明穿透状态与范围轰炸 GAS 能力；视觉、碰撞/伤害过滤和弹体特效均可配置。

## 实现与验证

- Phase 2 启用真实 Translucent Cloak 材质，Phase 1 恢复原材质。
- Cloak 时 Capsule/Mesh 忽略弹体通道；BulletBase 同时查询 pass-through，攻击不结算且弹体继续。
- AreaBarrage 在目标范围上方生成 12 个可替换弹体，不消费普通弹匣。
- 阶段2保留阶段1全部技能并新增 Barrage；自动化与真实 PIE 穿透验证通过。
