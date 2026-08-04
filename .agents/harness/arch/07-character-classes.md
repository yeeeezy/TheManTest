# 具体角色类

**何时读取：** 为某个具体角色新增专属 C++ 逻辑（专属能力、专属组件）时。

**当前活跃（继承 AFPSCharacterBase）：**

| 文件 | 对应角色 |
|---|---|
| `Source/TheManTest/Public/Characters/Infiltrator/FPSInfiltrator.h/.cpp` | 潜行者 FPS 版（空壳，SetupPlayerInputComponent 额外绑定 ScanAction） |
| `Source/TheManTest/Public/Characters/MaintenanceWorker/FPSMaintenanceWorker.h/.cpp` | 维修工 FPS 版（空壳） |
| `Source/TheManTest/Public/Characters/TheExecutive/FPSTheExecutive.h/.cpp` | 高管 FPS 版（空壳） |

**敌人：**

| 文件 | 对应角色 |
|---|---|
| `Source/TheManTest/Public/Enemy/EnemyBase.h/.cpp` | 敌人基类，ASC + UEnemyAttributeSetBase 挂在自身 |
| `Source/TheManTest/Public/Enemy/Humanoid/HumanoidEnemy.h/.cpp` | 人形怪基类（巡逻逻辑、转身请求、AI 状态） |
| `Source/TheManTest/Public/Enemy/Humanoid/HumanoidAIController.h/.cpp` | 视觉感知 + 黑板 + 行为树启动 |
| `Source/TheManTest/Public/Enemy/Humanoid/Phantom/Phantom.h/.cpp` | 第一个具体人形怪（空壳） |
| `Source/TheManTest/Public/Enemy/Nightmare/NightmareEnemy.h/.cpp` | 梦魇基类（空壳） |

**已删除（FEAT-041）：** 旧 `AInfiltrator` / `AMaintenanceWorker` / `ATheManExecutive`（继承旧 `ATheManCharacterBase`）连同基类已删除，备份在 scratchpad/deprecated-char-backup-session43。对应旧 `BP_Infiltrator` / `BP_MaintenanceWorker` / `BP_TheExecutive` / `BP_TheManCharacterBase` 需在编辑器一并删除。

> 现役角色差异化全部在对应的 BP_FPS* 蓝图中配置，C++ 文件暂无额外逻辑。

## Nightmare FlyingBug2

- `ANightmareEnemy` 保持空的 Enemy 语义基类。
- `ANightmareFlyingBug : ANightmareEnemy` 位于 `Enemy/Nightmare/FlyingBug2/`，负责 MOVE_Flying 三维随机游荡和 Locomotor `FVectorDamper` 平滑。
- 具体蓝图 `/Game/Enemy/Nightmare/FlyingBug2/Blueprint/BP_NightmareFlyingBug2` 配置最终 Mesh 与 `RoamAnimation`；C++ BeginPlay 显式循环播放动画。
