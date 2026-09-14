# 具体角色类

- FEAT-081 风墙反馈：Reassembly 的 SandWave Cue 实例池为 3 圈 × 3 层 × 128 扇区；接地墙身、顶部浪脊、后卷层等速径向扩散。M_CoreMorph_SandWave 的实例数据 0 是透明度、1 是组件提供的波龄，材质不用全局 Time，暂停时几何和流动一起冻结。源金属流／构建保持，沙尘方程与材质已按用户要求改造，不再要求此分支与源逐字／哈希一致。

- FEAT-081 第二批进行中：ACoreMorphBoss 新增 Reassembly 组件，与 Flight 共用原 154 蝠鲼组件，运行时新增 301 蝎子组件；唯一 Actor／ASC／Health 不变。DA_CoreMorphReassembly 存两形态采样和必要 VFX 成品，原 DA_CoreMorphVisualLayout 保持飞行专用 154 条。GA_Reassemble 捕获实际姿态和速度后取消飞行；完整构建提交 Scorpion GE，取消回滚 Manta，死亡清理所有查询面。Cue 创建并销毁瞬态实例网格／灯光；成功后沙尘尾效有界存活。未接入八足移动、尾刺或新 BT。验证状态以 archive 为准。

**何时读取：** 为某个具体角色新增专属 C++ 逻辑（专属能力、专属组件）时。

**当前活跃（继承 AFPSCharacterBase）：**

| 文件 | 对应角色 |
|---|---|
| `Source/TheManTest/Public/Characters/Infiltrator/FPSInfiltrator.h/.cpp` | 潜行者 FPS 版（空壳，SetupPlayerInputComponent 额外绑定 ScanAction） |
| `Source/TheManTest/Public/Characters/MaintenanceWorker/FPSMaintenanceWorker.h/.cpp` | 维修工 FPS 版（空壳） |
| `Source/TheManTest/Public/Characters/TheExecutive/FPSTheExecutive.h/.cpp` | 高管 FPS 版（空壳） |

**敌人：**

- FEAT-081：新增 `ABossEnemyBase : AEnemyBase → ACoreMorphBoss`，源码归 `Enemy/Boss/CoreMorph/`；一个 ASC／Health 管理两形态，不继承 `AHumanoidEnemy`。当前仅第一批蝠鲼外观与飞行预览；变形逻辑归具体头领，公共头领类不含形态字段。头领不做击退、布娃娃、血肉 Cue。详见 `11-enemy-ai.md` 和 FEAT-081 archive。

- FEAT-081 第一批反馈修正：编辑态 Actor／Capsule 原点直接对应主体，分件以相对变换跟随摆放；运行时从摆放反推固定编舞参考，切换到绝对分件姿态。不得再把编辑态 Actor 原点放在距主体 160 米的旧轨迹原点。

- FEAT-081 全身路线适配：`CoreMorph/Movement/CoreMorphFlightMotion.h/.cpp` 只接收实际世界位置／DeltaTime，驱动全身朝向、侧倾、扑翼／收翼和尾部三维轨迹跟随，并加入种子控制的平滑随机变化。FlightComponent 持有运动状态与有界尾迹历史；暂停／取消／死亡冻结、复位清空。原 TailMotion 已合并删除；FlightPath 只保留原参考路线的位置计算，不再持有分件或动作时间表。
- `ACoreMorphFlightRoute` 是可摆放的 Spline 路线 Actor；Boss 的 FlightComponent.FlightRoute 选择实例，RouteSpeed 控制移动，开放路线到末端结束、闭合路线持续循环。MotionRandomness 控制幅度（默认 .18，0 关闭），MotionSeed 非零可复现；随机参数在复位时读取。空路线引用继续使用 13.4 秒源对照路线。
- `CoreMorphFlightReview` 可选 Routes 列表用于 `/Game/Maps/CoreMorph/L_CoreMorphRoutes` 三路线观感检查：等待 Boss BeginPlay 后自动播放第 1 条，1／2／3 切换并重播同一头领。列表为空的原检查地图保留 V 手动启动。该输入和 UI 只属于临时 Review 相机。
- FlightMotion 的 AccelerationIntent／PowerStroke 将提前发力意图接入翼部表现；Spline 速度在发力就绪后逐步累积，RouteAcceleration 配置推力加速度。源位置曲线仍保留，前瞻速度只驱动翼部发力。
- 双速轴向侧滚：TravelRotation 与 ±360° RollAngle 分离，慢速 100°/s、快速 460°/s 上限，按剩余角度制动；翼根／翼尖耦合弹性模式根据转速和角加速度计算滞后、曲面卷曲及回摆，尾迹带完整旋转。bRandomRolls 默认开，RandomRollSpacing 默认 65000 cm，按种子和飞行状态筛选触发；MotionRandomness 只控制原平滑噪声，关闭随机侧滚应使用 bRandomRolls。只在已有飞行 GA 激活时允许机动，不引入闪避或新技能。临时 Review 相机 Q 慢滚／E 快滚；暂停和取消冻结，复位清除惯性状态。编译及 AdaptiveMotion／RollMotion／EditorPlacement／实际 PIE FlightBatch／RouteReview 均通过，观感等用户校验。

- 2026-09-05当前：ExplosionHitReaction仅有动画分支，Rig模式/参数/求解器已删除。默认HitReactionPostProcess为Humanoid/_Shared/Animations/Logic/ABP_Humanoid_HitReaction_C。BP_Phantom仅配置前后左右4条AS_Humanoid_BlastRifle方向动画；具体Skeleton成品在外部资源项目适配。ApplyAnimationRootMotion默认开启，以扫掠胶囊消费非下落水平根位移，反应期间临时暂停Movement模式，结束/关闭恢复，死亡不恢复。组件构造启用Tick以消费位移，受击部位分类及HumanoidReactionBones.h已删除。

- 致命枪击新增Enemy|Death.ProjectileKillKnockbackSpeed=250cm/s、ProjectileKillUpwardSpeed=120cm/s，沿弹道与世界上方向给整个布娃娃叠加速度；两个值设0关闭。ABulletBase只在活体变死体的直接伤害之后施加一次，随后仍保留ProjectileHitImpulse的命中点冲量。打旧尸体不重复全身击飞，爆炸范围伤害保持独立径向冲量。


- Phantom原始Rifle_01持枪配置：WeaponAttachSocket=hand_r_wepSocket（父骨骼hand_r_wep），WeaponMesh单位缩放。这是TMIIR Overview的原始配置，动画驱动武器骨骼完成Aim/Relax变化；禁止根据状态切换hand_rSocket_Aim/Relaxed，也不要恢复旧固定Aim挂点和.9缩放。

- 当前死亡流程：AEnemyBase::OnDeath为virtual；停止技能/AI/移动、关闭胶囊/Actor Tick、移除波次订阅，Mesh有PhysicsAsset时转布娃娃（暂停动画、禁用PostProcess，全身体物理）。Enemy|Death暴露CorpseLifetime=5游戏秒（最小.1）与ProjectileHitImpulse=5000 kg cm/s。尸体保留Pawn对象类型以兼容所有既有弹体查询，范围爆炸不再按Enemy类型排除。没有PhysicsAsset时仍延时消失，但无法布娃娃。
- AHumanoidEnemy::OnDeath置AIState=Dead/bIsAiming=false，并关闭附着武器碰撞以免干扰布娃娃；APhantom死亡取消隐身。重复死亡不会刷新尸体寿命，身体Decal和附着弹在最终EndPlay清理。


- APhantom新增默认true的bStationaryHitTest（Phantom|Testing），仅本类固定靶用途；BeginPlay停AI/移动/角色Tick，受击不进入父类转向逻辑，保留Mesh和方向性受击组件。开关在生成时生效，取消后重新PIE恢复。


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
