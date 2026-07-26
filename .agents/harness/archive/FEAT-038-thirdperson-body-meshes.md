# [FEAT-038] 第三人称全身骨架 + 三件套 Mesh 渲染分离（C++ 主架构）

**创建日期：** 2026-06-24
**状态：** planned（方案已与用户敲定，待开工）
**Archive 文件：** `archive/FEAT-038-thirdperson-body-meshes.md`

---

## 功能概述

把现有"纯手臂"第一人称角色升级为「**单套全身骨架驱动三件套 mesh**」：第一人称手臂 + 投影用全身 + 可见下半身腿，三者共用同一套**新骨架、新动画**，靠 Leader/Follower 共享同一份姿势（动画只算一次）。

目标视觉：玩家低头能看到自己的腿，地上/墙上有完整人形影子，手臂视角和操作手感与现在完全一致。

锁定的组件布局：
```
Capsule (root)   [bUseControllerRotationPitch=true, yaw=true]   ← 不变
├─ ArmsMesh        = Leader + 动画宿主，挂主 ABP，跟 capsule 俯仰，只渲染手臂(材质段)
│   └─ HeadCamera                                               ← 保持原样：挂 ArmsMesh "head" 骨骼, bUsePawnControlRotation
└─ BodyRoot (USceneComponent, 绝对旋转, 每帧只取 Yaw → 永远直立)
    ├─ ShadowBodyMesh = Follower，全身，OwnerNoSee + CastHiddenShadow（只投影）
    └─ LegsMesh       = Follower，只渲染腿(材质段)，OnlyOwnerSee，无影
```

---

## 设计决策（2026-06-24 与用户确认）

- **单骨架 + 单动画 + Leader/Follower**：三 mesh 引用同一个 Skeleton 资产。ArmsMesh 当 Leader/动画宿主（保留现状——所有装备/武器层链接、`EquipmentManager::AttachTargetMesh` 都已指向 ArmsMesh，不用改）；Shadow/Legs `SetLeaderPoseComponent(ArmsMesh)` 零成本复制同一份姿势 → "三者动画一样"天然成立。
- **俯仰用方案 B**：ArmsMesh 仍跟 capsule 整体俯仰（`bUseControllerRotationPitch=true` 不变，手臂俯仰范围满、能指正下方）；Shadow/Legs 挂在 `BodyRoot` 下，`BodyRoot` 标记绝对旋转、每帧只取 Yaw → 身体直立、投影不歪。手臂"硬俯仰"与影子上半身"AimIK 掰"的俯仰量不一致问题，**优先级低**，可后续用身体 AimIK 摆正（FEAT-040），也可不摆。Leader/Follower 复制的是组件空间骨骼变换，组件自身世界旋转独立 → 同一份姿势、手臂俯仰 / 身体直立 互不冲突。
- **几何分离只能用材质，禁用隐藏骨骼**：`HideBoneByName` 会改骨骼变换（姿势本身），Leader 上隐藏会连累 Follower（影子缺上半身），Follower 上隐藏又被 Leader 复制覆盖。故"只露手臂 / 只露腿"一律用 **`HideMaterialSection`（优先，前提是身体 mesh 分了手臂/躯干/腿材质段）** 或材质 OpacityMask；只改渲染、不碰姿势。投影 mesh 不挂任何遮罩 → 照投完整影子。
- **相机保持原样**：仍挂 ArmsMesh "head" 骨骼 + `bUsePawnControlRotation=true`，视角行为和现在一致。唯一副作用：ArmsMesh 改跑全身 locomotion 后 head 骨骼有走路 head bob → 相机位置轻微晃动。**这是手感调参项（压小动画头部位移 / 加相机阻尼曲线），不是架构阻塞**，不挡开工。
- **新骨架 + 新动画全部替换**：身体骨架和第三人称动画来源由用户提供全新一套；C++ 所有骨骼名（投影/腿隐藏的材质段、手部 socket 等）做成 `EditDefaultsOnly` 配置，换骨架只改蓝图不动代码。

---

## 范围

**涉及 C++ 文件：**
- `Public/Characters/FPSCharacterBase/FPSCharacterBase.h` / `Private/.../FPSCharacterBase.cpp`
  - 新增组件：`BodyRoot`(USceneComponent)、`ShadowBodyMesh`(USkeletalMeshComponent)、`LegsMesh`(USkeletalMeshComponent)；两 mesh 挂 BodyRoot 下。
  - 构造：`BodyRoot->SetUsingAbsoluteRotation(true)`；两 mesh `VisibilityBasedAnimTickOption = AlwaysTickPoseAndRefreshBones`（与 ArmsMesh 一致，切角色首帧姿势就位）。
  - `BeginPlay`：
    - `ShadowBodyMesh->SetLeaderPoseComponent(ArmsMesh)`、`LegsMesh->SetLeaderPoseComponent(ArmsMesh)`。
    - ShadowBodyMesh：`SetOwnerNoSee(true)` + `CastShadow=true` + `bCastHiddenShadow=true`。
    - LegsMesh：`SetOnlyOwnerSee(true)` + `CastShadow=false`。
    - 遍历 `LegsHiddenSections` / `ArmsHiddenSections`（新增 `EditDefaultsOnly TArray<int32>`）调 `HideMaterialSection` 隐藏不需要的材质段。
    - 把现有 BeginPlay 隐藏 + `RevealArmsAndWeapon` 显隐逻辑扩展到 Shadow/Legs（切角色首帧不闪）。
  - `Tick`：`BodyRoot->SetWorldRotation(FRotator(0, GetActorRotation().Yaw, 0))`（身体直立、只随偏航）。
  - getter：`GetBodyRoot()` / `GetShadowBodyMesh()`（FEAT-040 影子武器挂载会用）。

**涉及蓝图 / 编辑器（用户操作）：**
- 导入新全身骨架 + 身体 mesh（一张全身 mesh 分好材质段即可：手臂/躯干/腿各占材质槽，手臂与腿靠隐藏段，无需单独导"只有腿"资产）。
- 各角色 BP（`BP_FPSInfiltrator` / `BP_FPSMaintenanceWorker` / `BP_FPSTheExecutive` 等）填 ArmsMesh / ShadowBodyMesh / LegsMesh 的 SkeletalMesh + 各自的 `LegsHiddenSections` / `ArmsHiddenSections` 材质段索引。
- 本阶段动画可先挂一个简单 idle/walk ABP 验证骨架驱动；正式动画框架在 FEAT-039。

**依赖：**
- FEAT-005 `AFPSCharacterBase` 组件基类。
- 后续 FEAT-039（新主 ABP）、FEAT-040（影子武器 + 身体 AimIK）。

**完成标准（与 feature_list.json 一致）：**
- [ ] C++ 编译无错误无警告（Development Editor / Win64）
- [ ] 编辑器：导入新骨架/身体 mesh，角色 BP 填好三件套 mesh + 隐藏材质段
- [ ] PIE：地上出现完整人形影子（含上半身）；低头看到自己的腿；手臂视角/俯仰与现在一致；切角色无异常、首帧不闪

---

## 实现日志

### 2026-06-24-session43 — 功能创建（方案敲定）

- **背景：** 用户要求把纯第一人称手臂角色改成"第一人称 + 第三人称两套表现"的角色。经多轮讨论收敛为：单套新骨架驱动三件套 mesh（手臂可见 / 全身只投影 / 下半身腿可见），三者共享同一份动画。
- **关键讨论结论：**
  - 隐藏骨骼方案被否（破坏 Leader/Follower 共享姿势），改用材质段隐藏。
  - 俯仰用方案 B（手臂硬俯仰跟现状，身体 BodyRoot 直立）。
  - 相机保持原样挂 head 骨骼（head bob 列为调参待办）。

### 2026-06-24-session43 — C++ 实现完成 + 编译通过

- `FPSCharacterBase.h`：加 `BodyRoot`(USceneComponent) / `ShadowBodyMesh` / `LegsMesh` 三组件 + getter；新增 `EditDefaultsOnly TArray<int32> ArmsHiddenSections / LegsHiddenSections`；前向声明 `USceneComponent`。
- `FPSCharacterBase.cpp`：
  - 构造：BodyRoot 挂 RootComponent + `SetUsingAbsoluteRotation(true)`；ShadowBodyMesh 挂 BodyRoot（NoCollision + `SetOwnerNoSee(true)` + `CastShadow=true` + `bCastHiddenShadow=true` + AlwaysTickPose）；LegsMesh 挂 BodyRoot（NoCollision + `SetOnlyOwnerSee(true)` + `CastShadow=false` + AlwaysTickPose）。
  - BeginPlay：`ShadowBodyMesh/LegsMesh->SetLeaderPoseComponent(ArmsMesh)`；`HideMeshMaterialSlots()` 对 Arms/Legs 隐藏配置的材质段；并把首帧隐藏（防参考姿势闪现）扩展到 Shadow/Legs。
  - `RevealArmsAndWeapon`：姿势就绪后一并恢复 Shadow/Legs 显示。
  - Tick：`BodyRoot->SetWorldRotation(FRotator(0, GetActorRotation().Yaw, 0))` 维持直立。
- **材质段隐藏 API 修正（编译踩坑）：** UE5.7 **无 `HideMaterialSection`**，已查引擎头改用 `ShowMaterialSection(MaterialID, SectionIndex, bShow=false, LODIndex)`。helper `HideMeshMaterialSlots` 首版按"材质槽 index == LOD0 section index"处理，实际 mesh 布局不同需调整。
- **编译结果：** ✓ `Build.bat TheManTestEditor Win64 Development` 通过，无错误无警告（与 FEAT-041 删除+改名一起编）。
- **待：** 编辑器导入新骨架/身体 mesh + 角色 BP 配三件套 + 隐藏段；PIE 验证。
- **⚠️ 蓝图 mesh 摆放提醒：** Shadow/Legs 挂在 BodyRoot（绝对旋转、世界 Yaw）下，标准骨骼 mesh 需在**组件相对变换**里设 Yaw=-90（朝前）+ Z=-CapsuleHalfHeight（脚踩地），同默认 GetMesh()。

### 2026-06-24-session45 — ⚠️ 几何分离方案改为「物理拆 mesh」（材质段方案对此 mesh 不适用）

**起因**：用户实际 mesh 材质槽是**按服装类型**分的（`clothes / clothes_cloth / helmet / body / head / hair`），**不是按身体部位**（手臂/躯干/腿）。`body`(皮肤) 和 `clothes`(衣服) 各自是连着手臂+躯干+腿的**一整块焊死网格**。

**结论：材质段方案（`ShowMaterialSection` + `ArmsHiddenSections`/`LegsHiddenSections`）对此 mesh 行不通。**
- 只 head/hair/helmet 能单独切；手臂 vs 躯干 vs 腿切不开。
- 强行按区域重分材质 ID 会**毁掉原贴图**（手臂上同时有皮肤+袖子，需各自材质）。

**用户拍板（session45）：物理拆 mesh，只拆 2 块。**
- 在 Blender 里**按骨骼(顶点组)选**手臂骨骼组(upperarm/lowerarm/hand ±) → `P` Separate 出**手臂 mesh**；选腿骨骼组(thigh/calf/foot) → Separate 出**下半身 mesh**。蒙皮权重自动保留，肩/胯边界毛糙但 FP 看不到。
- **原始整块 mesh 不动**，直接给 ShadowBodyMesh 当投影。
- 导回 UE：ArmsMesh=手臂 mesh / LegsMesh=下半身 mesh / ShadowBodyMesh=原整块。
- **C++ 不改**：`ArmsHiddenSections`/`LegsHiddenSections` **留空**，三组件各挂各自 mesh 即可。`HideMeshMaterialSlots` 对此 mesh 形同空操作（保留，换别的分好区域材质的 mesh 时仍可用）。

**下次待办**：用户去 Blender 物理拆（新手，约 1h，按骨骼选 → Separate → 导回）；Claude 可写新手 Blender 一步步流程（导出 FBX→导入→按骨骼选→拆→导回设置）。

### 2026-06-25-session46 — 物理拆 mesh 实操完成（Arms 已导回，Legs 待确认）

带 Blender 新手用户一步步走完物理拆 mesh：

- **源 FBX**：用户无源文件 → UE 里右键 `SKM_CyberpunkMetalhead_FullBodyA`（`/Game/Characters/MaintenanceWorker/Meshes`）→ Asset Actions → Export 出 FBX。
- **mesh 结构**：实际是**一整块网格物体**（非按服装分多块，与 session45 推测不同，但不影响——一块更省事，无需 Ctrl+J 合并）。骨架标准 UE 命名（root/pelvis/clavicle/upperarm/lowerarm/hand/手指/thigh?/calf/foot/ball + ik_foot_root/ik_hand_root/center_of_mass/interaction 等非形变骨）。
- **拆法**（实测可行的新手流程）：Blender 导入 FBX → 选网格 `Tab` 进 Edit Mode → `Alt+Z` 开 X-Ray → 小键盘 `1` 正视图 → `B` 框选两条手臂（肩→指尖）→ `P` → Selection 分出 **Arms**；`Alt+A` 清空 → 框选两条腿（胯→脚）→ `P` → Selection 分出 **Legs**；**原剩余块（躯干+头）不导出**，UE 里 Shadow 直接用原资产。
- **导出设置**：选**目标网格 + 骨架物体 `SKM_CyberpunkMetalhead_F`**（两行底色高亮；骨架选中**不变橙、只描细边**，易误以为没选上）→ File→Export→FBX → **Limit to: Selected Objects ✅** + **Armature 组 Add Leaf Bones 去勾**。
- **导回 UE**：拖入 → 导入窗口 **Skeleton 指定原骨架**（不留空，否则新建独立骨架破坏 Leader/Follower）。

**踩坑记录（新手流程要点）：**
1. **骨架没选 → 静态网格**：第一次导出 Object Types 只剩 Mesh / 没选骨架物体 → FBX 无骨骼 → UE 当**静态网格**导入。修：导出前确保**网格 + 骨架两行都高亮**（骨架=最顶层 `SKM_CyberpunkMetalhead_F`，含 root/Pose/全部骨头的父项；`root` 只是其中一根骨头不是骨架本身）。Object Types 保持默认全选即可，别只留 Mesh。
2. **Skeleton Conflicts（root 冲突）**：UE→Blender→UE round-trip 后根骨头多出一节（树显示 `root → SKM_..._FullBodyA_001 → root`）→ 导入弹冲突。**用户已自行解决**（继续导入到原骨架，按名字对上）。若复现可试 Armature FBXNode Type=Null / Only Deform Bones 去勾，或在冲突窗口选继续用原骨架。
3. **重复材质**：导入会新建一堆重复材质 → 在 mesh 资产 Material Slots 把每槽换回**原材质** → 删多余（先换引用再删，避免 in-use 报错；不确定用 Reference Viewer）。

**当前状态**：**Arms 已导入成骨骼网格、套原骨架、材质换好。Legs 按同样流程做——下次开机先确认 Legs 是否完成。** CapsuleComponent Half Height = **88** → Shadow/Legs 组件 Location **Z=-88**。

**下次待办（编辑器，见 progress.md「第 14 步」）**：角色 BP 装三件套（ArmsMesh=Arms / LegsMesh=Legs / ShadowBodyMesh=原整块）；Shadow/Legs 设 **Rotation Yaw=-90 + Location Z=-88**；ArmsMesh 旋转不管（C++ 控）；编译 → PIE 验证影子完整 + 看到腿 + 手臂一致 + 切角色不闪 → 转 done。

### 2026-06-25-session47 — 相机方案修正 + 三处参数微调（C++）

PIE 测试三件套时发现两个手感问题，用户决定改相机挂载方案（对 session43 原"相机保持挂 head 骨骼"设计的修正）：

- **症状**：①相机随动画 head bob 太晃；②低头时视角被甩向前下方、很难看到脚——根因是相机挂在 ArmsMesh 的 `head` 骨骼上，而 ArmsMesh 跟 capsule 整体俯仰（`bUseControllerRotationPitch=true`），俯仰支点在 capsule 中心而非肩/腋窝，head 骨骼绕中轴画弧。
- **修法（`FPSCharacterBase.cpp` 构造函数）**：
  1. `HeadCamera` 从 `SetupAttachment(ArmsMesh, "head")` 改为 `SetupAttachment(RootComponent)` + `SetRelativeLocation(0,0,77)`（capsule 固定眼高，`bUsePawnControlRotation=true` 不变）→ 消除 head bob + 弧线下沉。
  2. `bUseControllerRotationPitch = true → false`：身体不再物理俯仰，保持直立（与 BodyRoot 直立的 Shadow/Legs 一致）。
  - **代价（已与用户确认接受）**：手臂/枪暂不随视线上下俯仰；本就计划用 **FEAT-039 上半身 AimOffset**（`AimPitch` 驱动脊柱/手臂绕肩俯仰）补回 → 物理俯仰改为动画俯仰，是正解。
- **顺带参数微调（构造函数默认值，均 BP 可调）**：`BodyRoot` 相对 Location **X=-30**（影子/腿整体往后，相机相对身体靠前）；`PitchMin` -89→**-75**、`PitchMax` 60→**40**；`WalkSpeed` 150→**250**、`SprintSpeed` 450→**550**。
- **⚠️ 编译状态**：session47 这些改动**用户在编辑器 Live Coding 编过手臂相关**（确认手臂不再跟俯仰=改动生效），但**整套未做干净的命令行重编验证**（BodyRoot/速度/俯仰默认值是否全生效待确认）。下次开机建议 Build.bat 重编一次确认无错无警告。

---

## Bug 记录

（暂无）

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（Development Editor/Win64） | 2026-06-24 | ✓ | session43 通过，无错误无警告 |
| 编辑器：mesh 导入（物理拆 Arms/Legs 套原骨架） | 2026-06-25 | 🔄 | session46：Arms 已导入；Legs 待确认；角色 BP 三件套装配待做 |
| PIE——影子完整 + 看到自己腿 + 手臂一致 + 切角色不闪 | — | ⏳ | |

---

## 最终备注 / 调参待办

> - **head bob 调参**：ArmsMesh 跑全身 locomotion 后相机（挂 head 骨骼）会随走路轻晃，必要时压小动画头部位移或给相机加阻尼曲线。
> - **手臂/影子俯仰量不一致**：方案 B 下手臂硬俯仰、影子靠 AimIK，看正下方时影子枪口可能没指那么下——若要修，在 FEAT-040 用身体 AimIK 摆正（优先级低）。
> - Leader/Follower 要求三 mesh 引用**同一个 Skeleton 资产**，导入时务必确认。
> - 几何分离**绝不能** `HideBoneByName`，只能材质段 / OpacityMask。
