# FEAT-073 — 子弹命中警觉与RepairGun减速

**状态：** done

**创建：** 2026-08-01

## 目标与确认方案

- 玩家任意子弹有效命中敌人后，敌人立即转向并锁定开枪玩家。
- RepairGun 子弹命中敌人后立即消失，并施加可配置的限时移动减速。
- `ARepairGunBullet` 暴露 `SlowPercent` 与 `SlowDuration`，默认 40% 与 2.5 秒。
- 连续命中刷新持续时间，不叠加减速强度。
- RepairGun 命中墙面、地面或危险区时仍使用原有膨胀/压制生命周期。
- 用户于 2026-08-01 确认方案，并明确要求减速参数归属 RepairGun 子弹。
- 写入前安全检查点：`6c79fe5`。

## 已实施

- `ABulletBase::ProcessHit_Implementation` 在穿透判定后统一通知敌人受击；敌方发射者被过滤，避免友军火力改写目标。
- `AEnemyBase::ReactToProjectileHit` 让所有敌人立即水平面向攻击者。
- `AHumanoidEnemy::ReactToProjectileHit` 进一步进入 `Aim`、写入 AimIK 目标、设置 AI Focus 与 Blackboard `TargetActor`。
- `AEnemyBase` 新增限时移动倍率管理；状态切换通过 `SetDesiredMaxWalkSpeed` 保存未减速基础速度，因此巡逻/追击/搜索不会冲掉减速。
- 重复减速保留最强倍率并刷新计时；到期恢复到届时最新的状态基础速度。
- `ARepairGunBullet` 新增 `SlowPercent=0.4`、`SlowDuration=2.5` 蓝图可调属性。
- RepairGun 命中敌人时施加减速并立即销毁；命中环境时仍膨胀并参与危险区压制；Phantom 穿透判定仍优先执行。

## 验证

- `TheManTestEditor Win64 Development` 完整编译成功；修复预 BeginPlay 临时 Actor 的速度初始化边界后再次完整编译成功。
- 冷启动运行时回读：攻击者位于正 Y 时敌人 Yaw 为 90°。
- 速度回读：基础 600 → 40% 减速后 360 → 重复命中仍为 360，证实不叠加。
- `BP_RepairGunBullet` CDO：`SlowPercent=0.4`、`SlowDuration=2.5`。
- 生命周期分支由同一 `ProcessHit` 路径保证：敌人分支 `Destroy` 后返回，环境分支继续原有膨胀逻辑。
## 2026-09-01 session274 — RepairGun 环境命中音效

- 新增 RepairGun 专属 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Impact_Ground`，并由 `BP_RepairGunBullet.EnvironmentImpactSound` 配置。
- `ARepairGunBullet::ProcessHit_Implementation` 仅在非敌人环境命中时于 `HitResult.ImpactPoint` 播放；敌人命中不混入地面音效，原减速/销毁和环境膨胀/压制生命周期保持不变。
- 音频为 0.860s、Stereo、96kHz，Volume/Pitch=1.0；Development Editor 构建成功，自动化冷启动加载和蓝图 CDO 绑定断言通过。

## 2026-09-01 session275 — 双层 Gameplay Cue 命中反馈

- session274 的 RepairGun 专属直接播放方案已被通用实现替代：声音迁为 `/Game/Weapons/_Shared/Audio/S_ProjectileImpact`，`ABulletBase` 对每次有效碰撞直接执行 `GameplayCue.Combat.ProjectileImpact`，因此环境与 Enemy 都有统一撞击声。
- 敌人伤害仍只使用现有 `GE_BulletDamage`；该 GE 内嵌 `GameplayCue.Combat.EnemyHit`，伤害成功应用后自动提供第二层敌人受击反馈。当前 `GC_EnemyHit` 无附加资源，作为之后配置血花、命中确认音等反馈的稳定扩展点。
- Phantom 穿透过滤早于通用 Cue；现有 Projectile/Hitscan 都汇入 `ProcessHit`。Development Editor 构建与 AmmoLifecycle 资产绑定回归均通过。

## 2026-09-01 session276 — 命中反馈所有权纠正

- session275 的跨武器共享撞击 Cue 与 GE 内嵌 EnemyHit 方案已撤销。`ABulletBase` 改为可选 `ImpactCueTag` 插槽，RepairGun 子弹默认选择其专属 `GameplayCue.Weapon.RepairGun.Impact`。
- RepairGun 音效/Cue 分别归档于 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Impact` 和 `GAS/GameplayCues/GC_RepairGun_Impact`；击中环境或敌人都执行，Phantom 穿透仍不执行。
- 共享 `GE_BulletDamage` 恢复为纯伤害。敌人受击反馈由 Health 实际扣减回调目标自己的 `HitReactionCueTag`，默认 `GameplayCue.Character.Enemy.Hit`；不同敌人蓝图可独立覆盖而不复制 GE。
