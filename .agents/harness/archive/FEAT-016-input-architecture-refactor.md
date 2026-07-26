# FEAT-016 — 输入架构重构（Controller 纯注册表）

**状态：** done  
**创建日期：** 2026-06-08  
**关闭日期：** 2026-06-08

---

## 动机

旧架构中 Controller 既持有 IA 资产又路由角色方法（HandleMove → Character->Move()），导致：
- Controller 需要知道每个角色有哪些输入方法
- 新增角色专属按键必须改 Controller
- 角色切换时旧绑定不自动清理

新架构：Controller = 纯注册表，Character = 自绑。

---

## 新架构

```
ATheManPlayerController
  BeginPlay        → AddMappingContext
  SetupInputComponent → 只绑 TestSwitchCharacterAction（Controller 元操作）
  UInputAction* 资产 → 通过 GetXxxAction() getter 暴露给 Character

AFPSCharacterBase::SetupPlayerInputComponent
  → 绑定所有角色共用输入（Move / Look / Jump / SwitchEquipment / PrimaryFire / SecondaryFire）

AFPSInfiltrator::SetupPlayerInputComponent（override）
  → Super::SetupPlayerInputComponent（继承通用绑定）
  → 额外绑定 ScanAction → ActivateScan()
```

**优势：**
- Controller 对角色类型零感知
- 角色被 Unpossess 时，Pawn 的 InputComponent 自动销毁，绑定自动清理
- 新增角色专属输入：只改该角色类，Controller 不动

---

## 修改文件

| 文件 | 变更 |
|---|---|
| `Core/TheManPlayerController.h` | 移除所有 HandleXxx 路由方法；IA 资产加 public getter；新增 ScanAction |
| `Core/TheManPlayerController.cpp` | SetupInputComponent 只含 TestSwitchCharacterAction 绑定；移除所有路由实现 |
| `FPSCharacterBase/FPSCharacterBase.h` | 新增 `SetupPlayerInputComponent` override；`Move/Look/SwitchEquipment` 签名改为 `const FInputActionValue&`；新增 `virtual void ActivateScan()` |
| `FPSCharacterBase/FPSCharacterBase.cpp` | 实现 SetupPlayerInputComponent（通用输入绑定）；更新三个方法签名；新增空 ActivateScan() |
| `Infiltrator/FPSInfiltrator.h` | 新增 SetupPlayerInputComponent override、PossessedBy override、ActivateScan override、DefaultAbilityClasses |
| `Infiltrator/FPSInfiltrator.cpp` | 完整实现 |

---

## 实现日志

- 2026-06-08：全量重构完成，C++ 层实现完毕
- 重构同步带入 FEAT-017（Infiltrator 扫描技能框架）

**完成标准满足日期：** 待 PIE 验证
