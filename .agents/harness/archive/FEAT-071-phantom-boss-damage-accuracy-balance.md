# FEAT-071 — Phantom Boss伤害与命中容错平衡

**状态：** in_progress

**创建：** 2026-08-01

## 目标

- 降低 Phantom 普通射击和 Area Barrage 共用子弹的单发伤害。
- 扩大初始及持续射击散布，降低 Boss 中距离连续命中的概率。
- 降低回合内伤害成长，避免后期重新回到两三发击杀。

## 开始状态与批准方案

- `BP_PhantomBullet1/2.Damage`：10。
- Phantom 射击 Ability：BaseSpread=1.5°、PerShot=0.45°、Max=5°、Recovery=3°/s、MovingPenalty=1°。
- Burst：3发/0.12秒；Suppressive：10发/0.09秒。
- Phantom 每波伤害成长20%，最大倍率2.0。
- 用户批准：子弹伤害降至6；散布改为3/0.8/9°、恢复2°/s、移动惩罚2°；成长降至10%、上限1.5。
- 安全检查点：`5ee22d9`。

## 已实施

- `BP_PhantomBullet1`、`BP_PhantomBullet2`：Damage 10 -> 6；Area Barrage 使用 Bullet2，因此其单发伤害同步降低。
- `BGA_PhantomShoot1/2`、`BGA_PhantomBurst`、`BGA_PhantomSuppressiveFire`：BaseSpread 1.5 -> 3°，PerShot 0.45 -> 0.8°，Max 5 -> 9°，Recovery 3 -> 2°/s，MovingPenalty 1 -> 2°。
- `BP_Phantom`：StrengthDamageBonusPerWave 0.2 -> 0.1；MaxDamageMultiplier 2.0 -> 1.5。
- 未修改 Burst 3发/0.12秒、Suppressive 10发/0.09秒、Area Barrage 12发/500半径，先单独观察伤害和命中率变化。

## 验证

- 7个目标蓝图逐一编译、保存成功；Phantom Ability 目录及三种关键蓝图资产验证通过。
- 完全退出并重启编辑器后冷回读：两种 Damage=6；四套散布均为3/0.8/9、Recovery=2、Moving=2；成长0.1、上限1.5，确认落盘。
- Headless NullRHI 自动化 `TheManTest.Enemy.Phantom.ReusableCombatModules`：Success，进程退出码0。日志：`Saved/Logs/FEAT071Automation.log`。
- 伤害预期：阶段1 Burst 全中18、Suppressive全中60；最大成长倍率下单发9、Burst全中27。实际命中率因散布扩大应显著低于原值，待用户前台PIE主观确认。
