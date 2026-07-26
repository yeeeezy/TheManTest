# [FEAT-002] 相机俯仰角限制

**创建日期：** 2026-06-06
**状态：** done
**Archive 文件：** `archive/FEAT-002-camera-pitch-clamp.md`

---

## 功能概述

在 `ATheManCharacterBase` 上新增两个 Blueprint 可配置的浮点变量 `PitchMin`（向上最大角度）和 `PitchMax`（向下最大角度），在 `PossessedBy` 时写入 `PlayerCameraManager` 的 `ViewPitchMin / ViewPitchMax`，由引擎原生限制相机俯仰范围。

---

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/Public/Characters/BaseCharacter/TheManCharacterBase.h`（新增两个 UPROPERTY）
- `Source/TheManTest/Private/Characters/BaseCharacter/TheManCharacterBase.cpp`（构造函数默认值 + PossessedBy 应用）

**涉及蓝图资产：**
- 所有继承自 `ATheManCharacterBase` 的 BP_ 角色蓝图——编译后可在 Camera 分类下看到并覆盖 PitchMin / PitchMax。

**依赖：** 无

**完成标准：**
- [ ] C++ 编译无错误无警告
- [ ] 蓝图 Camera 分类下可见 PitchMin / PitchMax 两个变量
- [ ] PIE 测试：向下看超过 PitchMax 时视角被锁住，向上看超过 PitchMin 时同样被锁住

---

## 实现日志

### 2026-06-06 — 功能规划

- **背景：** 相机可以旋转到完全朝下（甚至穿过地面），需要限制俯仰范围，且角度应可在蓝图中按角色差异化配置。
- **方案选择：** 使用 `APlayerCameraManager::ViewPitchMin / ViewPitchMax`，这是 UE 原生的俯仰限制机制，无需在 Look() 中每帧手动 Clamp，性能更优。
- **应用时机：** `PossessedBy`，此时 Controller 已赋值，可以安全 Cast 到 `APlayerController` 并访问 `PlayerCameraManager`。
- **默认值：** PitchMin = -89.0f（几乎可以垂直朝上），PitchMax = 60.0f（向下最多 60°，避免看穿地面）。

---

### 2026-06-06 — 实现完成

**修改文件：**
- `TheManCharacterBase.h`：Camera 分类下新增 PitchMin / PitchMax（EditAnywhere, BlueprintReadWrite）
- `TheManCharacterBase.cpp`：构造函数设默认值，PossessedBy 写入 PlayerCameraManager

**编译结果：** 待验证

---

## Bug 记录

（无）

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | 2026-06-06 | 通过 | |
| 蓝图 Camera 分类可见新变量 | 2026-06-06 | 通过 | |
| PIE 测试——向下俯仰被锁 | 2026-06-06 | 通过 | |
| PIE 测试——向上俯仰被锁 | 2026-06-06 | 通过 | |

---

## 最终备注

（待完成后填写）

**完成标准全部满足日期：** 2026-06-06
**功能关闭日期：** 2026-06-06
