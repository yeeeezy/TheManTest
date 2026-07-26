# [FEAT-039] 新主动画蓝图 + 上半身武器 Linked Layer（含 BBBAimIK）

**创建日期：** 2026-06-24
**状态：** planned（依赖 FEAT-038 落地后开工）
**Archive 文件：** `archive/FEAT-039-body-anim-blueprint.md`

---

## 功能概述

为 FEAT-038 的新全身骨架建立正式动画框架：一个**全身主 ABP**（locomotion）+ 一个**上半身武器 Linked Anim Layer**（持枪/瞄准 pose，内含 BBBAimIK 脊柱链）。主 ABP 挂在 ArmsMesh（Leader）上，Shadow/Legs 复制同一份姿势 → 手臂、影子、腿动画完全一致。

沿用现有"基础层 + 武器 Linked Layer"模式（与玩家旧 `ABP_FPSArms` + `ALI_WeaponAnim` + `ABP_FirearmBase`、以及人形怪 ABP 同构），只是迁到新骨架并改成全身。

---

## 设计决策

- **主 ABP 父类沿用 `UFPSCharacterAnimInstance`**（它继承 `UBaseLocomotionAnimInstance`，已输出 Speed / Velocity_Z / bIsFalling / AimPitch / Direction），无需新建 C++ 动画类——优先复用，省一层。如实测需要身体专属变量再加。
- **全身 locomotion**：用 2D blendspace（Speed × Direction）覆盖 8 向走跑 + 跳跃/falling 状态。腿部由它驱动（身体随 `bUseControllerRotationYaw` 面向相机偏航，strafe 靠 Direction 混合）。
- **上半身武器层**：AnimGraph 用 **Layered blend per bone（从 spine 起）** 叠加一个武器 Linked Anim Layer 插槽，复用现有 `ALI_WeaponAnim` 接口。武器层 ABP 父类 `UFirearmAnimInstance`，内含持枪 aim pose + **BBBAimIK** 脊柱链。
- **C++ 链路不动**：装备 `Equip()` → `LinkAnimClassLayers(ArmsMesh 的 AnimInstance)` 现成逻辑（`AEquipmentBase::Equip` / `AFirearm::Equip` 已取 `GetArmsMesh()`）直接复用，切武器即切上半身层。

---

## 范围

**涉及 C++ 文件：**
- 预期**零或极少 C++**。仅当主 ABP 需要身体专属动画变量（现有 `UBaseLocomotionAnimInstance` 不够）时，才在 `UFPSCharacterAnimInstance` 或新子类补 UPROPERTY + `NativeUpdateAnimation`。开工时评估。

**涉及蓝图 / 编辑器（用户操作）：**
- 新建全身主 ABP（父类 `UFPSCharacterAnimInstance`），搭 locomotion 状态机 + 2D blendspace + 跳跃/falling；AnimGraph 输出链插 Layered blend per bone（spine 起）+ 武器 Linked Anim Layer 插槽（`ALI_WeaponAnim`）。
- 新建武器层 ABP（父类 `UFirearmAnimInstance`），持枪 aim pose + BBBAimIK 节点（脊柱链 spine_01→spine_03，骨骼名按新骨架填）。
- ArmsMesh 默认 AnimClass 设为新主 ABP；武器 BP 的 `EquipmentAnimClass`（整体 ABP）/ `EquipmentAnimLayerClass`（Linked Layer）指向新资产。
- 把 FEAT-038 阶段的临时 idle/walk ABP 替换为正式主 ABP。

**依赖：**
- FEAT-038（三件套 mesh + Leader/Follower 架构落地）。
- FEAT-003/004/006 的武器层 + AimIK 经验（`ABP_FirearmBase` WeaponAimOffset 内 AimIK 范式）。
- BBBAimIK 插件（已集成）。

**完成标准：**
- [ ] C++ 编译无错误无警告（若有 C++ 改动）
- [ ] 编辑器：主 ABP + 武器层 ABP 创建并编译通过；ArmsMesh / 武器 BP 引用配好
- [ ] PIE：移动时腿部动画正确（8 向走跑）；上半身持枪 aim pose；切武器时上半身层切换；影子姿势与手臂完全一致

---

## 实现日志

### 2026-06-24-session43 — 功能创建（方案敲定，待 FEAT-038 后开工）

- 与 FEAT-038 同批规划。本条仅记录方案；开工后补实现日志。

### 2026-06-24-session45 — 下半身 locomotion C++ 部分完成（编译通过）

用户提供下半身动画资产清单（`A_N_Walk/Jog_Loop_*` 8~10 向 + `A_N_Walk/Jog_Stop_*` 方向×脚变体 LU/RU/v02 + Lean），要求按 方向+速度+左右脚 播停步，并实现 idle/走/跑/跳。先确认关键事实：`bUseControllerRotationYaw=true` → **strafe 式 locomotion**（身体面向准星，腿按速度方向播 8 向），与 FEAT-038 一致。

**用户拍板的决定：**
- 走/跑：**按住 Shift 提速、松开回走速**（加冲刺键，非切换、非耐力）。
- idle：用户自己找资产，C++ 不管。
- 移动混合：**单个 2D blendspace**（Direction × Speed）。
- 脚检测：**纯 C++** 读 `foot_l`/`foot_r` 骨骼判抬脚（用户回忆敌人 FEAT-029 做过——更正：那是 AnimNotify 手摆驱动 `SetIsLeftFootForward`，FEAT-029 改虚拟减速后删了；这次改纯几何 C++，无需 notify/曲线）。

**C++ 改动（已编译通过 Development/Win64）：**

1. **冲刺输入**
   - `Core/TheManPlayerController.h`：新增 `SprintAction`（`Input|Movement`）+ `GetSprintAction()`。
   - `Characters/FPSCharacterBase/FPSCharacterBase.h/.cpp`：新增 `SprintSpeed`（默认 900，`Movement`，BlueprintReadWrite）+ `StartSprint()`/`StopSprint()`（切 `MaxWalkSpeed`）；`SetupPlayerInputComponent` 绑 `SprintAction` Started→StartSprint / Completed→StopSprint。

2. **`Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h/.cpp`**（原空子类 → 加 locomotion 驱动）
   - override `NativeUpdateAnimation`：基类先算 Speed/Direction/bIsFalling，再补：
     - `bShouldMove`（`GetCurrentAcceleration` 非零 && !bIsFalling）—— Locomotion↔Stop 过渡
     - `bIsJogging`（Speed > `JogSpeedThreshold`，默认 350）
     - `AccelDirection`/`bHasAcceleration`（加速度方向角，起步 Lean 用）
     - `bLeftFootUp`（每帧 `UpdateFootPhase`：读 `foot_l`/`foot_r` Component Space Z，高者=抬脚；`bInvertFootUp` 可翻极性）
     - **停步上升沿冻结**：`bShouldMove` true→false 那帧锁 `LockedStopDirection`(角度) / `StopDirectionIndex`(8 向 0~7) / `bStopFromRun`(bIsJogging)；`bLeftFootUp` 此刻即冻结值。理由：松键后 velocity 衰减 Direction 变噪，必须边沿锁定。
   - 配置（EditDefaultsOnly）：`JogSpeedThreshold` / `LeftFootBone`(foot_l) / `RightFootBone`(foot_r) / `bInvertFootUp`。
   - `DirectionToStopIndex`：标准 8 向桶（0=F/1=RF45/2=R90/3=RB135/4=B180/5=LB135/6=L90/7=LF45）。**注意**：用户停步资产有 10 个方向（含 RB-090/LB-090），若全用需在 ABP 读 `LockedStopDirection` 浮点角度自分桶；只用 8 向用 `StopDirectionIndex`。**待用户定（8 向 vs 10 向分桶）。**

**编辑器待办（用户操作，见 progress.md 交接）：** IA_Sprint + 角色速度配置 + BS_Locomotion(2D) + 主 ABP 状态机（Idle/Locomotion/Stop/Jump）+ Stop 选择逻辑（用上面冻结变量）。idle 资产用户自找。上半身武器层 ABP（含 BBBAimIK）属本功能后半段，下半身验证后做。

**ABP 结构决定（用户 session45 定）：仿敌人模板模式。** 建一个 **Template ABP `ABP_BodyLocomotion`**（不绑骨架，父类 `UFPSCharacterAnimInstance`），locomotion 状态机/连线只搭一次；每角色建子 ABP（`ABP_Body_<角色>`）继承模板、绑各自骨架、用 **Asset Override** 换 blendspace/停步动画。与 `ABP_BaseLocomotion`→`ABP_HumanoidEnemy` 同构。注意：**不复用 `ABP_BaseLocomotion`**——它父类是 `UBaseLocomotionAnimInstance`（看不到 bShouldMove/bLeftFootUp/StopDirectionIndex 等新变量），且状态机是旧 1D 手臂 locomotion，无方向/停步可复用。blendspace 已建 `BS_IdleWalkRun_MaintenanceWorker`（MW 专属，子 ABP 里 override）。

### 2026-06-25-session47 — 全身主 ABP 跳跃状态机完成（编辑器）

`ABP_BodyLocomotion`（Template，父类 `UFPSCharacterAnimInstance`）的 locomotion + 跳跃状态机搭好并编译通过。

- **搭法**：用户直接**复制现成 ABP_BodyLocomotion 的状态机节点**（连混合空间一起复用），无需手搭。LocomotionSM 结构：`Idle ↔ Run/Walk(BS Direction×Speed)` + `Jump_Start/Jump_Loop/Jump_End` + State Alias `To Falling`（Idle/Run-Walk 起跳）/ `To Land`（落地）。过渡用 `bIsFalling`/`Velocity_Z`/`Speed` + 自动规则（剩余时间）。详见 arch/12「ABP_BodyLocomotion」节。
- **粘贴后清掉的报错**：别名缺失状态引用、几条 transition 条件未连、`Velocity_Z > ?` 空引脚等——用户已修完，零编译错误。
- **模板不填资产**：Jump_Start/Loop/End 的 Sequence Player、Run/Walk 的 Blend Space 在模板里留空，子 ABP `ABP_Body_<角色>` 用 Asset Override 填。
- **根运动**：ABP 设 `Root Motion from Montages Only` → 跳跃垂直交给 CharacterMovement 物理，跳跃动画原地播（不提取根运动）。

**BUG-039-002（跳进地里）已修** → 见下方 Bug 记录。

**当前进度**：跳跃 Start→Fall→Land 链路通。**仍未做**：Stop 状态（停步选脚/选向，用 C++ 冻结的 StopDirectionIndex/bLeftFootUp/bStopFromRun）；上半身武器层 ABP（含 BBBAimIK + AimPitch 驱动俯仰）；子 ABP 挂 ArmsMesh + PIE 整体验证（8 向走跑 / 影子腿一致）。

### 2026-06-26-session48 — Stop 状态 + 根运动方案 + 手臂宿主合并进 GetMesh（重大架构）

本会话围绕「停步要根运动滑步」展开，连带改了架构、定了根运动方案，踩了几个坑。

**A. 手臂宿主合并进 `GetMesh()`（架构变更，C++）**
- 起因：想让停步动画的根运动推动胶囊体滑步减速，但发现**根运动只从 `GetMesh()` 提取**，而动画宿主是独立的 `ArmsMesh` 组件 → 改任何根运动设置都没用。
- 改法：删除独立 `ArmsMesh` 组件，把手臂配置（OnlyOwnerSee/不投影/无碰撞/AlwaysTickPose）搬到 `GetMesh()`；`GetArmsMesh()` 改返回 `GetMesh()` → 装备/武器层/蒙太奇/摇摆/Follower 等所有调用点**零改动**。
- 安全前提：session47 相机已从 head 骨骼挪到 capsule → 手臂无人依赖才能合并。
- 涉及文件：`FPSCharacterBase.h/.cpp`（删组件、getter 改向、BeginPlay/Reveal/Tick 的 ArmsMesh→GetMesh）。BP 侧：Mesh 组件配骨架+AnimClass+Transform，**Cast Shadow 取消勾选**（BP 勾选覆盖 C++ 的 false → 手臂影子和 ShadowBodyMesh 穿帮）。

**B. `StopAnimIndex` 合并索引（C++）**
- `FPSCharacterAnimInstance` 新增 `StopAnimIndex`，停步边沿一次性算出并冻结：跑停 `0~7`=方向，走停 `8 + 方向×2 + (左脚抬?0:1)`。ABP 一个 `Blend Poses by int` 读它直选 24 个停步动画，不用层层 bool 嵌套。脚也随之冻结，修了原 `bLeftFootUp` 停步后还在变、走停 LU/RU 中途翻的隐患。

**C. Stop 状态搭建（编辑器）**
- 单 `Blend Poses by int`(ActiveChildIndex=`StopAnimIndex`)连 24 动画。资产名映射见下「停步资产对照」。
- 跑停 8 个 Sequence Player + 循环 blendspace 设 `Sync Group Name=Locomotion`，靠同名 FootSyncMarker 自动对脚（跑停是「一长段含左右脚」的单 clip，marker 同步进对应脚段）；走停 16 个是脚专属独立文件直选。全部取消 Loop。
- 过渡：`Run/Walk→Stop`=`bShouldMove==false`（falling 优先）；`Stop→Idle`=自动规则/Speed<3；`Stop→Locomotion`=`bShouldMove==true`（即时打断）。

**D. 根运动方案定稿（认知反复，记牢免再踩）**
- ABP 用 **`Root Motion from Everything`** + flag 铁律：**原地 clip（idle/走跑循环/跳跃）Enable Root Motion 关、位移 clip（停步等）开**。
- 关键认知：`Everything` 模式下 per-clip Enable Root Motion **起作用**（中途一度误以为它绕过 flag，被实测纠正）。任何「勾着但原地」的 clip → 贡献 0 位移根运动但 `bHasRootMotion=true` → CMC 接管移动压制输入 → 卡死+抖。
- 根运动停步**可被 `Stop→Locomotion (bShouldMove==true)` 过渡打断**，不是不可打断。
- 备选未采用：停步蒙太奇（Montages Only）/ Distance Matching（AAA 滑停，动画跟随物理、不碰根运动，留作后续打磨参考）。

**E. `bStopFromRun` 改冲刺键判断（B 方案，C++）**
- 走/跑停步档由 `AFPSCharacterBase::IsSprinting()`（按住 Shift）决定，不再用 `Speed > JogSpeedThreshold`——松键减速那刻 Speed 抖动会把走停误判成跑停。新增 `bIsSprinting` 标志（StartSprint/StopSprint 维护）+ `IsSprinting()` getter。`bIsJogging` 保留仅供 blendspace 走/跑混合。
- 边界：先松 Shift 再松移动键时会播走停（哪怕速度还高）——已知取舍，可接受。

**当前进度（session48）**：走/跑/停/跳整套在 `Everything`+flag 规则下跑通（移动跟手、停步根运动滑步、可打断）。**仍未做**：上半身武器层 ABP（BBBAimIK + AimPitch）；各角色子 ABP 全配 + 三件套验证；B 方案重编后 PIE 复验（用户待测）；arch 文档已同步。

### 2026-06-26-session49 — 停步手感打磨：gait 回退速度判断 + No Sync 修瞬切 + 延迟停(C++) + 定调 Distance Matching

全程打磨停步手感。C++ 改 `FPSCharacterAnimInstance.h/.cpp`（**动头文件，需全量重编**）。

- **gait 判断回退「速度」（A 方案）**：停步边沿 `bStopFromRun = bIsJogging`（`Speed > JogSpeedThreshold`，默认350）。删 session48 的冲刺宽限（`SprintReleaseGraceTime`/`TimeSinceSprinting`）。`AFPSCharacterBase::IsSprinting()`/`bIsSprinting` 保留供冲刺提速（动画不再依赖）。理由：边沿那刻 velocity 未衰减、单帧采样可靠（调试实测 Speed=550）。
- **瞬切修复（No Sync）**：跑停 SequencePlayer 从 `SyncGroup=Locomotion` 改 `No Sync`，解决 marker seek 把起播点顶到中后段、令 time-remaining 类过渡秒切。详见 BUG-039-005。
- **延迟停（C++ 已写、ABP 待接）**：松键 → `bStopRequested` + 冻结 gait（`bStopFromRunPending`）→ 保持 Locomotion 靠 CMC 刹车惯性滑 → **下一次落脚**（`bLeftFootUp` 翻转）或兜底（`Speed<StopCommitFallbackSpeed`30 / `StopRequestTimer>MaxStopDelayTime`0.35s）→ commit `bShouldStop=true`，此刻才冻结 `StopDirectionIndex`/`StopAnimIndex`。新增 `bShouldStop`(BlueprintReadOnly) + 上述配置/状态。**ABP `Run/Walk→Stop` 过渡需从 `bShouldMove==false` 改读 `bShouldStop` 才生效**（未改不会坏，退化即时停）。目的：消除"松键当场杵住"的突兀，靠惯性多走半步从满弧线起停。
- **定调根本解 = Distance Matching**：用户悟到真问题是**胶囊体物理减速节奏 ≠ 停步动画内置减速曲线** → 滑步/速度突变（非脚步衔接）。机制：动画播放头按"剩余刹停距离"驱动（非时间），把"动画根走的距离"锁死=「胶囊体走的距离」→ 美术烘进 clip 的「脚=根+偏移」踩地关系全程不破 → 零滑、任意进入速度自适应。**不换动画**（用现有 retarget `RTG_..._Stop_*`；换 Epic Game Animation Sample 动画不划算：自定义骨架要重 retarget + 曲线重烘 + 那套是 Motion Matching）。落地：启 **Animation Locomotion Library** 插件（内置）+ 拷 Game Animation Sample 的 `DistanceCurveModifier` 烘 `Distance` 曲线（一键）+ C++ 出 `DistanceToStop`（CMC 刹车前向模拟）+ ABP Distance Matching 节点。**`DistanceToStop` C++ 被用户打断尚未加，下次补。**
- **TEMP 调试**：`FPSCharacterAnimInstance.cpp` 的 `id=901` + `#include "Engine/Engine.h"` 仍在（查停步问题用，已定位），待删。

### 2026-06-27-session50 — 停步方案再修正：松键「保持原速滑行」（替代延迟期减速）

用户回去复盘 session49，定位到更准的根因并提出方案，**暂缓 Animation Locomotion Library / Distance Matching**，先试这个更轻的解法。

- **再定位根因**：session49 的「延迟停」在松键后的等待窗口里，胶囊体仍**靠 CMC 刹车在减速**（默认 `BrakingDecelerationWalking=2048` + `GroundFriction=8`，摩擦力 ∝ 速度）。所以：等待期物理已掉速（blendspace 按 Speed 采样 → 腿视觉减速）→ 进停步 clip（按满速烘的根运动减速曲线）→ 胶囊速度被根运动**顶回满速** → "突然减速后停步动画往前一冲"。
- **用户方案**：松键后**不减速、保持原速匀速滑行**，到下一个合适时机（落脚）才进停步 clip，由停步动画的**根运动统一做减速** → 物理减速节奏 = 动画内置减速曲线，速度全程连续。
- **C++ 实现（动了头文件，需全量重编）**：
  - `FPSCharacterAnimInstance.h`：加 `IsStopCommitted()`（返回 `bShouldStop`）。
  - `FPSCharacterBase.h/.cpp`：
    - 新增 `bHasMoveInput`，由 `Move`（Triggered 置 true）+ 新增 `OnMoveReleased`（Move 的 `Completed`/`Canceled` 置 false）维护——**无 1 帧延迟**，避免松键当帧默认摩擦先掉 ~100cm/s（550 速度下摩擦 8×550×dt 单帧就很大，那一帧本身就是要消的突兀）。故意不用有延迟的 `GetCurrentAcceleration()`。
    - BeginPlay 缓存 `DefaultGroundFriction`/`DefaultBrakingDecelerationWalking`。
    - Tick：`bCoasting = !bHasMoveInput && !IsFalling && !IsStopCommitted && Velocity>10`。滑行时 `BrakingDecelerationWalking=0`+`GroundFriction=0`（匀速），否则恢复默认。
  - 机制串联：松键→匀速滑行（ABP 留 Locomotion，腿满速循环不掉速）→ 动画落脚 commit `bShouldStop`→ABP 进 Stop（根运动减速）→角色 `IsStopCommitted` true→恢复刹车（此后根运动主导速度，刹车参数无所谓）。
- **⚠️ 必须的 ABP 编辑器改动（否则本方案不生效）**：ABP `Run/Walk→Stop` 过渡条件**必须**从 `bShouldMove==false` 改读 `bShouldStop`。若仍读 `bShouldMove==false`，松键即刻进 Stop、根运动立即接管，滑行被无视 → C++ 改动等于白做。（此项 session49 已列待办，本方案把它从"可选优化"升级为"必需"。）
- **调参提示**：滑行距离 = 满速 × (到落脚的时长)。`MaxStopDelayTime=0.35s` 是兜底上限——550 速度下最坏滑 ~192cm；落脚通常更早触发。嫌滑太远调小 `MaxStopDelayTime`；`StopCommitFallbackSpeed`(30) 在零摩擦下基本不触发（速度不掉）可忽略。
- **决策**：若此方案手感达标，**延迟停 + 保持原速这套就够，不必再上 Distance Matching**；若进入速度差异大仍有轻微不匹配，再考虑 Distance Matching 作为根本解。
- **TEMP 调试** `id=901` + `#include "Engine/Engine.h"`（FPSCharacterAnimInstance.cpp）仍在，停步定稿后删。

---

## Bug 记录

### BUG-039-005 停步动画播对了却被瞬间打断（已修，session49）
- **现象**：松键进 Stop，停步 clip 选得对（StopAnimIndex 正确）但刚播就被切走（多半切到 Idle）。
- **根因**：跑停 8 个 SequencePlayer 设 `SyncGroup=Locomotion` 当 follower。进 Stop 时 marker-based sync 把它 **seek 到跑步循环当前相位（中后段）** → `GetRelevantAnimTimeRemainingFraction` 瞬间很小 → `Stop→Idle` 的 time-remaining 类过渡（手动 `timeRemaining<0.3` 或 auto rule）立刻成立 → 秒切。
- **修复**：跑停 SequencePlayer 改 `Method=No Sync`（从 0 播，剩余时间正常递减）。用户验证 OK。
- **教训/认知**：
  - **auto rule 过渡不是"播完才切"**：它=「剩余时间 ≤ 本过渡 blend 时长」就切，与手动 time-remaining 同一套机制 → seek 一样坑它。
  - 任何"从中段起播"的方案（SyncGroup follower seek / 手动 StartPosition / Pose Matching）都会**缩短剩余时间** → 配 time-remaining 类过渡要留意会提前切。
  - 进停步"对脚"的正路不是 SyncGroup，而是 **Pose Matching**（`bStartFromMatchingPose`，需 PoseSearch 插件 + Pose History 节点）或 **Distance Matching**。

### BUG-039-004 Stop 永远播「跑停-前」不分方向走跑（已修，session48）
- **现象**：任何方向、任何速度松键，都播 index 0 的跑停-前。
- **根因**：ABP 的 `Blend Poses by int` 的 ActiveChildIndex **漏连 `StopAnimIndex`** → 恒为默认 0 → 永远选 index 0（=跑停-前）。
- **修法**：把 ActiveChildIndex 连到 `StopAnimIndex`。
- **教训**：两个症状（永远跑停 + 永远朝前）合起来＝「索引恒为 0」，优先怀疑 index 变量没接/没更新，用 Anim BP 调试器看 `StopAnimIndex`/`bShouldMove` 实时值即可定位。

### BUG-039-003 Everything 模式下移动卡死+抖动（已修，session48）
- **现象**：`Root Motion from Everything` 下，一移动就卡在原地高频抖动、推不动。
- **排查反复**：先以为「所有 clip 都变静止根运动抢输入」，又改口「per-clip flag 不 gate」，最终被用户实测纠正。
- **真根因**：**idle 的 Enable Root Motion 忘了关**。Everything 模式**尊重** per-clip flag，但 idle 是原地 clip 却勾着 → 贡献 0 位移根运动、`bHasRootMotion=true` → CMC 接管移动压制 WASD → 卡+抖。
- **修法**：原地 clip（idle/走跑循环/跳跃）一律关 Enable Root Motion，只位移 clip（停步）开。
- **教训**：批量关 Loop 的 Enable Root Motion 时**别漏 idle**；卡死先查所有原地 clip 的 flag。

### BUG-039-002 跳跃塌进地里（已修，session47）
- **现象**：跳跃时角色陷进地里。
- **排查**：先疑根运动提取——但 ABP 已是 `Root Motion from Montages Only`（跳跃不提取根运动），排除；又疑跳跃动画 root 骨骼位移 / Force Root Lock，也不是。
- **根因（用户自查出）**：**Land 动画被标成了 Additive（Local Space）**。加性动画是"差值"，状态机里当普通状态直接播时，那段差值把姿势整体拉偏 → 看起来塌进地里。
- **修法**：Land 动画资产改 **No Additive**（普通动画）。修复确认。
- **教训**：状态机里直接播的 clip 必须 No Additive；只有要叠加到别的姿势上（上半身瞄准/受击抖动叠在 locomotion 上）才用 Additive。

### BUG-039-001 新全身 mesh 上 ArmsMesh 进游戏歪 90°（已修，session45）
- **现象**：把新全身 mesh 放上 ArmsMesh 后，进游戏身体朝侧面歪 ~90°。
- **根因**：① 新骨架参考姿势朝 +Y（UE 标准人形），`ArmsMesh` 以 identity 挂 capsule（不像 ACharacter `GetMesh()` 自带 Yaw -90），未转正；② 武器摇摆（FEAT-008）`ArmsMesh->SetRelativeRotation(CurrentSway)` 每帧把相对旋转覆盖回 ~identity，蓝图里填的 Yaw base 被冲掉。
- **修法**（`FPSCharacterBase.h/.cpp`）：新增 `BaseArmsRotation`（EditAnywhere/BlueprintReadWrite，默认 `FRotator(0,-90,0)`）；Tick 改 `ArmsMesh->SetRelativeRotation(BaseArmsRotation + CurrentSway)`（叠加不覆盖）。各角色 mesh 朝向不同时在 BP 调 `Base Arms Rotation`。用户确认转正 ✓。
- 注：属 FEAT-038（ArmsMesh 朝向）集成问题，FEAT-039 session45 顺手修。

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| C++ 编译（如有改动） | — | ⏳ | |
| 编辑器：主 ABP + 武器层 ABP 编译通过 | — | ⏳ | |
| PIE——腿部 locomotion + 上半身持枪 + 切武器层 + 影子一致 | — | ⏳ | |

---

## 最终备注

> - Leader 是 ArmsMesh，所以"全身动画"实际在 ArmsMesh 上评估；Shadow/Legs 只复制。蒙太奇（开火/拔枪）也只需在 ArmsMesh 的 AnimInstance 播放，Follower 自动跟随。
> - 武器层切换走现有 `LinkAnimClassLayers`/`UnlinkAnimClassLayers`，无需为新框架重写 C++。
