# 具体角色类

**何时读取：** 为某个具体角色新增专属 C++ 逻辑（专属能力、专属组件）时。

**当前活跃（继承 AFPSCharacterBase）：**

| 文件 | 对应角色 |
|---|---|
| `Source/TheManTest/Public/Characters/Infiltrator/FPSInfiltrator.h/.cpp` | 潜行者 FPS 版（空壳，SetupPlayerInputComponent 额外绑定 ScanAction） |
| `Source/TheManTest/Public/Characters/MaintenanceWorker/FPSMaintenanceWorker.h/.cpp` | 维修工 FPS 版（空壳） |
| `Source/TheManTest/Public/Characters/TheExecutive/FPSTheExecutive.h/.cpp` | 高管 FPS 版（空壳） |

**敌人：**

- AHumanoidEnemy显式持有VisibleAnywhere/BlueprintReadOnly的ExplosionHitReaction组件UPROPERTY，同名原生子对象身份不变；它使具体蓝图的动画配置能够保存和重新编译后恢复。之前只CreateDefaultSubobject而无成员引用导致新增Sequence配置不能持久化。组件ReactionMode默认Animation，BP_Phantom填五条当前Skeleton兼容Sequence；ControlRig选项保留。死亡流程及HitReactionPostProcess软类不变。

- Phantom原始Rifle_01持枪配置：WeaponAttachSocket=hand_r_wepSocket（父骨骼hand_r_wep），WeaponMesh单位缩放。这是TMIIR Overview的原始配置，动画驱动武器骨骼完成Aim/Relax变化；禁止根据状态切换hand_rSocket_Aim/Relaxed，也不要恢复旧固定Aim挂点和.9缩放。

- 当前死亡流程：AEnemyBase::OnDeath为virtual；停止技能/AI/移动、关闭胶囊/Actor Tick、移除波次订阅，Mesh有PhysicsAsset时转布娃娃（暂停动画、禁用PostProcess，全身体物理）。Enemy|Death暴露CorpseLifetime=5游戏秒（最小.1）与ProjectileHitImpulse=5000 kg cm/s。尸体保留Pawn对象类型以兼容所有既有弹体查询，范围爆炸不再按Enemy类型排除。没有PhysicsAsset时仍延时消失，但无法布娃娃。
- AHumanoidEnemy::OnDeath置AIState=Dead/bIsAiming=false，并关闭附着武器碰撞以免干扰布娃娃；APhantom死亡取消隐身。重复死亡不会刷新尸体寿命，身体Decal和附着弹在最终EndPlay清理。

- 2026-09-05共享人形全身反应：AHumanoidEnemy新增HitReactionPostProcess软类，默认`/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction_C`（完整对象路径见代码），BeginPlay设置Mesh OverridePostProcessAnimBP；非Phantom人形自动接入。同骨骼层级可复用，其他骨架须配置BoneMapping并保证Rig层级兼容。Phantom SK_Mannequin原资产级PostProcess槽已清空，避免双重叠加。

- APhantom新增默认true的bStationaryHitTest（Phantom|Testing），仅本类固定靶用途；BeginPlay停AI/移动/角色Tick，受击不进入父类转向逻辑，保留Mesh和方向性受击组件。开关在生成时生效，取消后重新PIE恢复。

- 2026-09-05：AHumanoidEnemy构造原生ExplosionHitReaction组件（UEnemyHitReactionComponent，Enemy/Humanoid/Animation）。负责存活敌人的爆炸方向、部位与恢复参数，不驱动AI/胶囊/死亡。默认Enabled=true、MaxAngleDegrees=22、AttackDuration=.055、RecoveryDuration=.55游戏秒。Phantom Mesh已接专属PostProcess AnimBP，其他骨架需单独配置兼容Rig，不自动替换其动画。

- 默认Enemy Hit通过Enemy/_Shared/Audio/EnemyHitAudioComponent懒创建每实例痛呼状态，GCN_EnemyHit只配置PainSound/PainVolumeMultiplier/PainCooldown；不在共享GC CDO保存冷却，不改变EnemyBase继承或AI。

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
