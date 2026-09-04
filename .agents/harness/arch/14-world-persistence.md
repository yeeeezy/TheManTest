# 跨回合世界状态持久化

**何时读取：** 修改跨回合 Actor 状态、关卡切换前采集、地图加载后恢复、持久 GUID、墓碑或测试 Door 时。

## 核心文件

| 文件 | 职责 |
|---|---|
| `Public/Core/Persistence/WorldPersistenceTypes.h` | `EPersistencePolicy`、Actor/地图快照与版本字段 |
| `Public/Core/Persistence/PersistentStateComponent.h` / `Private/...cpp` | 实例策略、GUID、BeginPlay 注册、EndPlay 注销、显式墓碑入口 |
| `Public/Core/Persistence/PersistentActorInterface.h` / `Private/...cpp` | Actor 自定义 `FInstancedStruct` 采集与应用接口 |
| `Public/Core/Persistence/WorldPersistenceSubsystem.h` / `Private/...cpp` | GameInstance 生命周期内按地图与 GUID 保存、恢复和运行时实例重建 |
| `Public/Actors/Persistence/WorldPersistenceTestDoor.h` / `Private/...cpp` | FEAT-079 验收 Door；Pawn 进入 Trigger 后开启 |
| `Private/Core/Tests/WorldPersistenceTests.cpp` | Door、地图资产、注册表、Transform、运行时重建与墓碑自动化 |

## 规则

- 只有挂载 `UPersistentStateComponent` 的 Actor 接入系统；默认 `AcrossRounds`，实例可设为 `None`。
- `AcrossRounds` 固定保存 Transform、存在状态和接口自定义数据，不存在独立 Transform 开关。
- Component 在 `BeginPlay` 注册到 Subsystem，`EndPlay` 只注销；地图卸载不得写墓碑。
- Gameplay 永久销毁必须先调用 `MarkPersistentlyDestroyed()`，再调用 `Destroy()`。
- 预放置实例的自定义 GUID 随关卡实例序列化；运行时实例由 Subsystem 保存 Class/GUID 并重建。
- 状态只在本次游戏进程内保存在 `UWorldPersistenceSubsystem`；结构已使用 `SaveGame` 属性和版本字段，为后续 `USaveGame` 留扩展点。

## 流程

`UTheManGameInstance::HandlePlayerDeath/HandleGameOver` 在 `OpenLevel` 前调用 `CaptureWorldState`。Subsystem 以去 PIE 前缀的地图 Package 名为第一层键、`FGuid` 为第二层键。`PostLoadMapWithWorld` 后延迟一帧恢复：先匹配预放置组件，再按保存 Class 重建缺失的运行时实例，最后恢复 Transform 与业务数据。

## 当前验收对象

`TestMap` 的 `PersistenceAcceptanceDoor` 位于 PlayerStart 前方约 500cm。进入 Trigger 后门板旋转到 90°；回合结束进入大厅并再次进入 TestMap 后，应保持开启和原 Transform。
