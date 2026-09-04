# FEAT-079 — 跨回合世界状态持久化

**创建日期：** 2026-09-03  
**状态：** in_progress（核心框架与预 BeginPlay 跨关卡恢复已验证，待 Blueprint 双实例验收）

## 2026-09-03 恢复时序修正：状态在 Actor BeginPlay 前生效

- 代码审查确认旧 `PostLoadMapWithWorld → 下一 Tick` 恢复发生在 Actor BeginPlay 之后，会让默认状态、AI 或墓碑对象短暂运行；用户确认先修该问题，暂不扩展生成器协议。
- `UPersistentStateComponent` 改为 `OnRegister/OnUnregister` 维护弱引用注册表，并在 `InitializeComponent` 按 GUID 恢复预放置实例。
- `UWorldPersistenceSubsystem` 改用 `OnWorldInitializedActors` 挂接目标 World 的 `OnWorldPreBeginPlay`。该全局阶段负责重建运行时实例；World Partition External Actor 若稍后到达，则由自身组件的 `InitializeComponent` 在所属 Actor BeginPlay 前恢复。
- 运行时 Actor 改为先 `FinishSpawning`，确保 Blueprint/SCS Components 已建立，再查找持久化组件、写回原 GUID 并应用 Transform/业务状态。
- 所有回调均校验 `World->GetGameInstance() == GetGameInstance()`，避免其他 PIE/GameInstance 的 World 误恢复。
- 新增真实 `TestMap → LobbyMap → TestMap` 自动化：预放置 Door 与运行时 Door 均在各自 BeginPlay 中看到已恢复的开启状态和 Transform；墓碑预放置 Door 在第二次加载时不进入 BeginPlay。
- Development Editor / Win64 冷构建成功；`TheManTest.Core.Persistence` 5/5 Success：DoorLifecycle、PlacedDoorAsset、SubsystemPIE、TombstoneBeforeBeginPlay、WorldTravelBeforeBeginPlay。最终日志：`Saved/Logs/FEAT079RestoreTimingComplete.log`。

## 2026-09-03 实施阶段一：核心框架与验收 Door

- FEAT-080 经用户确认完成并归档，FEAT-079 切换为唯一 active feature。
- 新增 `UWorldPersistenceSubsystem`、`UPersistentStateComponent`、`IPersistentActorInterface` 与版本化 `FPersistentActorState/FPersistentMapState`；状态仅保存在本次 GameInstance 生命周期内。
- Component `OnRegister` 注册、`OnUnregister` 注销，`InitializeComponent` 执行实例级预 BeginPlay 恢复；Subsystem 遍历弱引用注册表采集，并拒绝无效/重复 GUID。
- `AcrossRounds` 固定保存 Transform、存在状态与 `FInstancedStruct` 业务数据；支持显式墓碑和运行时 Actor 按 Class/GUID 重建。
- `UTheManGameInstance::HandlePlayerDeath/HandleGameOver` 已在 OpenLevel 前采集；下一张地图通过 World `OnWorldPreBeginPlay` 与组件 `InitializeComponent` 两层机制恢复。
- 新增 `AWorldPersistenceTestDoor`，并通过 Unreal Editor 放置 `PersistenceAcceptanceDoor` 到 TestMap PlayerStart 前方约 500cm；Pawn 进入 Trigger 后开启。
- Development Editor 构建成功；`TheManTest.Core.Persistence` 三项测试全部 Success：DoorLifecycle、PlacedDoorAsset、SubsystemPIE。
- 待用户前台验收真实流程：触发 Door 开启 → 结束回合/返回大厅 → 再进入 TestMap → Door 保持开启。

## 2026-09-03 下次待办：Blueprint 多实例与运行时 Transform 验收

- 用户指出当前 TestMap 只放置一个 C++ Door，且 Trigger 只修改 `DoorMesh` 相对旋转，虽然自动化直接修改并验证了 Actor Transform，但前台没有可操作的 Actor Transform 测试入口。
- 下次先创建 `BP_WorldPersistenceTestDoor`，继承现有 C++ Door；提供实例可配置的运行时位移/旋转测试参数，玩家进入 Trigger 后同时开启门并修改整个 Actor Transform。
- 在 TestMap 放置至少两个 Blueprint 实例并确保 GUID 不同：Door A 使用默认 `AcrossRounds`，下一回合应保留运行时 Transform 和开启状态；Door B 显式设为 `None`，下一回合应恢复关卡默认状态。
- 扩展自动化覆盖多实例 GUID 唯一、两个实例状态隔离、`None` 不进入状态表，以及 `AcrossRounds` Transform/业务状态恢复。
- Blueprint 必须通过 Unreal Editor 创建、编译和保存；更新 TestMap 后冷构建并复跑 `TheManTest.Core.Persistence`，再交给用户前台验收。

## 需求

- 回合结束后，有些门、物体位置、物体存在状态和自定义业务状态需要保留到下一回合。
- 同一个 Actor 类的不同关卡实例必须可以分别选择是否持久化。
- 门、道具、Enemy 和运行时生成对象使用同一套实例级规则；持久化系统不按“是否随机刷新”进行特殊分类，也不接管随机刷新逻辑。

## 已确认的总体方案

- 保留现有流程：`ATheManGameStateBase` 负责本关卡倒计时；死亡或倒计时归零经 `UTheManGameInstance` 保存回合数、进入大厅，选角后重新 `OpenLevel` 测试地图。
- 只有挂载 `UPersistentStateComponent` 的 Actor 才接入本系统；组件策略默认 `AcrossRounds`，实例可显式改为 `None`。`None` 对象不保存，直接依靠地图重载恢复默认状态或由原 Gameplay 逻辑决定是否重新出现。
- 新增 `UWorldPersistenceSubsystem : UGameInstanceSubsystem`，利用与 GameInstance 相同的跨关卡生命周期保存世界状态；现有 `UTheManGameInstance` 继续负责游戏流程与关卡切换。
- 第一阶段仅在本次游戏进程内跨回合保存，关闭游戏后内存状态自然消失；暂不实现 `USaveGame`、读档或存档槽位，但通用状态使用可序列化 `USTRUCT`/`UPROPERTY`、稳定标识和版本号，为后续磁盘存档保留扩展位置。
- 回合结束时统一采集所有启用持久化的存活 Actor，不使用业务状态 `MarkDirty`。地图加载完成后按 GUID 恢复有记录的对象。

## 组件、接口与数据职责

### UPersistentStateComponent

- 只添加到支持持久化的 Actor 类，是否实际启用由实例级策略控制：`None` / `AcrossRounds`，默认 `AcrossRounds`。
- 持有保存进关卡实例的稳定 `FGuid PersistentId`。
- 不提供 `bSaveTransform`：只要策略为 `AcrossRounds`，就固定保存并恢复 Actor Transform、存在状态和自定义业务数据。
- GUID 属于具体 Actor 实例而非类默认身份；同类的多扇门必须各有不同 GUID。
- 预放置 Actor 的 GUID 由本系统生成和维护，作为组件实例属性随关卡序列化；运行时生成 Actor 的 GUID、Class、Transform 和状态保存在 Subsystem 内存记录中，下一回合由 Subsystem 重建并写回同一 GUID。
- `OnRegister` 时主动注册到 Subsystem、`OnUnregister` 时只注销；`InitializeComponent` 在所属 Actor BeginPlay 前应用已有状态。地图卸载不得写入墓碑。

### IPersistentActorInterface

- `CapturePersistentState`：Actor 自己提供需要保存的业务数据。
- `ApplyPersistentState`：Actor 自己通过正式业务入口恢复状态。
- Subsystem 不判断 Actor 是门、箱子还是机关，避免集中式类型分支。

### FPersistentActorState

- `PersistentId`
- `ActorClass` 与运行时/预放置来源信息（供运行时实例重建）
- `Transform`（所有 `AcrossRounds` 实例固定包含）
- `bExists`
- `FInstancedStruct CustomData`
- 状态数据版本号（为后续 `USaveGame` 迁移预留）
- `FInstancedStruct` 保留具体 USTRUCT 类型；例如门使用 `FDoorPersistentData { bOpen, bUnlocked, bBroken }`。
- 正式状态保存在 Subsystem 的 `TMap<MapName, TMap<FGuid, FPersistentActorState>>` 中，不在即将随地图销毁的 Actor/Component 上保留副本。

## 保存与恢复流程

1. `UPersistentStateComponent::OnRegister` 向 Subsystem 注册；Subsystem 以 `TSet<TWeakObjectPtr<UPersistentStateComponent>>` 维护当前有效组件，不在保存时扫描整个 World。
2. 玩家死亡或倒计时归零进入统一的回合结束路径。
3. `OpenLevel` 前，Subsystem 只遍历注册表中策略为 `AcrossRounds` 的存活组件；组件提供 GUID，Subsystem 固定采集 Transform，接口提供自定义数据。
4. `UTheManGameInstance` 继续保存 `CarriedRoundNumber` 并切换到大厅。
5. 玩家选角后重新加载测试地图；预放置 Actor 先恢复为关卡默认状态。
6. `OnWorldPreBeginPlay` 重建缺失的运行时实例；预放置实例在各自 `InitializeComponent` 阶段按 GUID 恢复 Transform、存在状态与接口自定义数据，均早于所属 Actor BeginPlay。
7. 没有持久记录的 Actor 保持关卡默认状态。

## 销毁与刷新规则

- 持久 Actor 若在回合中被拾取或销毁，不能等回合结束再扫描；必须在 `Destroy` 前立即写入 `bExists=false` 墓碑记录。
- `OnUnregister` 只负责从 Subsystem 注册表注销，不能把地图切换造成的统一销毁误记为墓碑；只有明确的 Gameplay 永久销毁入口先写墓碑再调用 `Destroy()`。
- 编辑器预放置对象和运行时生成对象均按实例策略处理，默认 `AcrossRounds`；系统不因道具或 Enemy 来自随机刷新而改变持久化规则。
- 运行时生成且选择持久化的对象额外保存 Actor Class、Transform 与运行时分配的 GUID，下一回合由 Subsystem 重建；原生成逻辑必须避免与恢复实例重复生成，具体接合方式后续单独确认。

## GUID 约束

- GUID 只负责把旧世界的状态记录对应到新地图中的具体 Actor；真正恢复由保存数据和 `ApplyPersistentState` 完成。
- 不使用 Actor 名称、`GetUniqueID()` 或引擎编辑器内部 ActorGuid 作为游戏持久化契约。
- 编辑器创建实例时生成一次并保存；复制 Actor 后必须检测重复并生成新 ID。
- GUID 的值和生命周期由本系统维护；虚幻只负责把已写入组件实例属性的 GUID 随 `.umap` 序列化/反序列化。生成后需标记关卡 Package Dirty，用户保存关卡后才正式写入资产。
- PIE/正式运行发现无效或重复 GUID 时应明确报错，不静默改写身份。

## 暂缓范围

- `USaveGame` 暂不属于第一阶段；后续可将 Subsystem 的状态表写入磁盘并在加载时装回，复用现有采集、GUID、墓碑与恢复流程。
- 当前项目尚无正式 Enemy/道具生成器；生成器与运行时持久实例的唯一生成权协议按用户要求暂不实现，待真正引入动态刷怪/随机道具时单独设计。
- Blueprint 双 Door（AcrossRounds/None）、多实例 GUID 隔离和前台操作整个 Actor Transform 仍是当前下一步。
- 2026-09-03 用户决定在正式业务 Actor 尚未接入前暂缓前台验收；自动化通过的实现保留，功能状态转为 `needs_improvement`，后续从 Blueprint 双实例验收继续。
