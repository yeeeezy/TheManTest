# [FEAT-041] 删除弃用旧角色系统 + 玩家动画实例改名

**创建日期：** 2026-06-24
**状态：** in_progress（C++ 已改，待用户编译 + 编辑器删旧蓝图验证）
**Archive 文件：** `archive/FEAT-041-remove-deprecated-character-system.md`

---

## 功能概述

清理弃用旧角色系统并给玩家动画实例正名，为 FEAT-038/039（三件套全身骨架）扫清命名/死代码障碍。

1. **删除旧弃用 `ATheManCharacterBase` 系统**（代码自洽、现役 FPS/Core/敌人系统无任何引用，grep 确认）。
2. **改名 `UFPSArmsAnimInstance` → `UFPSCharacterAnimInstance`**：该类现在驱动手臂/影子/腿三件套共享的全身姿势，"Arms" 名不副实；改成与 `AFPSCharacterBase` 对齐的通用名。+ CoreRedirect 保留旧 ABP 父类链接。

---

## 设计决策（2026-06-24-session43 与用户确认）

- 删的是**非 FPS 前缀**那批（旧系统），FPS* 现役类全保留。
- 动画实例只删旧 `UTheManAnimInstanceBase`；保留 `UBaseLocomotionAnimInstance` / `UFPSCharacterAnimInstance`(原 Arms) / `UHumanoidEnemyAnimInstance` / `UFirearmAnimInstance`。
- 不把内容折进 `UTheManAnimInstanceBase`（那是要删的旧类），而是直接改名旧 `UFPSArmsAnimInstance`（本就是空子类，仅作玩家专属父类锚点）。
- 项目无 git，删除前**全部备份**到 scratchpad `deprecated-char-backup-session43/`（保留相对路径，可还原）。

---

## 范围

**删除的 C++ 文件（10 个，旧系统）：**
- `Public/Characters/BaseCharacter/TheManCharacterBase.h` + `Private/.../TheManCharacterBase.cpp`
- `Public/Characters/BaseCharacter/Animation/TheManAnimInstanceBase.h` + `Private/.../TheManAnimInstanceBase.cpp`
- `Public/Characters/Infiltrator/Infiltrator.h` + `Private/.../Infiltrator.cpp`
- `Public/Characters/MaintenanceWorker/MaintenanceWorker.h` + `Private/.../MaintenanceWorker.cpp`
- `Public/Characters/TheExecutive/TheExecutive.h` + `Private/.../TheExecutive.cpp`

**改名的 C++ 文件（2 个）：**
- 删 `Public/Characters/FPSCharacterBase/Animation/FPSArmsAnimInstance.h` + `Private/.../FPSArmsAnimInstance.cpp`
- 建 `Public/Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h` + `Private/.../FPSCharacterAnimInstance.cpp`（`UFPSCharacterAnimInstance : UBaseLocomotionAnimInstance`，内容仍为空，留扩展点）

**配置：**
- `Config/DefaultEngine.ini` 新增 `[CoreRedirects]`：`+ClassRedirects=(OldName="/Script/TheManTest.FPSArmsAnimInstance",NewName="/Script/TheManTest.FPSCharacterAnimInstance")` → 现有 ABP（parent 到旧类）自动重链，无需手动 reparent。

**保留（虽同目录但现役在用，未动）：** `TheManCharacterDataAssetBase.*`（FPSCharacterBase 的 CharacterData）、`TheManAttributeSetBase.*`（GAS 属性集）。

**备份位置：** `<scratchpad>/deprecated-char-backup-session43/`（12 个文件，含 2 个改名前的旧 anim 文件）。

**完成标准：**
- [ ] C++ 编译无错误无警告（Development Editor / Win64）—— 删除 + 改名后
- [ ] 编辑器：删除旧蓝图资产，确认无"父类丢失"报错残留：`BP_TheManCharacterBase` / `BP_Infiltrator` / `BP_MaintenanceWorker` / `BP_TheExecutive` / `ABP_MainCharacter` / `ABP_FirstPerson_MainCharacter`
- [ ] 编辑器：现役 `ABP_FPSArms` / `ABP_FPS_Arm_MainCharacter` 父类经 CoreRedirect 自动显示为 `UFPSCharacterAnimInstance`，编译通过

---

## 实现日志

### 2026-06-24-session43 — 删除 + 改名（C++ 完成，待编译/编辑器验证）

- grep 确认 5 个旧类名仅在自身那一簇 7 文件出现、include 也仅内部互引，外部零引用 → 一起删安全。
- `UFPSArmsAnimInstance` 经 grep 仅在自身两文件出现（空子类），改名零外部 C++ 影响。
- 执行：备份 12 文件 → 删 12 → 建 2 个改名后新文件 → 加 CoreRedirect。最终 grep 全库无残留引用。
- **编译结果：** ✓ `Build.bat TheManTestEditor Win64 Development` 通过（与 FEAT-038 一起编）；`FPSCharacterAnimInstance.cpp` 正常编译、被删文件已从 unity 列表消失，确认删除+改名生效。
- **待：** 编辑器删上述 6 个旧蓝图资产 + 确认 ABP_FPSArms 经 CoreRedirect 自动重链。

---

## Bug 记录

（暂无）

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（删除+改名后） | 2026-06-24 | ✓ | session43 Build.bat 通过 |
| 编辑器：旧蓝图删除无报错 | — | ⏳ | 6 个旧 BP/ABP |
| 编辑器：ABP_FPSArms 父类自动重链 UFPSCharacterAnimInstance | — | ⏳ | CoreRedirect |

---

## 最终备注

> - 备份在 scratchpad，会话级目录，若要长期保留请尽快转存。
> - 删 C++ 后旧蓝图会"父类丢失"，必须在编辑器一并删除，否则每次加载报错。
> - CLAUDE.md / arch 文档里"旧弃用系统（ATheManCharacterBase）代码保留"的描述已过时，落地验证后需同步更新（03-character-base / 06 / 12 / system-overview）。
