# [FEAT-043] 可驾驶机甲（GASP Walker，Mover pawn）

**创建日期：** 2026-06-27
**状态：** planned（低优先，未开工，仅记录方案）
**Archive 文件：** `archive/FEAT-043-mech-walker.md`

---

## 功能概述

把 Epic **Game Animation Sample**（本地：`D:\Unreal Projects\GameAnimationSample`，UE5.7）里的 **Walker 机甲**作为一个独立的"可驾驶单位/载具"加入本游戏。玩家可上机甲操控、下机甲回到角色。

核心利好：机甲是**自包含的 Mover pawn（非 ACharacter）**，移动系统逐 pawn 挂载 → 它与玩家现有的 **CMC + GAS 角色系统并存、互不干扰**，**不需要把项目改造成 Mover**。加机甲是"加法"，不是"替换"。

---

## 设计决策 / 关键事实（来自 session50 对 GameAnimationSample 的勘查）

- 机甲 = `BP_Walker`，建在 **Mover** 插件上（演示 Mover "不绑死 ACharacter/胶囊体"的能力）。
- 腿部"自动踩地"= **Control Rig 程序化 IK**（`CR_Walker` / `CR_Mech`），与移动系统无关，可独立移植复用。
- 样例里机甲单独一套：关卡 `Levels/LocomotorLevel.umap` + GameMode `GM_Locomotor` + Controller `PC_Locomotor` + pawn `BP_Walker`（mesh `SKM_Mech`/`SK_Mech`）。
- Mover 和 CMC **逐 pawn**，同项目并存无冲突——玩家角色继续 CMC，机甲用 Mover。

---

## 范围

**资产迁移（UE Migrate）：** 右键 `BP_Walker` → Asset Actions → Migrate，连依赖复制进 TheManTest：
- `BP_Walker` + Mover 的 `Blueprints/MovementModes/BP_MovementMode_*`
- `SKM_Mech` / `SK_Mech` + 机甲材质
- `ABP_Walker` + `CR_Walker` / `CR_Mech`

**插件：** 启 **Mover + NetworkPrediction**（机甲依赖；装着不用不影响 CMC 角色）。

**接合胶水（主要工作量在此，非重构）：**
1. **上/下机甲**：`ATheManPlayerController` 现为 `AFPSCharacterBase` 配；做触发时 `Possess(BP_Walker)`、下机甲 `Possess` 回角色。
2. **输入**：机甲自带输入 context（`PC_Locomotor` 那套），接进现有输入系统或单独给一套。
3. **相机**：机甲第三人称，本游戏第一人称 → 按需设。
4. **GAS（可选，要机甲有血/技能时）**：架构便利——ASC 挂 `ATheManPlayerState` **跨 pawn 持久**，玩家切到机甲 PlayerState/ASC 不变。机甲 pawn 像角色那样实现 `IAbilitySystemInterface` 指回 PlayerState 的 ASC 即接上。

**依赖 / 前置：**
- 无硬依赖。可在 GASP Motion Matching 主线（玩家角色）稳定后单独开工。
- Mover 为 experimental（5.7）：API 会变、文档社区资源少；但隔离在单个 pawn 内，问题波及不到主角色系统。

**完成标准：**
- [ ] Migrate `BP_Walker` 及依赖进 TheManTest，启 Mover + NetworkPrediction，编辑器无报错
- [ ] 玩家可触发上机甲（Possess `BP_Walker`）+ 下机甲（Possess 回角色），输入/相机正常
- [ ] PIE：机甲移动 + Control Rig 腿部踩地正常；（可选）机甲走 PlayerState ASC 有血量/技能
- [ ] Mover pawn 与现有 CMC 角色系统并存，互不影响

---

## 实现日志

### 2026-06-27-session50 — 功能创建（仅记录方案，未开工）

- 用户在勘查 GameAnimationSample（为 GASP Motion Matching 主线选型）时玩了 `LocomotorLevel` 机甲，觉得酷，提出未来想加进自己游戏。
- 勘查确认机甲是自包含 Mover pawn、与 CMC 逐 pawn 并存可行 → 评估为"中等工作量、高可行性、不动摇主线"，登记为 planned FEAT-043 备查。详见上方设计决策/接合胶水。

---

## 验证证据

| 检查项 | 日期 | 结果 | 备注 |
|---|---|---|---|
| Migrate + 插件启用无报错 | — | ⏳ | |
| 上/下机甲 Possess 切换 | — | ⏳ | |
| PIE 机甲移动 + 腿部 IK | — | ⏳ | |
