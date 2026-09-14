# 跨回合世界状态持久化

**何时读取：** 修改跨回合 Actor 状态、关卡切换前采集、地图加载后恢复、持久 GUID、墓碑或测试 Door 时。

## 核心文件

| 文件 | 职责 |
|---|---|
| `Public/Core/Persistence/WorldPersistenceTypes.h` | `EPersistencePolicy`、Actor/地图快照与版本字段 |
| `Public/Core/Persistence/PersistentStateComponent.h` / `Private/...cpp` | 实例策略、GUID、OnRegister/OnUnregister 注册表生命周期、InitializeComponent 预 BeginPlay 恢复、显式墓碑入口 |
| `Public/Core/Persistence/PersistentActorInterface.h` / `Private/...cpp` | Actor 自定义 `FInstancedStruct` 采集与应用接口 |
| `Public/Core/Persistence/WorldPersistenceSubsystem.h` / `Private/...cpp` | GameInstance 生命周期内按地图与 GUID 保存、恢复和运行时实例重建 |
| `Public/Actors/Persistence/WorldPersistenceTestDoor.h` / `Private/...cpp` | FEAT-079 验收 Door；Pawn 进入 Trigger 后开启 |
| `Private/Core/Tests/WorldPersistenceTests.cpp` | Door生命周期、注册表、Transform、运行时重建与墓碑自动化 |

## 规则

- 只有挂载 `UPersistentStateComponent` 的 Actor 接入系统；默认 `AcrossRounds`，实例可设为 `None`。
- `AcrossRounds` 固定保存 Transform、存在状态和接口自定义数据，不存在独立 Transform 开关。
- Component 在 `OnRegister` 注册到 Subsystem、`OnUnregister` 注销；`InitializeComponent` 按自身 GUID 恢复预放置实例，确保 World Partition External Actor 即使晚于全局初始化到达，也在自己的 BeginPlay 前获得正式状态。地图卸载不得写墓碑。
- Gameplay 永久销毁必须先调用 `MarkPersistentlyDestroyed()`，再调用 `Destroy()`。
- 预放置实例的自定义 GUID 随关卡实例序列化；运行时实例由 Subsystem 保存 Class/GUID 并重建。
- 状态只在本次游戏进程内保存在 `UWorldPersistenceSubsystem`；结构已使用 `SaveGame` 属性和版本字段，为后续 `USaveGame` 留扩展点。

## 流程

`UTheManGameInstance::HandlePlayerDeath/HandleGameOver` 在 `OpenLevel` 前调用 `CaptureWorldState`。Subsystem 以去 PIE 前缀的地图 Package 名为第一层键、`FGuid` 为第二层键。

恢复采用两层时序：

1. `OnWorldInitializedActors` 只负责给所属 GameInstance 的目标 World 挂接一次 `OnWorldPreBeginPlay`；`OnWorldPreBeginPlay` 在任何 Actor BeginPlay 前重建保存的运行时实例。
2. World Partition 的 External Actor 可能在上述全局初始化事件之后才注册，因此每个 `UPersistentStateComponent` 还会在自己的 `InitializeComponent` 中按 GUID 恢复 Transform、业务数据或处理墓碑。该阶段仍早于所属 Actor 的 BeginPlay。

运行时 Blueprint Actor 先 `FinishSpawning` 以创建 SCS/Blueprint Components，再查找 `UPersistentStateComponent`、写回原 GUID 并应用状态；因为正式恢复发生在 World BeginPlay 前，所以不会提前启动 Gameplay。

## 当前验收对象

用户于2026-09-13要求清理TestMap，PersistenceAcceptanceDoor预放置实例已删除。依赖它的PlacedDoorAsset、真实跨关卡摆件恢复和墓碑专项测试已撤下；核心持久化实现与LobbyMap流程保持。

TheManTest.Core.Persistence 当前保留 DoorLifecycle／SubsystemPIE 两项，运行时自行生成并清理对象，不依赖或保存地图摆件。此前5项的历史验证见FEAT-079归档，不能再作为当前注册清单。
