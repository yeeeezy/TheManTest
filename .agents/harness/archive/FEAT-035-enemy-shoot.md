# FEAT-035 人形怪射击技能（复用子弹管线）

**状态：** in_progress
**创建：** 2026-06-18-session40
**最后更新：** 2026-06-18-session40

---

## 目标

给人形怪加开火能力，且尽量复用玩家现有的射击/伤害系统。

## 复用性分析（关键决策）

**玩家的 `UGA_Shoot` 不能直接复用** —— 它和玩家强耦合：
- `Cast<AFPSCharacterBase>()` 硬转玩家类（敌人是 `AEnemyBase` 体系）
- `GetHeadCamera()` 用相机算瞄准（敌人无相机，靠 AI 写 `AimTargetWorld`）
- `GetEquipmentManager()->GetCurrentEquipment()`（敌人无装备管理器，武器是挂 hand_r 的 `UStaticMeshComponent`）
- `GetArmsMesh()` / `AddRecoil()`（玩家专属）

**真正可完整复用的是子弹+伤害管线**：`ABulletBase`（+伤害弹 BP）+ `GE_BulletDamage` + `ProcessHit/InitBullet`。子弹不关心谁开火，只要传 instigator + source ASC，命中带 Health 的目标即扣血；且已忽略发射者自身（BUG-034-003）。敌人开的子弹打玩家天然扣血。

→ **方案：新建薄的敌人专属 `UGA_EnemyShoot`，复用子弹管线。** 比把 `GA_Shoot` 改造成通吃两边省事、零回归风险（用户在 session40 选定此路线）。

## 重要约束：触发链未就绪

`HumanoidAIController` 的感知/瞄准目前是 TEMP 禁用状态（`OnTargetPerceptionUpdated` 空、`bIsAiming` 恒 false、`AimTargetWorld` 无人写），FEAT-031/032 搁置中。即敌人永远 Patrol，不会自己进战斗触发开火。
→ 本功能附带 `bTestAutoFire` 测试驱动，使射击技能可独立 PIE 验证；FEAT-032 复活后改由感知/行为树调用 `FireAtActor()`。

---

## C++ 改动（session40 完成，待编译验证）

1. **`GAS/Abilities/GA_EnemyShoot.h/.cpp`（新建，**作为基类**）**：`UGA_EnemyShoot : UGameplayAbility`。
   - **不注册 GameplayEvent 触发器**（共享 tag 会同时激活所有同类技能 → 多颗子弹）。由 `FireAbility` 按类激活。
   - **配置作为技能 UPROPERTY（技能与子弹绑定）**：`BulletClass` / `MuzzleSocketName`(默认 "Muzzle") / `FireMontage` / `FireSound` / 音量音调倍率，均 EditDefaultsOnly。
   - `ActivateAbility`：`Cast<AHumanoidEnemy>` avatar；从 `GetWeaponMesh()` 的 `MuzzleSocketName` socket 取枪口（退回组件原点/敌人位置）；方向 = `(AimTargetWorld - Muzzle).GetSafeNormal()`（无效退回正前方）；调 `SpawnProjectiles()`；播 `FireMontage`（敌人 GetMesh AnimInstance）、`FireSound`；EndAbility。
   - **`virtual void SpawnProjectiles(Enemy, Muzzle, Dir)`（扩展点）**：默认单发 `SpawnActor<ABulletBase>(BulletClass,...)` + `InitBullet(Enemy, Enemy ASC)`。散射/连发/hitscan 由 C++ 子类重写本函数，复用基类其余流程。
2. **技能授予 + 选择系统（session40 末轮重构到 AEnemyBase，详见 FEAT-032 archive「技能集系统」）**：
   - `AEnemyBase`：`DefaultAbilities`（常驻）+ `PhaseSkillSets`（阶段×近/中/远 `FEnemyPhaseSkillSet`），`BeginPlay` 经 `GrantAbilities` 全部授予；`UseRandomSkill(Target, Range)` 当前阶段对应距离档随机放招 → `AimAtTarget`(virtual) → `TryActivateAbilityByClass`。
   - `AHumanoidEnemy`：重写 `AimAtTarget` 写 `AimTargetWorld=Target 位置`（供 GA_EnemyShoot 取子弹方向）。
   - `BGA_EnemyShoot` 放进敌人 `PhaseSkillSets` 的某个距离档；由 `BTTask_UseCombatSkill` 触发。
   - ~~`FireAbility` / 测试驱动 `bTestAutoFire` / `BTTask_EnemyShoot`~~ —— **session40 已删除/取代**。

## 可扩展性决策演变（session40，用户两轮迭代）

**第 1 轮**——最初配置写在技能上，用户问扩展性。一度改为「配置放敌人、技能读配置」（一敌一弹好用）。
**第 2 轮**——用户提新需求：**同一敌人两个技能发两种子弹，且技能与子弹绑定 + 逻辑复用**。配置放敌人无法满足（一敌只能一弹）。
→ **最终方案：配置放回技能（技能=子弹的绑定单元）**，同时满足两种场景：
- 同一敌人多种子弹：授予多个 `UGA_EnemyShoot` 蓝图子类，各填一种子弹；`FireAbility(Target, 哪个类)` 分别触发。
- 不同敌人不同子弹：各敌人的 `DefaultAbilities` 引用不同技能子类即可。
- 仅数据不同 → 蓝图子类即可（一份 C++ 逻辑复用）；逻辑也不同（散射等）→ C++ 子类重写 `SpawnProjectiles`。
- 触发改 `TryActivateAbilityByClass`（弃用 `Event.Enemy.Shoot` 事件 tag，已从 TheManGameplayTags 删除），避免共享事件同时激活多个技能。
- 将来敌人若要「换武器」（运行时切换整组武器+子弹），再把技能集合抽象为武器对象/数据资产；当前够用。

> ⚠️ 含新增 UPROPERTY/方法声明/新 Tag，**需完整重新编译**，不能只 Live Coding。

---

## 待办（编辑器，蓝图）

- [ ] 敌人武器 StaticMesh 资产加 `Muzzle` socket（朝枪口前方）。
- [ ] 创建 `BGA_EnemyShoot`（父类 `UGA_EnemyShoot`）：填 `BulletClass`=伤害弹 BP（可复用玩家那颗，或新建敌人伤害值的）、`MuzzleSocketName` 与上面 socket 一致。
- [ ] `BP_Phantom` `DefaultAbilities` 数组填入 `BGA_EnemyShoot`；`TestFireAbility` 选同一个；勾选 `bTestAutoFire`。
- [ ] PIE：站到敌人视线+射程内，敌人定时开火，子弹命中玩家扣血（玩家受伤血量变化）。
- [ ] （可选，验证多子弹）再建 `BGA_EnemyShoot_2` 填另一种子弹，`DefaultAbilities` 同时加入两个，`FireAbility` 传不同类可分别打出两种子弹。

## 后续（非本功能范围）

- FEAT-032 AI 复活后：感知发现玩家 → SetAIState(Aim) + 每帧写 AimTargetWorld → 战斗逻辑按节奏调 `FireAtActor`，移除/关闭 `bTestAutoFire`。
- FEAT-027 上半身蒙太奇插槽就绪后：`FireMontage` 走上半身 slot，不盖住移动。
- 敌人子弹目前也会命中其它敌人（只忽略发射者）；如需阵营过滤后续再加。

## Bug 日志

（暂无）
