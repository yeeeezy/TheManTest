# FEAT-030 敌人左手武器 IK（Control Rig）

**状态：** done  
**创建：** 2026-06-13  
**最后更新：** 2026-06-13-session34  
**关闭：** 2026-06-13-session34

---

## 目标

用 Control Rig Two-Bone IK 把人形怪左手固定在枪的前握把上，消除跑动时单靠动画 slot 导致的左手脱离抖动。

---

## 架构决策

**枪挂 hand_r，只做左手 IK**

- `WeaponMesh`（UStaticMeshComponent）在 `AHumanoidEnemy` 上，`BeginPlay` 挂到 `WeaponAttachSocket`（默认 `hand_r`）
- 右手跟动画走，天然贴合枪柄
- Control Rig 只解算左臂 Two-Bone IK：`upperarm_l → lowerarm_l → hand_l`，Effector = 武器 `grip_l` socket 的 Component Space Transform
- IK 骨骼名通过 `EditDefaultsOnly` 变量配置，不同骨骼的子 ABP 在 Class Defaults 里覆盖

**为何不做脊柱 IK：** 当前只需左手跟枪，脊柱留给后续瞄准偏移（FEAT-XXX），避免两套 IK 互相干扰。

---

## C++ 实现（已完成，待编译）

### 修改文件

| 文件 | 变更 |
|---|---|
| `HumanoidEnemy.h` | 前向声明 `UStaticMeshComponent`；新增 `WeaponMesh`（VisibleAnywhere）、`WeaponAttachSocket`（EditDefaultsOnly = "hand_r"）；`GetWeaponMesh()` getter |
| `HumanoidEnemy.cpp` | include `StaticMeshComponent.h`；构造函数 `CreateDefaultSubobject + SetupAttachment(GetMesh())`；BeginPlay 重新 `AttachToComponent` 到 `WeaponAttachSocket` |
| `HumanoidEnemyAnimInstance.h` | 新增 `IK_UpperArm / IK_LowerArm / IK_Hand`（EditDefaultsOnly FName）；`WeaponGripLeftSocket`（EditDefaultsOnly = "grip_l"）；`LeftHandIKTarget`（BlueprintReadOnly FTransform）；`bHasWeapon`（BlueprintReadOnly bool） |
| `HumanoidEnemyAnimInstance.cpp` | include `StaticMeshComponent.h`；NativeUpdateAnimation 末尾加武器 IK Target 计算块（GripWorld → Component Space） |

### 核心逻辑

```
NativeUpdateAnimation 每帧：
  Weapon = HumanoidOwner->GetWeaponMesh()
  bHasWeapon = Weapon->GetStaticMesh() != nullptr
  if bHasWeapon:
    GripWorld = Weapon->GetSocketTransform("grip_l")
    LeftHandIKTarget = GripWorld.GetRelativeTransform(GetMesh()->ComponentTransform)
    → 传给 Control Rig（Component Space Effector）
```

---

## 待完成（编辑器操作）

- [ ] 编译 C++（Development Editor / Win64）
- [ ] 骨骼编辑器：在 `hand_r` 骨骼上添加 weapon socket（可选，直接用骨骼也行）
- [ ] 武器 StaticMesh：添加 `grip_l` socket（左手握把位置，Z 轴朝上）
- [ ] 创建 `CR_HumanoidEnemy`（Control Rig 资产）：
  - 暴露输入变量：`LeftHandTarget`（FTransform）、`IKAlpha`（float）
  - 骨骼名输入：`UpperArmBone`（FName）、`LowerArmBone`（FName）、`HandBone`（FName）
  - Two-Bone IK 节点：Root = UpperArmBone，Mid = LowerArmBone，End = HandBone，Effector = LeftHandTarget
- [ ] `ABP_HumanoidEnemy` AnimGraph：状态机输出后插入 Control Rig 节点
  - CR Class = `CR_HumanoidEnemy`
  - 接线：`LeftHandIKTarget` → `LeftHandTarget`；`bHasWeapon`（bool→float）→ `IKAlpha`
  - 接线：`IK_UpperArm / IK_LowerArm / IK_Hand` → CR 对应骨骼名变量
- [ ] BP_Phantom：`WeaponMesh` 组件赋值武器网格资产（SCI_FI 包或其他）
- [ ] PIE 全流程验证：跑动时左手贴合前握把，无抖动

---

## Bug 日志

### BUG-030-001 左手肘部屈肘消失

**现象：** Two-Bone IK 激活后，左手肘部失去原始动画的屈肘，手臂变直或弯向错误方向。

**根因：** Joint Target Space 设为 Component Space 时数值难以调准；UE4 Mannequin 左臂肘部方向需要极向量匹配。

**修复方案（Session34 验证通过）：**
- `Joint Target Space` = **Bone Space**
- `Joint Target Bone Name` = `lowerarm_l`
- `Joint Target Location` = `(0, 0, 0)`
- 效果：肘部始终沿动画自然方向弯曲，不依赖手动调数值。

**状态：已解决 ✓**

---

## 已知风险

- `grip_l` socket 不存在时，`GetSocketTransform` 返回武器 mesh 的 component transform，IK 会把左手拉向武器中心点。务必先在编辑器建好 socket。
- UE4 Mannequin 骨骼（enemy 所用），`hand_l` bone 的 local axes 可能与 UE5 不同，需在 CR 里确认 Two-Bone IK 的 Bend Direction（肘部弯曲方向）。

---

## BUG-030-002 Patrol / Aim 状态武器 socket 位置不一致导致手肘过伸

**现象：** 巡逻状态下左手 IK 正常；BBBAimIK 激活（脊柱旋转朝向玩家）后手肘必须过度伸长才能触及握把，或者用一个 socket 适配了瞄准就破坏了巡逻。

**根因：** 武器只挂在单一 socket（`hand_r`）。Patrol 状态脊柱中立，`hand_r` 在 A 位置；Aim 状态 BBBAimIK 旋转脊柱后 `hand_r` 移到 B 位置。两种状态武器的世界坐标本身就不同，单个固定 socket offset 无法同时满足两种持枪姿势。

**决定方案：两 socket + 状态切换挂载**

在骨骼 `hand_r` 骨骼上添加两个子 socket：

| Socket | 用途 |
|---|---|
| `hand_r_carry` | 巡逻自然持枪位置（枪托贴身，枪口微低） |
| `hand_r_aim` | 瞄准位置（枪管朝前，与脊柱旋转后方向对齐） |

`SetAIState` 切换时调用 `WeaponMesh->AttachToComponent(GetMesh(), SnapToTarget, TargetSocket)`。  
Two-Bone IK 的 `grip_l` 目标跟着武器整体移动，天然适配，无需额外处理。

**需新增 UPROPERTY（HumanoidEnemy.h）：**
```cpp
UPROPERTY(EditDefaultsOnly, Category = "Weapon")
FName CarrySocketName = "hand_r_carry";

UPROPERTY(EditDefaultsOnly, Category = "Weapon")
FName AimSocketName = "hand_r_aim";
```

**状态：待实现（下一会话）**
