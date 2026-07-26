# FEAT-015 — 扫描材质控制系统

**状态：** done  
**创建日期：** 2026-06-08  
**关闭日期：** 2026-06-08-session12

---

## 功能描述

为已有的球形扫描材质（Mat_Outline_library 中的扫描波节点组）建立完整的运行时控制机制。

材质逻辑：
- `ScanTime × Frac` 驱动扫描波在 [0, 最大半径] 之间膨胀
- 外球 SphereMask（Radius = 半径 × Frac）减去内球 SphereMask（Radius = 半径 × Frac × 扫描厚度）= 薄环遮罩
- 扫描原点通过 MPC Vector 参数（相机相对坐标）传入

## 最终架构

### MPC_ScanEffect 参数

| 参数名 | 类型 | 默认值 | 用途 |
|---|---|---|---|
| `ScanTime` | Scalar | 0 | 0→1 驱动扩张进度（EaseOut 曲线，C++ Tick 写入） |
| `ScanAlpha` | Scalar | 0 | 效果开关（0=隐藏，1=显示，避免默认显示） |
| `ScanOrigin` | Vector | (0,0,0,0) | 扫描原点（相机相对坐标，C++ 每帧写入 WorldOrigin - CamPos） |

### 材质节点结构（解决 UE5 SM6 LWC 类型冲突）

**UE5.5+ SM6 的 LWC 问题**：`AbsoluteWorldPosition` 输出 FWSVector3（LWC 类型），SphereMask 不接受 LWC 输入，导致 `Invalid input types: FWSVector3, FWSVector3` 错误。

**解决方案**：
- SphereMask **A 引脚**：WorldPosition 节点，Shader Offsets = **Camera Relative World Position**（输出 float3，非 LWC）
- SphereMask **B 引脚**：CollectionParameter(ScanOrigin) → ComponentMask(RGB) → SphereMask B（float4 → float3）
- 两者均为 float3，SphereMask 正常编译

**C++ ScanOrigin 的维护**：每帧在 Tick 里计算 `WorldSpaceOrigin - CameraPos` 并写入 MPC，与材质的 Camera Relative 坐标对齐。

### C++ 组件（UScanEffectComponent）

- 挂在 `AFPSCharacterBase` 上，`GetScanEffect()` getter 供 GA 调用
- `TriggerScan(FVector Origin)`：设 ScanAlpha=1，ScanTime=0，bScanning=true，开启 Tick
- `RetractScan()`：bRetracting=true，开启 Tick
- Tick：
  - 每帧计算相机相对坐标，写入 ScanOrigin
  - bScanning：ScanProgress 0→1，EaseOut `1-(1-t)²`，写入 ScanTime；完成后关 Tick
  - bRetracting：ScanProgress 1→0，写入 ScanTime；到 0 后设 ScanAlpha=0，关 Tick
- `ScanDuration`：默认 2 秒（可在蓝图 Details 调整）

---

## 实现日志

### 2026-06-08 — 功能立项

- 用户提供了完整的材质节点数据（SphereMask 双球差分方案）
- 分析材质逻辑，确认扫描原点当前为 CameraPositionWS，需替换为 Vector Parameter
- 确认 Time 节点需替换为手动 Scalar 以支持一次性触发

### 2026-06-08 — C++ 层完成（未编译）

- 新建 `Public/Characters/Components/ScanEffectComponent.h`
- 新建 `Private/Characters/Components/ScanEffectComponent.cpp`
- `FPSCharacterBase` 新增 `ScanEffect` 组件 + `GetScanEffect()` getter
- `GA_InfiltratorScan::ActivateAbility`：展开全息 UI 时调用 `TriggerScan(CharacterLocation)`；收起时调用 `RetractScan()`

### 2026-06-08 — Session11：材质节点调试（进行中）

**遇到的 LWC 类型冲突问题（完整排查记录）：**

1. **首次错误**：`Arithmetic between types LWCVector3 and float4 are undefined` — SphereMask A 为 AbsoluteWorldPosition(LWC)，B 为 CollectionParameter(float4)，类型不匹配
2. **错误修复尝试（失败）**：建议用 `Subtract(AbsoluteWorldPosition - CameraPositionWS)` 方案，触发新错误 `float3 and float4`
3. **用户反馈**：节点太复杂，原版 `CameraPositionWS` 本身没有报错
4. **最终诊断**：根本原因是 UE5 SM6 LWC — `AbsoluteWorldPosition` 输出 `FWSVector3`，不是 float3；SphereMask 在 SM6 下不接受 LWC 输入
5. **正确方案**：WorldPosition 节点改为 `Camera Relative World Position` 模式（输出 float3），B 用 ComponentMask(ScanOrigin, RGB) 得到 float3

**MPC 参数名迭代：**
- 初版使用中文名（`扫描原点`、`手动时间`）→ 运行时报错 `invalid ParameterName '扫描原点'`
- 改为英文：`ScanTime`、`ScanOrigin`、`ScanAlpha`

**ScanAlpha 参数引入原因：**
- 材质默认值下扫描环始终可见（ScanTime=0 时仍有残影）
- 用 `ScanAlpha` Scalar 控制最终 Add 的强度，TriggerScan 置 1，回缩完成后置 0

**黑屏 Bug 修复：**
- 最初将 SceneTexture(PostProcessInput0) float4 直接乘以 ScanAlpha → Alpha 通道变 0 → 黑屏
- 修复：在 SceneTexture 后加 ComponentMask(RGB)，只取 RGB 通道做 Add

### 2026-06-08 — Session12：材质改动确认全部完成

**用户提供了当前材质的完整节点数据，逐项核查结果：**

- ✅ 节点 233（WorldPosition_5）：`WPT_CameraRelative` — A 引脚 LWC 已修
- ✅ 节点 238（WorldPosition_3）：`WPT_CameraRelative` — A 引脚 LWC 已修
- ✅ 节点 240（ComponentMask_8）← 节点 227（CollectionParam ScanOrigin）→ SphereMask_0 B — B 引脚已修
- ✅ 节点 239（ComponentMask_9）← 节点 228（CollectionParam ScanOrigin）→ SphereMask_1 B — B 引脚已修
- ✅ 节点 226（CollectionParam ScanTime）→ 节点 201（Frac）→ 节点 195（Multiply×半径）→ SphereMask Radius — ScanTime 驱动已修
- ✅ 节点 230（CollectionParam ScanAlpha）→ 节点 231（Multiply）→ 节点 0（Add → Emissive）— ScanAlpha 门控已完成
- ✅ MPC_ScanEffect 已存在：`/Game/Characters/Infiltrator/Material/MPC_ScanEffect`

SphereMask 节点上残留的 `"Invalid input types: FWSVector3"` 错误是**上次编译的旧缓存**，节点已修改但未重编译。材质编辑器点 Apply 即可清除。

---

## 未解决问题 / 阻塞

（无阻塞，功能已关闭）

---

## 完成标准核查

- [x] `MPC_ScanEffect` 创建（ScanTime Scalar / ScanAlpha Scalar / ScanOrigin Vector）
- [x] 材质 SphereMask A：两个 WorldPosition 节点改为 `Camera Relative World Position`（WPT_CameraRelative）
- [x] 材质 SphereMask B：CollectionParameter(ScanOrigin) → ComponentMask(RGB) → B
- [x] 材质 ScanAlpha：扫描效果 × ScanAlpha → Emissive
- [x] 材质 ScanTime：CollectionParam → Frac → SphereMask Radius
- [x] C++ TriggerScan / RetractScan / Tick EaseOut（UScanEffectComponent）
- [x] GA_InfiltratorScan 调用 TriggerScan / RetractScan
- [x] 材质编辑器 Apply，LWC 错误消失
- [x] C++ 编译通过
- [x] BP_FPSInfiltrator ScanEffect 组件 → ScanMPC 赋值 MPC_ScanEffect
- [x] PIE 验证：E 键 → 全息 UI 展开 + 扫描波从角色位置以 EaseOut 三次曲线 2s 膨胀；再按 E → 波形立即消失

---

## 最终实现要点

- **EaseOut 曲线**：`1 - (1-t)⁵`（五次方，前段极快/后段极缓；前20%时间完成约67%位移）
- **ScanDuration**：2 秒（`EditDefaultsOnly`，蓝图 Details 可调）
- **RetractScan 行为**：即时隐藏（ScanAlpha=0、ScanTime=0、停止 Tick），不播放回缩动画
- **ScanOrigin 坐标系**：每帧 `WorldSpaceOrigin - CameraPos`，与材质 Camera Relative WorldPosition 对齐
- **Emissive 结构**：`SceneTexture_RGB + (描边+扫描环) × ScanAlpha`（ScanAlpha=0 时纯透传）
