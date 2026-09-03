# FEAT-079 — 跨回合世界状态持久化

**创建日期：** 2026-09-03  
**状态：** planned（仅设计归档，未实施）

## 需求

- 回合结束后，有些门、物体位置、物体存在状态和自定义业务状态需要保留到下一回合。
- 另一些对象（随机刷新物、普通可移动物、敌人和临时掉落物）应在回合结束后恢复或消失。
- 同一个 Actor 类的不同关卡实例必须可以分别选择是否持久化。

## 已确认的总体方案

- 保留现有流程：`ATheManGameStateBase` 负责本关卡倒计时；死亡或倒计时归零经 `UTheManGameInstance` 保存回合数、进入大厅，选角后重新 `OpenLevel` 测试地图。
- 非持久对象不保存，直接依靠地图重载恢复编辑器默认状态或清除运行时对象。
- 新增 `UWorldPersistenceSubsystem : UGameInstanceSubsystem`，利用与 GameInstance 相同的跨关卡生命周期保存世界状态；现有 `UTheManGameInstance` 继续负责游戏流程与关卡切换。
- 回合结束时统一采集所有启用持久化的存活 Actor，不使用 `MarkDirty`。地图加载完成后按 GUID 恢复有记录的对象。

## 组件、接口与数据职责

### UPersistentStateComponent

- 只添加到支持持久化的 Actor 类，是否实际启用由实例级策略控制：`None` / `AcrossRounds`。
- 持有保存进关卡实例的稳定 `FGuid PersistentId`。
- 保存通用配置，例如是否保存 Transform。
- GUID 属于具体 Actor 实例而非类默认身份；同类的多扇门必须各有不同 GUID。

### IPersistentActorInterface

- `CapturePersistentState`：Actor 自己提供需要保存的业务数据。
- `ApplyPersistentState`：Actor 自己通过正式业务入口恢复状态。
- Subsystem 不判断 Actor 是门、箱子还是机关，避免集中式类型分支。

### FPersistentActorState

- `PersistentId`
- 可选 `Transform`
- `bExists`
- `FInstancedStruct CustomData`
- `FInstancedStruct` 保留具体 USTRUCT 类型；例如门使用 `FDoorPersistentData { bOpen, bUnlocked, bBroken }`。
- 正式状态保存在 Subsystem 的 `TMap<MapName, TMap<FGuid, FPersistentActorState>>` 中，不在即将随地图销毁的 Actor/Component 上保留副本。

## 保存与恢复流程

1. 玩家死亡或倒计时归零进入统一的回合结束路径。
2. `OpenLevel` 前，Subsystem 收集所有策略为 `AcrossRounds` 的存活 Actor，组件提供 GUID，接口提供自定义数据。
3. `UTheManGameInstance` 继续保存 `CarriedRoundNumber` 并切换到大厅。
4. 玩家选角后重新加载测试地图；预放置 Actor 先恢复为关卡默认状态。
5. 地图 Actor 初始化完成后，Subsystem 建立 `GUID → Actor` 索引，再恢复 Transform、存在状态与接口自定义数据。
6. 没有持久记录的 Actor 保持关卡默认状态。

## 销毁与刷新规则

- 持久 Actor 若在回合中被拾取或销毁，不能等回合结束再扫描；必须在 `Destroy` 前立即写入 `bExists=false` 墓碑记录。
- 编辑器固定摆放的物品可由物品实例持有 GUID。
- 固定点动态刷新的内容，由固定 SpawnPoint 持有 GUID，并保存“生成了什么/是否已领取”等状态。
- 每回合随机地图刷新点和随机物品使用 `None`，不参与持久化，下一回合重新随机。
- 玩家运行时创建且需要跨回合保留的对象，后续实现时需额外保存 Actor Class、Transform 与分配后的持久 GUID。

## GUID 约束

- GUID 只负责把旧世界的状态记录对应到新地图中的具体 Actor；真正恢复由保存数据和 `ApplyPersistentState` 完成。
- 不使用 Actor 名称、`GetUniqueID()` 或引擎编辑器内部 ActorGuid 作为游戏持久化契约。
- 编辑器创建实例时生成一次并保存；复制 Actor 后必须检测重复并生成新 ID。
- PIE/正式运行发现无效或重复 GUID 时应明确报错，不静默改写身份。

## 尚未决定/实施

- 尚未创建任何 C++ 类型、模块依赖、测试或资产配置。
- 恢复挂接到具体哪个 World/Level 初始化回调，实施前再结合当前 UE 5.7.4 生命周期确定。
- `PermanentSave`/`USaveGame` 暂不属于第一阶段；第一阶段仅保证本次游戏运行期间跨回合保存。
- 方案允许后续讨论继续调整，实施前仍需按 harness 规则重新列出落地计划并等待用户确认。
