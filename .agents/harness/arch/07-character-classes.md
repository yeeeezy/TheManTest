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
- `ANightmareFlyingBug : ANightmareEnemy` 位于 `Enemy/Nightmare/FlyingBug2/`，使用 `MOVE_Walking`、运行时 `UControlRigComponent` 执行 Locomotor + FullBodyIK 多足链、`FVectorDamper` 平滑速度和地面法线对齐完成贴地爬行。SkeletalMesh 的 `Default Animating Rig` 仅用于编辑器预览，不能替代运行时组件映射。
- 碰撞胶囊始终世界竖直；Actor 只跟随移动切线 Yaw，`CharacterMesh0` 单独跟随地表 Pitch/Roll。该模型参考姿势的视觉正前方是局部 `+Y`，所以地表旋转必须用 `MakeFromYZ(Forward, SurfaceNormal)`，禁止按通用局部 `+X` 使用 `MakeFromXZ`，否则会稳定横向爬行。Control Rig 在移动/地表对齐后显式 Update，并刷新最终骨骼，确保六条接地腿输出同帧进入渲染 Pose。
- FEAT-076 经骨骼层级与参考姿势全局高度复核，FlyingBug2 是六条接地腿的生物；`tent_low*` 位于头部，禁止作为 Feet。Locomotor 使用两组交叉三足支撑：左前+右中+左后 Phase `0`，右前+左中+右后 Phase `0.5`。禁止恢复前三排左右腿同相的 `0/0.333/0.667` 配置，该配置会产生机械式横排摇摆。六个 FeetTransform 逐项进入六个 FullBodyIK Effector。
- FlyingBug2 运行时动画链为 `Anim_Nightmare_bug2_walk1 -> AnimGraph Control Rig -> Output Pose`。原 Walk 是完整 Source Pose，保留头、触须、躯干与尾部动作并向 FBIK 提供自然关节弯曲初值；禁止恢复外置 `UControlRigComponent` 完整骨架 Output 覆盖。
- 具体蓝图 `/Game/Enemy/Nightmare/FlyingBug2/Blueprint/BP_NightmareFlyingBug2` 配置最终 Mesh 与 `RoamAnimation`；C++ BeginPlay 显式循环播放动画。
