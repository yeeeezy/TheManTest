# FEAT-018 — 扫描高亮系统

**状态：** done  
**创建日期：** 2026-06-08  
**关闭日期：** 2026-06-08-session14

---

## 功能描述

扫描波（FEAT-015）经过场景物体时，自动对实现 `IHighlightable` 接口的 Actor 触发轮廓线高亮。高亮通过 Post Process 材质读取 Custom Depth 实现深度边缘检测，而非 SetOverlayMaterial。

---

## 最终架构

### 类关系

```
AInteractableBase（AActor + IHighlightable）
  └── UHighlightComponent（默认挂载）
        └── StartHighlight() → SetRenderCustomDepth(true) on all UMeshComponent
        └── StopHighlight()  → SetRenderCustomDepth(false)

UScanEffectComponent（已有，扩展）
  └── Tick 中 SphereOverlapActors(WorldSpaceOrigin, CurrentRadius)
        → 找到实现 IHighlightable 的 Actor
        → Execute_StartHighlight(Actor, HighlightDuration)
        → 加入 AlreadyHighlighted（去重，TriggerScan 时清空）

M_Highlight（Post Process 材质）
  └── Domain = Post Process
  └── Blendable Location = Scene Color Before Bloom（挂后处理体积）
  └── 5 个 SampleSceneDepth → Use Custom Depth = true
  └── Laplacian 深度边缘检测 → Saturate → × Color.A × Distance Fade
  └── Lerp(SceneTexture_RGB, Color.RGB, Alpha) → Emissive
```

### 新增文件

| 文件 | 说明 |
|---|---|
| `Public/Interfaces/Highlightable.h` | IHighlightable 接口（StartHighlight / StopHighlight） |
| `Public/Characters/Components/HighlightComponent.h` | 高亮组件声明 |
| `Private/Characters/Components/HighlightComponent.cpp` | SetRenderCustomDepth 实现 |
| `Public/Actors/InteractableBase.h` | 可交互基类声明 |
| `Private/Actors/InteractableBase.cpp` | 默认挂载组件，委托高亮给组件 |

### 修改文件

| 文件 | 变更 |
|---|---|
| `ScanEffectComponent.h` | 新增 MaxScanRadius / HighlightDuration / AlreadyHighlighted / DetectAndHighlight() |
| `ScanEffectComponent.cpp` | TriggerScan 清空 AlreadyHighlighted；Tick 调用 DetectAndHighlight |

### 材质参数（M_Highlight）

| 参数名 | 类型 | 默认值 | 用途 |
|---|---|---|---|
| OutlineWidth | Scalar | 1.0 | BlurSampleOffsets 偏移量，控制轮廓线粗细 |
| Amount | Scalar | 0.1 | 边缘灵敏度 |
| Power | Scalar | 2.0 | 边缘锐度 |
| ViewDistance | Scalar | 10000 | 距离淡出范围 |
| Color | Vector | (R=1,G=0,B=0.05,A=10) | RGB=颜色，A=亮度倍率 |

---

## 实现日志

### 2026-06-08 Session13 — C++ 框架完成

**材质问题排查：**
- M_Highlight 使用 SceneTexture + SampleSceneDepth，是 Post Process 材质，不能用 SetOverlayMaterial
- Color.RGBA（float4）被用作 Lerp Alpha（float1），R=0 导致 Alpha 永远为 0，高亮不可见
- 修复：Multiply_34.A 改接 Color.A（alpha 通道 = 10）
- Blendable Location 在 UE5.7 对应名称：Scene Color Before Bloom

**C++ 实现完成，未编译：**
- IHighlightable 接口（BlueprintNativeEvent）
- UHighlightComponent（GetComponents<UMeshComponent> → SetRenderCustomDepth）
- AInteractableBase（默认挂 HighlightComponent，委托接口实现）
- ScanEffectComponent 扩展（SphereOverlapActors + AlreadyHighlighted 去重）

---

## Session14 补充修改

- `AInteractableBase` 新增 `UStaticMeshComponent`（RootComponent）和 `USkeletalMeshComponent`，解决 `GetComponents<UMeshComponent>` 找不到网格导致高亮静默失败的问题
- `ScanEffectComponent::DetectAndHighlight` 移出 `else` 分支，扫描完成帧也调用一次，修复最外圈物体漏检 bug
- `UHighlightComponent::AutoFadeDuration` 字段删除（冗余），Duration 统一由 `ScanEffectComponent.HighlightDuration` 控制
- `DetectAndHighlight` 新增 `FindComponentByClass<UHighlightComponent>` 回退路径：Actor 未实现 `IHighlightable` 时直接检测组件，任意 Actor 加 `HighlightComponent` 即可高亮，无需继承 `AInteractableBase`
- 扫描波缓动从 EaseOut 五次方改为二次方（`pow(1-t, 2)`），扩张速度更平缓

## 未解决问题 / 阻塞

无。

---

## 完成标准核查

- [x] C++ 编译无错误无警告
- [x] Project Settings → Custom Depth-Stencil Pass = Enabled with Stencil
- [x] M_Highlight：5 个 SampleSceneDepth Use Custom Depth = true
- [x] M_Highlight 挂入后处理体积 Blendable Array
- [x] 基于 AInteractableBase 创建 BP_InteractableTest
- [x] PIE：扫描波经过 → 轮廓线出现
