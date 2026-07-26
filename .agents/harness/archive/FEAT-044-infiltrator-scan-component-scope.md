# [FEAT-044] 扫描组件下沉到 Infiltrator 专属

**创建日期：** 2026-07-03  
**状态：** planned  
**Archive 文件：** `archive/FEAT-044-infiltrator-scan-component-scope.md`

---

## 功能概述

当前 `UScanEffectComponent` 在 `AFPSCharacterBase` 构造函数中创建，因此所有 FPS 玩家子类（包括 `AFPSMaintenanceWorker` / `AFPSTheExecutive`）组件树里都会出现 `ScanEffect`。

实际技能触发仍然是正确的：只有 `AFPSInfiltrator` 在 `PossessedBy()` 中通过 `DefaultAbilityClasses` 授予 `BGA_InfiltratorScan`，`GA_InfiltratorScan` 监听 `Input.Character.Interact` 后才会调用扫描组件。非潜行者没有该 Ability，按 E 不会扫描。

但从职责边界看，扫描组件属于潜行者专属能力，不应挂在所有玩家角色基类上。

---

## 目标方案

- 从 `AFPSCharacterBase` 移除 `ScanEffect` 成员、getter 和 `CreateDefaultSubobject<UScanEffectComponent>`。
- 在 `AFPSInfiltrator` 中创建专属 `UScanEffectComponent`，并提供访问函数或接口。
- 修改 `UGA_InfiltratorScan`：不再从 `AFPSCharacterBase::GetScanEffect()` 获取组件，改为只接受 `AFPSInfiltrator` 或扫描组件接口。
- 保持基类 `ActivateInteract()` 仍然通用发送 `Input.Character.Interact`，角色是否响应由授予的 Ability 决定。

---

## 完成标准

- [ ] `AFPSCharacterBase` 不再创建/暴露 `ScanEffect` 组件。
- [ ] 非潜行者角色 BP 组件树无 `ScanEffect`。
- [ ] `AFPSInfiltrator` 持有专属 `UScanEffectComponent`，`BP_FPSInfiltrator` 可继续配置 `MPC_ScanEffect`。
- [ ] `UGA_InfiltratorScan` 只从 Infiltrator 专属路径获取扫描组件。
- [ ] C++ Development Editor / Win64 编译通过。
- [ ] PIE：Infiltrator 按 E 扫描波/高亮/UI 正常；MaintenanceWorker/TheExecutive 按 E 不触发扫描。

---

## 实现日志

### 2026-07-03-session64

- 用户发现 `FPSMaintenanceWorker` 组件树里存在 `ScanEffect`，确认当前原因是组件挂在 `AFPSCharacterBase` 基类。
- 决定暂不改代码，登记为待优化 feature。
