# FEAT-081 蝠鲼／蝎子头领分批迁移

## 用户授权与边界（2026-09-13）

用户确认“分步骤迁入，每轮迁入审查完成后我自己校验”。本轮仅准备工作及第一批外观、飞行；自验后停下，等待用户校验，再开始变形批次。头领无需人形击退、布娃娃。

- 目标保持 UE 5.7.4；源 UE58Blank 为 5.8.2，不升级目标，不改源项目。
- `AEnemyBase → ABossEnemyBase → ACoreMorphBoss`：一个 Pawn、一个 ASC、一套 Health；公共头领层不持有变形逻辑。
- 战斗阶段继续用 `FEnemyPhaseSkillSet`，形态由这只具体头领管理；二者独立。最终专属主 BT 决策、按需拆形态子树；GA 技能、GE 伤害与状态、Gameplay Cue 特效与声音。
- 原有人形怪受击、布娃娃、持枪与正式 AI 仅作回归，不是头领要求。
- 源当前实际单向蝠鲼→蝎子；R 是调试复位，不应误报为已实现逆向变形。

## 起点与版本处理

- Git 起点 `58817b1`，写入前工作区干净，无需空 checkpoint。FEAT-080 经本轮授权暂停；其他未完成工作不改。
- 源只读快照：`D:/CodexRollbackBackup/CoreMorphMigration-20260913/UE58Blank-baseline.zip`，5195 文件、约 3609 MB；同目录 `source-sha256.json`。
- 外部准备工程：`D:/Unreal Projects/CoreMorph57Prep`。用源 FBX 与数值布局在 5.7 重建成品，经 AssetTools 迁入。5.8 uasset 不直接降版复制。
- 自动编辑器进程从启动即 RenderOffscreen、隐藏窗口；不接管前台，不遮挡右侧 PS Remote。

## 四批交付

1. 外观与飞行：154 蝠鲼分件、5 材质、布局、头领身份和飞行 GA；独立检查地图。在 13.4 秒粒子释放前停住。
2. 变形重组：迁入 301 蝎子分件及必要采样；金属流、火花、冲击／沙尘与形态交接。源粒子为 ISM，并非 Niagara。
3. 蝎子移动与攻击：保留八足和尾刺，隐藏源移动 Actor 改成同头领的运动组件；专属主 BT 与攻击 GA。
4. GAS 闭环：正式伤害／状态 GE、Cue 声画、三枪及附着面适配、阶段与形态独立、爆炸去重、所有取消／死亡／退出清理和人形回归。

## 第一批实现与审查

- 新增公共 `ABossEnemyBase`，专属 `ACoreMorphBoss`、`UCoreMorphVisualLayout`、`UCoreMorphFlightComponent`、`FCoreMorphFlightPath`、`UGA_CoreMorphFlight`、`UGE_CoreMorphManta`。
- 飞行沿用 FEAT058 曲线和翼／尾运动。固定编舞坐标与实际 Pawn 根位置分离；根跟随飞行，154 可命中静态分件均属于同一头领。
- `State.CoreMorph.Form.Manta` 由无限 GE 维护；飞行 GA 只授予一次。取消停止位移，死亡沿用 EnemyBase 终态与延时销毁并关闭分件碰撞。无骨骼资产、血肉 Cue、物理击退。
- 资产目录 `/Game/Enemy/Boss/CoreMorph/{Blueprint,Data,Meshes,Materials}`；检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`。地图专属 `ACoreMorphFlightReview`：V 播放、R 复位、P 暂停、F 跟随／自由相机，不改正式玩家输入与默认地图。
- 154 网格、5 材质已重建并迁入，包围盒最大误差 0.00013733 cm，159 包迁移 SHA-256 一致。Blueprint 与检查地图已在编辑器生成、编译保存。
- 修复：5.7 材质 Python API 差异；commandlet Interchange 触发 Slate 断言，改用离屏完整编辑器；旧导入器轴向错误，清除准备工程错误成品后全新导入通过；原生 GE 构造改为具名默认组件，修复编辑器启动错误。
- 第一批已完成代码审查与自验，等待用户手动校验；未开始第二批，未提交／push。

## 第一批验证结果（2026-09-13）

- Development Editor / Win64 最终编译成功：外部准备工程 `Saved/Review/build-final.log`。
- 冷启动资产审计通过：`cold-audit-04.log`、`cold-audit.json`。161 个头领资产，加 2 个检查地图资产；无 Redirector，无源项目模块／场景依赖，无重定向动画工作资产。头领 Blueprint 在离屏编辑器中打开、编译、保存。
- `TheManTest.Enemy.CoreMorph.FlightBatch` 最终 Success：`flight-pie-final.log`。真实 PIE 两次启动，验证一 ASC／属性集／飞行授予、154 同 Owner 分件且无重复、GE 扣血不改变朝向或取消飞行、阶段不改形态和授予、取消冻结、复位不回血／不重置阶段、死亡停飞并在基类寿命后销毁、实时世界 Tick 飞行到 13.4 秒自动结束、飞行中退出 PIE 销毁。
- 五项既有回归均 Success：`batch1-validation.log` 内 `ThreeWeaponBaseline`、`ThreeWeaponPIESwitch`、`ProjectileCrosshairAim`、`ExplosionRadialDamage`、`EnemyDeathRagdoll`。该日志早一版 FlightBatch 的失败为测试时序问题，已由最终单项运行覆盖；不得把整份早期日志写成 6/6 成功。
- 测试时序修正：结束边界用跨过边界的一帧检查钳制，避免浮点求和恰好小于 13.4；`FEndPlayMapCommand` 只请求退出，等待编辑器下一 Tick 清理后再检查 World／弱引用。
- 实际渲染截图：目标 `Saved/CoreMorphMigration/Flight-{0000,0500,1000,1340}.png`。检查相机采用固定曝光并按视口比例容纳完整尾部；检查场地是简易地面，不是源沙漠场景。
- `source-final-audit.json`：5195 个源文件 SHA-256 一致，8 个飞行函数体与源逐项一致。源工程保持未修改；本次未重新启动源 PIE，不替代源原有 FEAT058/059/060 验收证据。
- 新资产约 14.98 MB，Git LFS 规则有效，`git diff --check` 通过。原有武器／人形代码、资产、默认地图与目标引擎版本未修改。
- 编辑器冷启动仍记录既有玩家 `M_UE4Man_Body` 材质及 AimIK 缓存警告；这些资产不属于头领依赖，本批未修改。头领 5 个材质冷加载无编译失败。

## 用户手动校验入口

打开 `/Game/Maps/CoreMorph/L_CoreMorphFlight`，Play 后 V 播放、P 暂停／继续、R 复位、F 跟随／自由相机（自由相机 WASD／鼠标）。本批停在 13.4 秒，粒子重组与蝎子在下一批。重点对照外观、起飞扑翼、盘旋爬升、压翼俯冲及尾部运动。收到用户反馈后继续；不要自行进入第二批。

## 证据位置

外部准备工程 `Saved/Review/` 保存导入、迁移、编译与编辑器日志；目标 `Saved/CoreMorphMigration/` 保存实际 PIE 截图。源 FEAT058/059/060 已验收影像保留于源和外部快照作对照。

## 后续必须处理

### 第一批反馈：加速前发力与翻转卷翼（2026-09-13，未完成）

- 用户提出加速前自动大力挥翼，并希望随机轻柔／快速“水平翻转”且翅膀随之卷起。已询问翻转是沿飞行方向侧滚，还是水平面掉头；该问题待用户回复，翻转实现尚未开始，不能宣称已有侧滚。
- 写入前把三路线检查成果保存为本地 WIP checkpoint `3938497`。本轮仅修改 FlightMotion、FlightComponent 与现有测试源码；没有改资产或源工程，也未 push。
- 已准备加速前发力代码：Motion 接收 AccelerationIntent，平滑形成 PowerStroke 并提前加强抬翼／扑翼；Spline 当前速度从 0 起步，发力达到目标响应的 70% 后才加速，下拍提供更强推力，RouteAcceleration 默认 16000。目标速度降低仍保留原来的立即响应。源参考路线不改位置数学，以前瞻 .4 秒的速度差提前驱动挥翼。
- 测试已补“身体未位移前已经发力”和“实际 Spline 速度逐步建立”；旧常速两秒=18000 cm 断言改为实际积分路程与样条位置核对。**尚未编译或 PIE，当前编辑器中仍是上一轮 DLL。** 检测到用户前台 TheManTest 编辑器 PID 33624，已请求保存并关闭以便冷编译，未擅自关闭用户编辑器。
- 下一步先接收翻转方向说明及编辑器状态；完成对应轻柔／快速机动和卷翼设计后编译，再跑 AdaptiveMotion／FlightBatch／RouteReview。当前第一批整体继续等待用户观感校验，不进入重组批次。

### 第一批观感检查：三条示例路线（2026-09-13，自验通过，待用户校验）

- 用户要求提供几条路线自行判断自然程度。本轮新增独立 `/Game/Maps/CoreMorph/L_CoreMorphRoutes`，一个头领和三个可编辑 Spline；原对照地图不改。写入前将全身适配成果保存为本地 WIP checkpoint `c8b8e0e`，无 push。
- 复用检查相机 `CoreMorphFlightReview`，仅示例地图配置 Routes 列表后绑定 1／2／3 切换并立即播放。等 Boss BeginPlay 完成后自动播放第一条；保留 V 播放、R 复位、P 暂停、F 相机。每次切换复用同一个 Boss、ASC 和飞行 GA，取消旧飞行再启动新路线。
- 路线为 Gentle Climb（8000 cm/s，缓弯爬升）、S-Turns（11000 cm/s，左右转弯伴轻微升降）、Orbit and Dive（10000 cm/s，盘旋上升后俯冲拉平）；随机强度仍 .18，种子 0。
- `routes-build.log` 与 `routes-tests-build.log` 均编译成功。地图初次用 AssetTools 直接复制 World 后加载，因 Python 持有 World 包触发编辑器 World Memory Leaks 断言，未生成磁盘新地图且原地图未改。改用 LevelEditorSubsystem.NewLevelFromTemplate，并在重开验证前释放 Python Actor 引用，`routes-author-02.log` 创建／保存／重开通过，无 Error。
- `routes-authored.json` 记录 3 条路线全部控制点、速度和长度换算时长：19.91／22.32／27.14 秒；重开地图后逐点回读通过。一个 Boss、一个 Review 和三个 Route 引用完整，Maps/CoreMorph 范围无 Redirector；只新增约 99 KB 检查地图，复用既有地面材质。原地图、头领资产和运动求解器未改。
- `routes-validation.log`：`CoreMorph.RouteReview` 与原 `CoreMorph.FlightBatch` 均 Success。三条保存路线分别在实际 PIE 正常世界 Tick 下完整播放至末端；验证自动起飞、飞行中切换、单 GA／154 分件、所有末端姿态有限、GA 结束、重新播放与活动飞行退出 PIE 清理。新地图完成审查，自动编辑器已退出；本轮修改未提交／push。
- 用户入口：打开 `/Game/Maps/CoreMorph/L_CoreMorphRoutes` 后 Play 自动播放第 1 条；1／2／3 切换并重播，P 暂停，R 复位后 V 播放，F 自由／跟随相机。`Saved/CoreMorphMigration/RouteReview-{1,2,3}.png` 是实际渲染截图；离屏视口较窄，仅作运行证据，自然程度仍由用户在编辑器观察确认。新检查地图同样列入最终验收后的临时工具清理范围。

### 第一批反馈：全身路线适配与平滑随机（2026-09-13，自验通过，待用户校验）

- 用户要求所有部位根据线路和实时状态变化，换路线不重写动作，另要求增加随机性。本轮属于第一批飞行反馈；不进入重组批次。已将上一轮尾部反馈保存为本地 WIP checkpoint `bf812e8`，无 push。
- `FCoreMorphFlightPath` 缩减为原对照路线的位置提供者；移除固定 `DivePose/SourcePose/ChoreographyTime` 表现代码。新 `FCoreMorphFlightMotion` 只接收世界位置和 DeltaTime，计算速度、向上速度、下降比例、加速度及有符号转向；身体朝向／侧倾、扑翼幅度／频率／收翼由这些运动量驱动。刚性躯干和核心统一跟随身体，保持装配关系。
- 尾部改为按距离采样身体实际走过的三维轨迹，短历史用初始方向向后延伸；历史只保留尾长所需范围，替代旧固定半径、固定时点的转弯。旧 `CoreMorphTailMotion.h/.cpp` 合并进全身 Motion 后删除，避免两套尾部驱动并存。
- 随机由种子决定的低频连续信号提供，翼面／尾部共享连续变化；轻微改变节奏、幅度、左右差异及身体侧倾，不改变路线或各分件装配归属。`MotionRandomness` 默认 0.18，0 关闭；`MotionSeed` 默认 0，在复位时取新种子，非零可复现。振荡与平滑仍使用 DeltaTime，但没有“第几秒开始动作”的表现时间表。
- 新增可摆放 `ACoreMorphFlightRoute`，自带可编辑 Spline。Boss 的 FlightComponent 中 `FlightRoute` 选该实例，`RouteSpeed` 控制速度；从样条首点开始，开放路线到末端结束，闭合路线循环直到取消。空引用仍走原 13.4 秒对照路径。只在飞行开始选定路线对象，速度可实时变化；路径被销毁时安全结束 GA。运行 Tick 以最多 1/120 秒步长采样，暂停／取消／死亡冻结，复位清空路径历史。
- 原检查地图仍用于用户复查，未新增／修改目标资产。Review 屏幕时间改为实际飞行秒数，不对自定义路线显示固定 13.4 秒终点。源工程与目标引擎版本未改动。
- `adaptive-build.log`：Development Editor Win64 编译成功，无新增 C++ 警告。`adaptive-validation.log`：`AdaptiveMotion/EditorPlacement/FlightBatch` 三项均 Success；实际 PIE 使用两条镜像 Spline 检查转弯侧倾、升降、距离跟随和末端停止，闭合路线飞过 13.4 秒仍活动，零速度悬停与路线销毁正常结束。原路径的取消／暂停／复位／死亡／退出和编辑器移动／旋转／缩放／重构造／复制均通过。
- AdaptiveMotion 直接验证三维弯曲历史位置（3 cm 容差）、升降自动收展翼、同种子可复现、异种子差异、随机关闭、连续帧平滑与 30／120 FPS 运动响应。`adaptive-source-audit.json`：5195 源文件未变；5 个保留的轨迹函数与源完全一致。旧翼部动作公式已改为实时驱动，历史“翼部不变”结论只适用于之前批次。
- 差异审查完成：表现求解器无 Schedule、固定轨道半径或飞行绝对时间触发；振荡器仅以 DeltaTime 累积相位。全局随机只在实例复位选择种子，分件求值无随机抽签；尾迹按最大尾长裁剪。无新增 GAS、计时器或伤害归属；目标资产和源项目均未改变。本轮修改未提交／push，自动编辑器已退出，等待用户第一批观感校验。

#### 用户调整路线／随机参数

1. 在关卡放置原生 `CoreMorphFlightRoute`，选中其 `FlightRoute` Spline 编辑控制点，需要循环时开启 Closed Loop。
2. 选择 `BP_CoreMorphBoss` 的 Flight 组件，在 `CoreMorph|Flight` 的 Flight Route 指定该 Actor；开始飞行会移至样条首点。Route Speed 默认 11000 cm/s，设 0 可停在当前位置。
3. `CoreMorph|Motion` 的 Motion Randomness 默认 0.18；0 关闭随机，数值越大变化越明显。Motion Seed 为 0 时复位会换种子，非零可复现。调整随机参数后按 R 复位再 V 播放。
4. Flight Route 留空恢复原对照路线。姿态会随新路线计算；原源工程仍保留原先逐时编排的视觉对照。当前未接入避障或战斗 AI，也未进入重组批次。

### 第一批反馈：爬升驱动尾巴摆动（2026-09-13，自验通过，待用户校验）

- 用户要求“改成根据是否爬升自己计算摆动”。按本轮明确请求处理第一批飞行反馈；不进入第二批。写入前将已知 Capsule 修正保存为本地 WIP checkpoint `441b533`，没有 push。
- 新增专属 `FCoreMorphTailMotion`，由 FlightComponent 每个有效飞行 Tick 用身体实际世界位移／有效 DeltaTime 驱动。CharacterMovement 当前关闭，不读取其 GetVelocity；世界 Z 速度为正才增强爬升波，侧倾不会误判爬升。
- 尾巴摆动相位仍由连续振荡器提供，各节错相；幅度由移动速度和向上速度决定，0.3 秒响应时间平滑进出爬升。尾根固定，幅度向尾尖递增。移除原 SourcePose 在 9.4 秒触发的固定上下甩尾脉冲。平飞／下降保留小幅巡航摆动，不是物理尾链。
- 本次只改尾巴摆动驱动。飞行轨迹、翼部运动与尾巴横向盘旋弧形保留原编排；不能宣称整个尾巴或飞行 AI 已适配任意路线。
- 暂停／取消／到达批次终点／死亡冻结尾部状态，重播清零；不新增计时器、组件、资产或 GAS 状态。没有编辑源工程或目标资产。
- `tail-build.log`：Development Editor Win64 编译成功。`tail-source-audit.json`：5195 个源文件 SHA-256 一致，7 个轨迹／调度函数及翼部变形函数块仍与源一致；旧“8 个函数全部一致”仅适用于本反馈前的历史快照。
- `tail-validation.log`：`CoreMorph.TailMotion`、`CoreMorph.EditorPlacement`、实际 PIE `CoreMorph.FlightBatch` 三项均 Success。验证爬升／平飞／下降、不同时间开始爬升、平滑退场、尾根锚定与 30／120 FPS；实际 PIE 覆盖早于旧脉冲时点的爬升响应、暂停／取消／复位／死亡与下降减弱。完整飞行仍在 13.4 秒终止，退出 PIE 清理通过。编译无新增 C++ 警告，仍存在原有 StructUtils 弃用提示。
- 完成代码差异审查；本轮未改变资产。自动截图已生成，但当前离屏编辑器视口窄、头领在图中较小，不据此宣称尾部观感已人工验收。自动编辑器已退出；尾部修改未提交／push，等待用户在原检查地图复查。

### 第一批反馈：编辑器 Capsule 与主体偏移（已修正，待用户复查）

用户于本批检查时指出未 PIE 的 Capsule 远离分件，已确认动手修复。原因：编辑状态以旧编舞原点作为 Actor 位置，而主体加了 (-16000,0,1600) cm 的起飞偏移；只在 BeginPlay 后同步根位置。改为 Actor 位置直接表示主体，编辑和 BeginPlay 用逆起飞偏移推导固定编舞坐标；原飞行计算不改。只对已知检查地图迁移摆放，保持 154 分件世界姿态和原路线。修正前本地 WIP checkpoint 为 `0516b0d`，无 push。本次修正完成后继续等待用户第一批校验。

- 实现：编辑态分件使用相对变换，拖动时直接跟随 Actor；BeginPlay 记录独立编舞坐标后重建为绝对世界姿态。Actor 新位置直接代表主体，检查地图从 (0,0,900) 调整到 (-16000,0,2500)，得到的编舞参考仍为 (0,0,900)。Capsule 保持 NoCollision，分件命中查询规则不变。
- `origin-before.json`／`origin-fixed.json`：154 分件修正前后世界位置最大变化 0 cm，旋转和缩放一致；保存并重开地图后仍一致。根至刚性核心距离 0.7684 cm。
- `origin-build.log`：最终 Development Editor Win64 编译成功。`origin-validation.log` 中 FlightBatch Success（包含原世界编舞参考、完整实时飞行、取消、死亡及退出检查）；`origin-editor-verified.log` 中 EditorPlacement Success（移动、旋转、缩放、三次重构造和真正编辑器复制）。
- 测试入口修正：SpawnActor 的 Template 生成默认会复合模板根变换，并不等同编辑器复制，改用 EditorActorSubsystem.DuplicateActor；AutomationOpenMap 会自动启动 PIE，编辑态测试改用 FEditorFileUtils.LoadMap。早期失败日志保留供追溯，以 origin-editor-verified.log 为最终编辑器结果。
- Blueprint 在离屏编辑器打开、编译保存验证；只有检查地图产生资产差异，没有修改 154 网格／5 材质或源项目。修正代码仅涉及 FlightComponent 与回归测试。自动编辑器已退出，本次修正未提交／push。

用户另确认最终验收后清理临时检查地图／相机类／演示截图日志；自动回归测试保留有价值的断言，临时演示部分清除。源项目与对照备份保留，本轮不提前删除验收工具。

三枪最终适配尚未完成：现有枪口遮挡与附着弹偏向 skeletal mesh；爆炸需按头领身份去重并选择真实表面检查遮挡。阶段技能重复授予风险、BT 取消边界在战斗批次处理。第一批不宣称蝎子／重组／完整战斗或三枪命中已验收。
