# FEAT-034 伤害弹（子弹伤害 GE）

**状态：** done
**创建：** 2026-06-17
**关闭：** 2026-06-17-session38
**最后更新：** 2026-06-17-session38

> 用户确认（session38）：编译通过，PIE 射击敌人正常按 Damage 扣血，屏幕橙色输出剩余血量。
> ⚠️ 遗留：敌人扣血调试输出（`TheManAttributeSetBase::PostGameplayEffectExecute`）仍在，正式上线前可删。

---

## 目标

让子弹命中带 ASC 的目标时按可配置数值扣血。复用现有子弹基类，伤害数值配置在子弹上，伤害规则放在一个可复用的 GameplayEffect 里。

---

## 架构事实（已确认）

- `ABulletBase`（`Equipment/Firearms/Bullets/`）已自带：
  - `UProjectileMovementComponent`（默认 5000 速度直飞）——"射出去"已现成。
  - `OnBulletHit → ProcessHit_Implementation`：命中时对目标 ASC 施加 `HitEffectClass`（`TSubclassOf<UGameplayEffect>`）。`HitEffectClass` 为空 → 直接 return，不扣血。
- `UTheManAttributeSetBase::PostGameplayEffectExecute` 已处理：Health 钳制 [0, MaxHealth] + Health≤0 调用 `OnDeath()`（FPSCharacterBase / EnemyBase）。
- → **伤害逻辑不在子弹写，在 GE 写；扣血/死亡链路全自动。无需新写子弹 C++ 子类。**

## 设计决策

- **伤害 GE 配置在子弹的 `HitEffectClass`**：没配 = 不扣血，配了 = 按 GE 来（代码已是此行为）。
- **伤害数值用 SetByCaller**：子弹新增 `Damage` 浮点字段，命中时 `SetSetByCallerMagnitude(TAG_Data_Damage, -Damage)` 传入 GE。一个 GE_BulletDamage 复用所有子弹，改伤害只改子弹上的数字。

---

## C++ 改动（session38 完成，待编译验证）

1. `GAS/TheManGameplayTags.h/.cpp`：新增 `TAG_Data_Damage`（`"Data.Damage"`）。
2. `Bullets/BulletBase.h`：新增 `UPROPERTY(EditDefaultsOnly) float Damage = 0.f;`（Bullet|GAS 分类，ClampMin 0）。
3. `Bullets/BulletBase.cpp`：include TheManGameplayTags；`ProcessHit_Implementation` 在 `ApplyGameplayEffectSpecToTarget` 前 `Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage, -Damage)`。传负值 → GE 的 Health Add 修改器读取后扣血。GE 不使用此 Tag 时该调用无副作用（对 RepairGun 泡泡等无影响）。
4. `TheManAttributeSetBase.cpp`：`PostGameplayEffectExecute` Health 分支加调试输出——Avatar 为 `AEnemyBase` 时屏幕打印 `[名字] 剩余血量: X / Max`（橙色，2s）。include `Engine/Engine.h`。验证扣血用，正式上线前可删。

> ⚠️ 含头文件新增 UPROPERTY + 新 Gameplay Tag，**需完整重新编译**（VS / 编辑器 Build），不能只 Live Coding。

---

## 编辑器完成情况（蓝图）—— session38 全部完成

- [x] 创建 `GE_BulletDamage`：Duration=Instant；Modifier：Attribute=Health，Op=Add，Magnitude=Set By Caller，Data Tag=`Data.Damage`。
- [x] 创建伤害弹 BP（继承 `ABulletBase`）：填 `BulletMesh`、`HitEffectClass`=GE_BulletDamage、`Damage`=数值、ProjectileMovement 速度。
- [x] 武器 `BulletClass` 指向该 BP。
- [x] PIE 验证：射击敌人按 Damage 扣血，屏幕橙色输出剩余血量（用户确认）。

> 注：目标必须实现 `IAbilitySystemInterface` 且有 ASC + Health 属性，否则 ProcessHit 不施加效果（打墙/地无效，正常）。

---

## Bug 日志

### BUG-034-001：子弹击中 enemy 会击退 / 让其飘逸（已修 session39）
**现象（用户报告 2026-06-17 session38）：** 无论哪种子弹，命中 enemy 都会把它击退 / 飘逸（drift/float）。
**根因：** `ABulletBase::CollisionSphere` = `QueryAndPhysics` + `SetNotifyRigidBodyCollision(true)` + Block `ECC_Pawn`，5000 速度抛射体参与物理求解，与 enemy 胶囊体碰撞时施加物理冲量，把角色推飞。
**修法（session39 已实施，`BulletBase.cpp` 构造函数）：**
- CollisionSphere `SetCollisionEnabled` 由 `QueryAndPhysics` 改为 `QueryOnly` —— 仅查询碰撞，不参与物理求解，消除冲量。
- 移除 `SetNotifyRigidBodyCollision(true)` —— 该标志仅对物理模拟生成的碰撞事件有效，QueryOnly 下无意义。
- 命中检测不受影响：ProjectileMovement 的 sweep 移动遇到 Block 响应仍触发 `OnComponentHit`（不依赖物理模拟）；危险区 Overlap 照常。
- RepairGun 泡泡（ARepairGunBullet）继承同一碰撞设置，sweep 命中逻辑不变，行为不受破坏。
**状态：** 已修复，用户 PIE 确认（session39）：射击 enemy 正常扣血且不再被击退/飘飞。

### BUG-034-002：伤害弹命中后不销毁、继续穿飞（已修 session39）
**现象（用户报告 session39）：** 伤害弹打过去命中后不销毁，继续飞。
**根因：** 基类 `ABulletBase::ProcessHit_Implementation` 只施加伤害 GE，从不停下/销毁子弹。RepairGun 泡泡靠子类重写自管膨胀+销毁；伤害弹是纯 BP 继承 ABulletBase、无子类，故命中后一直飞。
**修法（session39，`BulletBase` + `RepairGunBullet`）：**
- `BulletBase.h` 新增 `UPROPERTY bool bDestroyOnHit = true`（命中后是否销毁）。
- `BulletBase.cpp` `ProcessHit_Implementation`：原先一连串早退 return 改为嵌套 if（打墙/地等无 ASC 目标只是跳过伤害施加，不再提前 return）；末尾 `if (bDestroyOnHit) { ProjectileMovement->StopMovementImmediately(); Destroy(); }`。
- `RepairGunBullet.cpp` 构造函数置 `bDestroyOnHit = false`，保留泡泡命中后膨胀+定时销毁逻辑。
**状态：** 已修复，用户 PIE 确认（session39）：伤害弹命中即停下销毁，泡泡行为不变。

### BUG-034-003：子弹生成在枪口撞到自己（已修 session39，待 PIE 验证）
**现象：** 子弹生成在枪口（紧贴角色胶囊体/手臂），一出膛就撞到发射者自己，在枪口炸开。
**根因：** 子弹 `CollisionSphere` Block `ECC_Pawn`，`SetOwner` 不会让 sweep 自动忽略 owner。
**修法（`BulletBase.cpp` `InitBullet`）：** `CollisionSphere->IgnoreActorWhenMoving(HitInstigator, true)` 忽略发射者，并遍历 `GetAttachedActors` 忽略其挂载的装备。
**状态：** 已修，待 PIE 验证。

### BUG-034-004：切角色后开火技能泄漏累积 → 一次开火多颗子弹枪口炸（已修 session40，用户确认）
**现象（用户确认 session40）：** 只切一次到维修工开火正常；切两次及以上再开火，同枪口同帧多颗子弹互相 Block 炸开。渐进式累积 = 典型技能规格泄漏。
**真实根因（session40 定位，比 session39 记录更深）：**
- ASC 在 PlayerState 持久存在。`SwitchCharacter` 顺序是先 `Possess(NewCharacter)` 再 `OldPawn->Destroy()`；`Possess` 会先 UnPossess 旧角色，**把旧角色的 PlayerState 置空**。
- session39 的修法（`EndPlay` 里 `GetCurrentEquipment()->Unequip()`）方向对但**拿不到 ASC**：`Unequip` 走 `GetOwner()->GetAbilitySystemComponent()` → `GetPlayerState()` 此时已为 null → `RevokeAbilities(null)` 直接 early return → **技能根本没回收**。每切一次漏一个 `GA_Shoot` 规格。
**最终修法（session40，`Firearm.h/.cpp`）：**
- `Firearm` 新增 `TWeakObjectPtr<UAbilitySystemComponent> GrantedASC`，`GrantAbilities` 授予时缓存（指向持久 PlayerState 上的 ASC，指针始终有效）。
- `RevokeAbilities`：传入 ASC 为 null 时回退到 `GrantedASC.Get()`，确保技能一定被回收；回收后置空缓存。
- `EquipmentManagerComponent::EndPlay` 销毁装备前 `Unequip()` 的调用保留（现在能真正回收了）。
**状态：** ✅ 已修复，用户 PIE 确认（session40）：连续切换维修工多次后开火只出 1 颗子弹，不再炸膛。

### BUG-SWITCH-002：切换角色瞬间武器起始位置错乱（待优化，标记 needs_improvement）
**现象（用户报告 session40）：** 切到维修工那一下，武器先出现在错误位置（左下角）再到正确位置；过程很短。
**已尝试（session40，均未根治）：**
1. `ArmsMesh->VisibilityBasedAnimTickOption = AlwaysTickPoseAndRefreshBones`（默认隐藏时不评估姿势）。
2. BeginPlay 强制 `TickAnimation`+`RefreshBoneTransforms`（已回退）。
3. **拔枪蒙太奇与 Equip 解耦**（保留，架构更干净）：`EquipmentBase::Equip()` 不再内部播蒙太奇；新增 `PlayEquipMontage()`，滚轮切枪立即播、切角色初次装备推迟到下一帧 `RevealArmsAndWeapon`（姿势就绪）再播。
**结论：** 仍未根治，推测"下一帧"仍不足以让动画实例/姿势完全就绪，或拔枪蒙太奇本身起始帧即如此。用户决定**暂缓**——此为测试用武器，精度要求低。
**状态：** needs_improvement（待优化）。后续方向：等蒙太奇/姿势真正 ready 的稳健信号再触发显示，而非固定下一帧；或检查拔枪蒙太奇起始帧。
**保留改动：** 蒙太奇解耦（PlayEquipMontage）与 `AlwaysTickPoseAndRefreshBones` 已保留，无副作用、架构更清晰。

> 注：session39 记录的 BUG-SWITCH-001（"黑影"藏一帧方案）即本问题前身，已并入 BUG-SWITCH-002。
