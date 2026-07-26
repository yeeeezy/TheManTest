# [FEAT-001] FirstPersonMesh 作为相机父级

**创建日期：** 2026-06-06
**状态：** done
**Archive 文件：** `archive/FEAT-001-firstpersonmesh-as-camera-root.md`

---

## 功能概述

修改 `ATheManCharacterBase` 构造函数中的组件挂载层级。原结构是 HeadCamera 挂在角色 Mesh 的 head 骨骼上、FirstPersonMesh 挂在 HeadCamera 下；新结构改为 FirstPersonMesh 挂在 head 骨骼上、HeadCamera 挂在 FirstPersonMesh 下。其余属性设置和其他逻辑不变。

---

## 范围

**涉及 C++ 文件：**
- `Source/TheManTest/Private/Characters/BaseCharacter/TheManCharacterBase.cpp`（构造函数挂载顺序）

**涉及蓝图资产：**
- 所有继承自 `ATheManCharacterBase` 的 BP_ 角色蓝图需在 UE 编辑器内重新编译验证，确认组件层级无报错。

**依赖：** 无

**完成标准：**
- [ ] C++ 编译无错误无警告（Development Editor / Win64）
- [ ] 蓝图无编译错误（UE 编辑器验证）
- [ ] PIE 测试：相机视角跟随正常，第一人称手臂网格位置正确，角色切换后视角不异常

---

## 实现日志

### 2026-06-06 — 功能规划

- **背景：** 将 FirstPersonMesh 改为父级，使手臂网格体能独立控制相机位置偏移，为后续第一人称动画和武器偏移提供更合理的层级基础。
- **变更前层级：**
  ```
  GetMesh() [head 骨骼]
    └── HeadCamera
          └── FirstPersonMesh
  ```
- **变更后层级：**
  ```
  GetMesh() [head 骨骼]
    └── FirstPersonMesh
          └── HeadCamera
  ```
- **设计决策：** 仅调整构造函数中两个组件的 `SetupAttachment` 调用顺序及参数，其余属性（bOwnerNoSee、bCastDynamicShadow、bUsePawnControlRotation 等）保持不变。

---

### 2026-06-06 — 实现完成

**修改文件：** `Source/TheManTest/Private/Characters/BaseCharacter/TheManCharacterBase.cpp`

**变更内容（构造函数中）：**

原代码：
```cpp
HeadCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HeadCamera"));
HeadCamera->SetupAttachment(GetMesh(), FName("head"));
HeadCamera->bUsePawnControlRotation = true;

FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
FirstPersonMesh->SetOnlyOwnerSee(true);
FirstPersonMesh->SetupAttachment(HeadCamera);   // 挂在相机下
FirstPersonMesh->bCastDynamicShadow = false;
FirstPersonMesh->CastShadow = false;
```

修改后：
```cpp
FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
FirstPersonMesh->SetOnlyOwnerSee(true);
FirstPersonMesh->SetupAttachment(GetMesh(), FName("head"));  // 直接挂在 head 骨骼上
FirstPersonMesh->bCastDynamicShadow = false;
FirstPersonMesh->CastShadow = false;

HeadCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HeadCamera"));
HeadCamera->SetupAttachment(FirstPersonMesh);   // 挂在 FirstPersonMesh 下
HeadCamera->bUsePawnControlRotation = true;
```

**编译结果：** 待验证

---

## Bug 记录

### BUG-001 — HeadCamera 挂到 FirstPersonMesh 根节点导致相机跑到脚底

**发现日期：** 2026-06-06
**严重程度：** 高
**状态：** fixed

**现象：** 编译后相机出现在角色脚底。

**根本原因：** `HeadCamera->SetupAttachment(FirstPersonMesh)` 未指定插槽名，HeadCamera 挂到了 FirstPersonMesh 的本地根节点，而非 head 骨骼位置。

**修复方案：** 2026-06-06
- 将 `SetupAttachment(FirstPersonMesh)` 改为 `SetupAttachment(FirstPersonMesh, FName("head"))`。

---

### BUG-002 — FirstPersonMesh 被错误挂载到 GetMesh() 的 head 骨骼上

**发现日期：** 2026-06-06
**严重程度：** 高
**状态：** fixed

**现象：** FirstPersonMesh 整体飞到头部高度，与第三人称 Mesh 错位。

**根本原因：** FirstPersonMesh 与 GetMesh() 是同一套骨骼，需保持完全相同的根位置叠放。错误地将 FirstPersonMesh 挂到 `GetMesh()` 的 `"head"` 骨骼，导致它偏移到头部坐标，脱离角色身体。

**修复方案：** 2026-06-06
- 将 `FirstPersonMesh->SetupAttachment(GetMesh(), FName("head"))` 改为 `FirstPersonMesh->SetupAttachment(GetMesh())`，挂到 GetMesh() 根节点，两套 Mesh 完全重叠。

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | 2026-06-06 | 通过 | |
| 蓝图无编译错误（编辑器） | 2026-06-06 | 通过 | |
| PIE 测试——相机跟随 | 2026-06-06 | 通过 | |
| PIE 测试——第一人称手臂位置 | 2026-06-06 | 通过 | |
| PIE 测试——角色切换后视角 | 2026-06-06 | 通过 | |

---

## 最终备注

（待完成后填写）

**完成标准全部满足日期：** 2026-06-06
**功能关闭日期：** 2026-06-06
