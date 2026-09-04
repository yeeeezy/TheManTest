# 当前进度

## Active Feature

- `FEAT-079`：跨回合世界状态持久化
- 状态：`in_progress`
- 当前阶段：核心框架与预 BeginPlay 跨关卡恢复已验证；待 Blueprint 双实例和前台验收。
- 详细历史：`archive/FEAT-079-round-world-persistence.md`

## 已完成

- `UWorldPersistenceSubsystem`、`UPersistentStateComponent`、`IPersistentActorInterface` 与版本化状态结构已实现。
- `UTheManGameInstance::HandlePlayerDeath/HandleGameOver` 在 OpenLevel 前采集当前地图状态。
- AcrossRounds 实例固定保存 Transform、存在状态和 `FInstancedStruct` 自定义数据；支持运行时 Actor 重建和显式墓碑。
- `UPersistentStateComponent` 使用 `OnRegister/OnUnregister` 维护弱引用注册表，并在 `InitializeComponent` 恢复 World Partition 预放置实例。
- `UWorldPersistenceSubsystem` 通过 `OnWorldInitializedActors` 挂接 `OnWorldPreBeginPlay`，在 Actor BeginPlay 前重建运行时实例；回调过滤所属 GameInstance。
- 运行时 Actor 先 `FinishSpawning` 创建 Blueprint/SCS Components，再写回保存 GUID 并应用状态。
- TestMap 已放置一个 C++ `PersistenceAcceptanceDoor`。

## 验证

- Development Editor / Win64 冷构建成功；因另一个 VFXPack Editor 开启 Live Coding，使用 UBT 官方 `-NoHotReloadFromIDE` 绕过共享 UnrealEditor 可执行文件的全局检测，未关闭或修改外部项目。
- `TheManTest.Core.Persistence` 5/5 Success：
  - `DoorLifecycle`
  - `PlacedDoorAsset`
  - `SubsystemPIE`
  - `TombstoneBeforeBeginPlay`
  - `WorldTravelBeforeBeginPlay`
- 真实 `TestMap → LobbyMap → TestMap` 已验证：预放置 Door 与运行时 Door 均在自己的 BeginPlay 中看到恢复后的开启状态和 Transform；墓碑 Door 在第二次加载时未进入 BeginPlay。
- 最终日志：`Saved/Logs/FEAT079RestoreTimingComplete.log`

## 当前待办

- 创建 `BP_WorldPersistenceTestDoor`，提供 Trigger 后修改整个 Actor Transform 的前台测试入口。
- TestMap 放置两个不同 GUID 的 Blueprint 实例：Door A=`AcrossRounds`，Door B=`None`。
- 扩展多实例隔离、GUID 唯一和 None 不入状态表自动化；编译保存 Blueprint 后再做前台 PIE 验收。
- 当前项目没有正式 Enemy/道具生成器；生成器唯一生成权协议按用户要求暂不考虑。

## 会话交接

- 本轮已修复旧 `PostLoadMapWithWorld → 下一 Tick` 导致状态晚于 BeginPlay 的问题。
- 直接在 `OnWorldInitializedActors` 恢复曾被新增真实测试否定：World Partition External Actor 的组件在该事件之后才注册。最终使用“World `OnWorldPreBeginPlay` 重建运行时实例 + Component `InitializeComponent` 恢复预放置实例”的两层方案，5 项自动化全部通过。
- 写入前安全检查点：`4ea5163`（`WIP checkpoint before fixing FEAT-079 restore timing`）。本轮修正尚未提交，等待用户明确要求更新 Git。
- 下一步仍需按 harness 先列 Blueprint 双实例实施方案并等待用户确认，不得直接修改资产。
