# FEAT-057 — Phantom 原始 Rifle_01 小白人动画重建

**状态：** done  
**创建：** 2026-07-30  
**完成：** 2026-07-30  
**依赖：** FEAT-051 原始骨架策略  
**后续：** FEAT-058 至 FEAT-063

## 目标

Phantom 不再使用重定向到 Cyber01 的动画表现。直接采用 `D:\Unreal Projects\TMIIR\Content\Rifle_01` 的原始小白人 Mesh、Skeleton 与兼容动画；TheManTest 只接收最终可用资产，不创建 IK Rig、IK Retargeter 或批量重定向目录。

动画语义必须严格分层：未发现玩家时只用 Relaxed Idle/Walk/Run/Turn/环视；发现玩家并进入战斗后才使用 Aim Idle/locomotion/fire。到达巡逻点时从 `W2_Stand_Relaxed_Fgt_v1_IP` 等多个 Fgt 变体中随机选择。

## 完成标准

- 原始 Mesh/Skeleton/必要动画迁入 Phantom 项目语义目录，依赖完整。
- Phantom 子 AnimBP 绑定原始 Skeleton，公共 `UHumanoidEnemyAnimInstance` 驱动继续有效。
- Patrol/SearchScan 使用 Relaxed 动画；Aim 使用 Aim 动画，状态间无串用。
- 多个 Relaxed Fgt 环视动画可随机播放且不立即重复（资源数量允许时）。
- Blueprint 编译、Development Editor 构建和基础 PIE 状态切换通过。

## Session122 审计

- 用户确认放弃 Phantom 动画重定向路线，要求直接使用 TMIIR `/Game/Rifle_01` 原始小白人及原骨架。
- 文件系统只读盘点确认 Rifle_01 含 Relaxed/Aim 完整动画集，包括 `W2_Stand_Relaxed_Fgt_v1..v4`、Relaxed Walk/Jog/Run、Aim locomotion、Burst、Continuous、Reload 等资源。
- 当前 TheManTest 已有通用 `AHumanoidEnemy` 巡逻、到点 Scan 标记、`UHumanoidEnemyAnimInstance` 随机 `PatrolScanAnimIndex`；`SearchRush/SearchScan` 枚举存在但逻辑尚未实现，后者已拆到 FEAT-058。
- 写入前选择性 WIP checkpoint：`9248c1c`。明确排除用户脏资产 `BP_MaintenanceWorker.uasset` 与未知 TestMap External Actor。

## 待办

- [x] TMIIR 只读命令行审计：`SK_Mannequin` 与所有选定动画均绑定 `UE4_Mannequin_Skeleton`；Fgt v1..v4 时长约 9.0/12.13/10.53/9.9 秒。
- [x] 通过 Unreal AssetTools 迁入最终 Mesh/Skeleton、M4、材质纹理和 Relaxed/Aim/Turn 动画；正式路径统一为 `/Game/Enemy/Phantom/OriginalRifle/`，临时 `/Game/Rifle_01` 已清空。
- [x] 未迁入 IK Rig、IK Retargeter、源 FBX 或重定向工作目录。
- [x] 创建 `BS_Phantom_RelaxedPatrol2D`（20 samples，所有速度层只使用 Relaxed）与 `BS_Phantom_AimLocomotion`（15 samples，只使用 Aim）。类型不兼容的临时 1D BlendSpace 在零引用确认后删除。
- [x] 创建 `ABP_Phantom_OriginalRifle`，父类为无 Skeleton 公共模板，目标为原始 UE4 Mannequin Skeleton；覆盖 Idle、Patrol、Aim、Fgt v1..v4 和左右 45/90/135/180 转身。定向依赖检查确认零 Cyber01/旧 Phantom 动画依赖。
- [x] `BP_Phantom` 切换为原 `SK_Mannequin`、新 AnimBP、原 `M4_Rifle_01`，武器挂 `hand_rSocket_Aim`。
- [x] 原 Skeleton 有 hand_r/hand_l/spine_01..03/标准 IK 骨和 hand weapon sockets；M4 暂无 `Muzzle`/`grip_l`，公共左手 IK/AimIK 会安全禁用，具体 socket 留到 FEAT-060。
- [x] Blueprint 编译保存成功；资产验证通过（OriginalRifle 51 个资产，BP_Phantom 可加载）。
- [x] 临时 PIE：Phantom 可见并落地到 Z=90.15；运行时 Mesh=`SK_Mannequin`、AnimClass/Instance=`ABP_Phantom_OriginalRifle_C`、AIState=Patrol、Speed=0。临时 Actor 已销毁，TestMap 未保存。
- [x] 关闭相关 Unreal Editor/Live Coding 进程后完整执行 `TheManTestEditor Win64 Development` 构建：40/40 actions，`Result: Succeeded`。

## 风险与清理记录

- 第二批转身动画迁移的依赖 Consolidate 曾把已加载的 MaintenanceWorker 动画标脏；这些文件本轮开始前干净，已立即逐一 `git restore`。预存脏资产 `BP_MaintenanceWorker.uasset` 未触碰。
- TestMap 原有未跟踪 External Actor 保持不变；临时 Phantom External Actor 已删除。

## Session123 验收修正（2026-07-31）

- 用户前台检查发现 `ABP_Phantom_OriginalRifle` 实际只保存了转身覆盖，Idle、Patrol、Aim 与 PatrolScan 仍为空；session122 的完成记录与资产实际状态不符。
- 已补齐 Relaxed Idle、`BS_Phantom_RelaxedPatrol2D`、`BS_Phantom_AimLocomotion` 以及 Fgt v1～v4。父模板 `ABP_HumanoidEnemy` 继续保持无骨架/空资产设计。
- Aim 使用二维方向移动 BlendSpace，不使用单一 Aim Idle：当前共 15 个采样，前后左右 Walk/Jog 驱动，斜向由二维插值形成八方向移动，速度为零时回中心 Idle。
- 新增只读自动化 `TheManTest.Enemy.Phantom.AnimationOverrides`，逐项检查关键 Parent Asset Override 非空、四种扫描变体齐全、Aim BlendSpace 至少 9 个采样且覆盖正负方向/移动/中心 Idle。
- 修正后 `TheManTestEditor Win64 Development` 构建成功；`AnimationOverrides` 自动化 Success，日志证据：`PHANTOM_AIM_BLENDSPACE samples=15`。
