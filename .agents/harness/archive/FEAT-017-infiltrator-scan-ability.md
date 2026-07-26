# FEAT-017 — Infiltrator 扫描技能 GAS 框架

**状态：** done  
**创建日期：** 2026-06-08  
**关闭日期：** 2026-06-08

---

## 功能描述

为 AFPSInfiltrator 建立扫描技能的完整 GAS 链路。E 键触发，独立于武器系统。
当前阶段：E 键触发 → Spawn 全息 UI Actor（BP_uiFrame）附着相机正前方，再按 E 播 Hide 动画后销毁。

---

## 完整链路

```
E 键按下
  → FPSCharacterBase::SetupPlayerInputComponent
      绑定 InteractAction → ActivateInteract()
  → ActivateInteract()
      ASC->HandleGameplayEvent(TAG_Input_Character_Interact, &Payload)
  → UGA_InfiltratorScan::ActivateAbility()
      （AbilityTriggers 监听 Input.Character.Interact）
      Toggle 逻辑：
        首次按 → Spawn HologramActorClass → AttachToComponent(HeadCamera) →
                  SetRelativeLocation(SpawnDistance, 0, 0) → SetRelativeRotation(0, 90, 0) →
                  调用蓝图 Show 事件（BP_uiFrame 内含闪烁+展开动画）
        再次按 → 调用蓝图 Hide 事件（反向收缩+淡出） → SetLifeSpan(2.5s) → 自动销毁
```

---

## 技能授予方式

`AFPSInfiltrator::PossessedBy` 遍历 `DefaultAbilityClasses`（`TArray<TSubclassOf<UGameplayAbility>>`），调用 `ASC->GiveAbility()`。

在蓝图 `BP_FPSInfiltrator` 的 Details → GAS|Abilities → Default Ability Classes 中添加 `BGA_InfiltratorScan`。

---

## 新增/修改文件

| 文件 | 作用 |
|---|---|
| `Public/GAS/Abilities/GA_InfiltratorScan.h` | 技能声明；HologramActorClass / SpawnDistance / SpawnedHologram |
| `Private/GAS/Abilities/GA_InfiltratorScan.cpp` | Toggle：Spawn+Attach+Show / Hide+SetLifeSpan |
| `GAS/TheManGameplayTags.h/.cpp` | TAG_Input_Character_Interact = "Input.Character.Interact" |
| `Infiltrator/FPSInfiltrator.h/.cpp` | DefaultAbilityClasses；PossessedBy 授予技能 |
| `Core/TheManPlayerController.cpp` | HandleTestSwitchCharacter 切换到 Infiltrator |
| `FPSArmsAnimInstance.cpp` | CalculateDirection 改为内联实现（移除废弃 API） |

---

## 蓝图配置步骤（编译通过后）

1. 创建 `IA_Interact`（E 键，Digital Bool），加入 `IMC_Default`
2. `BP_TheManPlayerController` → Input|Ability → Interact Action → 填 `IA_Interact`
3. 在 Content/GAS/Abilities/ 创建 `BGA_InfiltratorScan`（父类 `UGA_InfiltratorScan`，无需节点）
4. `BGA_InfiltratorScan` → Scan|Hologram → Hologram Actor Class → 填 `BP_uiFrame`（或其子类）
5. `BP_FPSInfiltrator` → GAS|Abilities → Default Ability Classes → 填 `BGA_InfiltratorScan`
6. PIE 测试：以 Infiltrator 按 E → 全息 UI 出现在相机正前方并播放展开动画；再按 E → 收缩淡出销毁

---

## 实现日志

- 2026-06-08（Session8）：C++ 框架完整实现（Tags + GA_InfiltratorScan + FPSInfiltrator 全链路）
- 2026-06-08（Session9）：
  - E 键触发确认（PIE 调试文字验证通过）
  - 切换角色修复（HandleTestSwitchCharacter → Infiltrator）
  - FPSArmsAnimInstance CalculateDirection 废弃 API 修复（内联实现）
  - GA_InfiltratorScan 升级为全息 UI Toggle（Spawn+Attach+Show / Hide+SetLifeSpan）
- 2026-06-08（Session10）：
  - 蓝图配置全部完成（IA_Interact / BGA_InfiltratorScan / BP_FPSInfiltrator / BP_TheManPlayerController）
  - 排查 BP_uiFrame 显示问题：BeginPlay 自动调用 Show（Autoplay=false），与 C++ ProcessEvent Show 双重触发
  - 修复：移除 C++ 手动调用 Show，依赖 BeginPlay 自动触发
  - 用户调整 BP_uiFrame 内 Delay 参数，持久显示问题解决
  - PIE 全流程验证通过（展开 + 收缩）

---

## 完成标准核查

- [x] C++ 编译通过
- [x] E 键触发链路 PIE 验证通过
- [x] BGA_InfiltratorScan 蓝图创建 + HologramActorClass 填入 BP_uiFrame
- [x] BP_FPSInfiltrator DefaultAbilityClasses 配置
- [x] IA_Interact + IMC_Default 配置
- [x] BP_TheManPlayerController InteractAction 配置
- [x] PIE：E 键全息 UI 展开/收缩完整流程验证
