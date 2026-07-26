# [FEAT-014] 环境危险区系统（火焰）

**创建日期：** 2026-06-07
**状态：** done
**Archive 文件：** `archive/FEAT-014-environment-hazard.md`

---

## 功能概述

基于接口的可扩展环境危险区系统。
- 任何实现 `IHazardSuppressor` 的 Actor（当前：`ARepairGunBullet`）进入危险区时，关闭 Niagara 特效和伤害 tick。
- 压制者 Actor 销毁后（RepairGun 泡泡 LifetimeAfterExpansion 到期），危险区自动恢复。
- 支持多个同类 Actor 同时压制（计数清零才恢复）。

---

## 继承与接口关系

```
IEnvironmentHazard（伤害接口，留空）
  └── AEnvironmentHazardBase（Abstract，核心逻辑）
        └── AFlameHazard（具体火焰，蓝图配置 Niagara 资产）
              └── BP_FlameHazard（蓝图实例）
        └── APoisonGasHazard（待扩展）
        └── ...

IHazardSuppressor（标记接口）
  └── ARepairGunBullet（膨胀泡泡，压制危险区）
  └── 未来其他"修复/抑制"工具
```

---

## 碰撞设计（压制检测关键）

| 组件 | ObjectType | 对 WorldDynamic | 对 GameTraceChannel1 | GenerateOverlapEvents |
|---|---|---|---|---|
| `BulletBase::CollisionSphere` | GameTraceChannel1 | ECR_Overlap（新增）| ECR_Block | true（新增）|
| `EnvironmentHazardBase::HazardZone` | WorldDynamic | — | ECR_Overlap | true |

两端均 ECR_Overlap + GenerateOverlapEvents = true → `HazardZone::OnComponentBeginOverlap` 触发。
检测 `OtherActor->Implements<UHazardSuppressor>()` → `Suppress(OtherActor)`。
绑定 `OtherActor->OnDestroyed` → `Resume(OtherActor)`。

---

## 新增文件

| 文件 | 作用 |
|---|---|
| `Public/Environment/Hazards/EnvironmentHazardInterface.h` | IEnvironmentHazard 接口（InflictDamage 留空） |
| `Public/Environment/Hazards/HazardSuppressorInterface.h` | IHazardSuppressor 标记接口 |
| `Public/Environment/Hazards/EnvironmentHazardBase.h/.cpp` | 抽象基类：HazardZone + HazardEffect + 压制逻辑 |
| `Public/Environment/Flame/FlameHazard.h/.cpp` | 具体火焰类（蓝图配置 Niagara 资产） |

## 修改文件

| 文件 | 变更 |
|---|---|
| `TheManTest.Build.cs` | 添加 `Niagara` 模块依赖 |
| `BulletBase.cpp` | CollisionSphere 添加 ECR_Overlap for WorldDynamic + SetGenerateOverlapEvents(true) |
| `RepairGunBullet.h` | 继承 `IHazardSuppressor` |

---

## 蓝图配置步骤

1. 编译 C++
2. Content/Environment/Flame/ 下创建 Blueprint，父类 `AFlameHazard`，命名 `BP_FlameHazard`
3. 打开 BP_FlameHazard：
   - 选中 `HazardEffect` 组件 → Details → Niagara System Asset → 选择火焰 Niagara 资产
   - 选中 `HazardZone` 组件 → Details → Sphere Radius → 调整覆盖范围
4. 将 BP_FlameHazard 拖入关卡
5. PIE 测试：RepairGun 泡泡接触火焰区 → 特效停止；泡泡到期销毁 → 特效恢复

---

## 扩展新危险区类型（未来）

1. 新建 C++ 类继承 `AEnvironmentHazardBase`
2. 重写 `InflictDamage_Implementation` 实现对应伤害 GE
3. 在编辑器创建蓝图子类，配置对应 Niagara 资产
4. 无需修改 RepairGun 或 BulletBase 任何代码

---

## 实现日志

- 2026-06-07：C++ 层完成（接口、基类、FlameHazard、BulletBase 修改、RepairGunBullet 接口添加）
- 2026-06-07：编译报错 `FOverlapResult` 未定义 → DamageTick 改用 `UKismetSystemLibrary::SphereOverlapActors`，消除对 `WorldCollision.h` 的依赖
- 2026-06-07：编译通过，PIE 验证通过

---

**完成标准全部满足日期：** 2026-06-07
**功能关闭日期：** 2026-06-07
