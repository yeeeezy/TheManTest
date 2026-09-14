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

## 第二批：变形重组（自审通过，待用户校验）

- 用户明确“可以挺好的，接着导入吧”，第一批飞行／路线／双速侧滚已接受，授权按原批次继续。写入前 WIP `cc72c0f` 保存已验收飞行；无用户编辑器进程，不需要再次询问。
- 本批迁入 301 蝎子成品分件、采样、六种蝎子材质和源金属流／火花／沙尘／碎土依赖。保持源只读、目标 UE 5.7.4；外部 prepare_scorpion.py 已完成 301 网格和 6 材质，包围盒误差最大 0.0001431 cm。
- 新增具体头领 ReassemblyComponent，复用原 154 蝠鲼分件并捕获实际姿态／速度；金属流、构建引导和沙尘方程沿用源。GA_CoreMorphReassemble 负责事务；GE_CoreMorphTransforming、GE_CoreMorphScorpion 管理 Tag；GC_CoreMorph_Reassembly 创建／移除瞬态 ISM 和灯光，成功后保留有界沙尘尾效，取消／死亡立即清理。源码无原头领独立音频依赖，本批不虚构音效。
- 初次编译发现 TObjectPtr 的 auto* 推导错误，已显式 Get()/const auto& 修正；reassembly-build-02.log 编译成功。资产准备／迁移、Cue 冷回读和实际 PIE 尚待完成，不能宣称本批已验收。
- 完成后停在蝎子静态装配；八足移动、尾刺、专属 BT 和武器闭环仍属于后续批次。复位为检查工具，不作为逆向变形技能。
- 314 包 AssetTools 迁移完成，reassembly-migration-result.json／audit.json 校验一致。新增 DA_CoreMorphReassembly、GC_CoreMorph_Reassembly 和 L_CoreMorphReassembly；原飞行布局与地图不变。Blueprint 默认新增变形技能和组件数据引用，保存并重开通过。Python 的 EditAnywhere 属性必须用 get_editor_property 读取，初次脚本只在回读失败，第二次 reassembly-author-02.log 全部通过。
- reassembly-tests-build-02.log／reassembly-final-build.log 成功。reassembly-validation.log 六项 CoreMorph 测试全部 Success：真实 PIE 完整飞行→重组→有界沙尘、四个取消时点（.2／.8／2.2／4.8 秒）、三个死亡时点（.3／2.5／5.5 秒）、暂停、单 ASC／Health、阶段独立、两种 GA 各授予一次、活动变形退出及旧飞行／侧滚／编辑器摆放／三路线回归。
- reassembly-cold-audit.log/json：477 项头领资产冷加载，455 布局条目及采样、314 迁移包哈希、Cue AssetRegistry GameplayCueName=GameplayCue.CoreMorph.Reassembly、源模块依赖为零、Redirector 为零全部通过。新资产直接进入正式语义目录，未生成供应商或迁移暂存目录。
- Source 审计：5195 文件未变；reassembly-math-audit.json 中 Schedule、PeelStart、BuildStart、BuildSpan、PrepareBuildGuides、StreamFor、PrepareStreams、CoilOffset、FlowPoint、GroundFlowPoint、ParticlePoint、UpdateImpact 共 12 函数仅类型名适配后逐项一致。源 SourcePose 被实际飞行姿态／速度捕获替代，未恢复旧翼部时间表；地面查询改用固定世界参考并忽略同一头领。
- 已查看真实 Reassembly 截图的构建和成型阶段；发现金属流阶段检查相机过近，已加入实际可见粒子包围盒进行取景，冷编译通过。reassembly-final-pie.log 再次 ReassemblyBatch Success、退出码 0；复查 1280×720 的完整金属流、落地构建和无残留蝎子成型图通过。自动编辑器已退出，第二批等待用户观感验收，不进入第三批。
- 用户入口：L_CoreMorphReassembly，Play 后 V 完整飞行并自动重组，M 直接从当前姿态重组，P 暂停，R 复位，F 相机。完成约 5.2 秒后提交蝎子形态，沙尘尾效在变形开始后约 9.2 秒清理。没有新增声音、攻击或逆向变形。

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

### 第一批反馈：加速前发力与双速侧滚（2026-09-13，自验通过，观感待校验）

- 用户已确认沿飞行方向侧滚一圈，分慢／快两种；快滚要求像转动带双翼的棍子，翼根跟随、翼尖因惯性和风阻滞后。用户已保存关闭编辑器，进程复核无 UnrealEditor；无需再次询问。追加本地 WIP `6add0f3` 保存未编译的提前发力代码后开始本轮实现。
- 写入前把三路线检查成果保存为本地 WIP checkpoint `3938497`。本轮仅修改 FlightMotion、FlightComponent 与现有测试源码；没有改资产或源工程，也未 push。
- 已准备加速前发力代码：Motion 接收 AccelerationIntent，平滑形成 PowerStroke 并提前加强抬翼／扑翼；Spline 当前速度从 0 起步，发力达到目标响应的 70% 后才加速，下拍提供更强推力，RouteAcceleration 默认 16000。目标速度降低仍保留原来的立即响应。源参考路线不改位置数学，以前瞻 .4 秒的速度差提前驱动挥翼。
- 两种侧滚通过目标角度 ±360°、角速度和制动距离完成，不依赖路线绝对时刻；TravelRotation 独立于侧滚角，保持飞行轴与路线一致。翼根／翼尖两个耦合弹性模式响应实际角加速度和角速度，跨翼展连续弯曲并加入随转动能量衰减的波动；尾迹保留完整滚转姿态。
- 随机触发基于累计飞行距离和种子，避开发力、陡俯冲和急转；组件 bRandomRolls／RandomRollSpacing 可调。临时 Review 相机增加 Q 慢滚／E 快滚，沿用 1／2／3 路线切换。所有机动只在现有飞行 GA 内表现，不新增技能授予或闪避状态。
- roll-build.log：Development Editor Win64 冷编译成功，无新增 C++ 警告。roll-validation.log：AdaptiveMotion／RollMotion／EditorPlacement／FlightBatch／RouteReview 全部 Success，覆盖两方向完整一圈、双速惯性差异、回稳、复位、随机可复现、30／120 FPS，以及实际 PIE 两种侧滚暂停／取消、快速侧滚中死亡、三路线实际 Tick 机动与完整结束、活动技能退出清理。
- 再次执行 audit_adaptive_revision.py：5195 个源文件哈希不变，五个保留的位置／调度函数块与源一致；本轮没有改动 Content 资产。差异审查确认没有新增组件、计时器、GE 或技能授予，旧命中／阶段／形态规则未改。
- 旧离屏视口 982×187 不足以判断翼部细节；已只在 RouteReview 自动测试临时固定渲染表面为 1280×720，结束恢复，不更改用户保存的窗口或地图配置。roll-review-build.log 冷编译成功，roll-review-visual.log 的三路线实际 PIE 再次 Success、退出码 0；三张 RouteReview 截图均为 1280×720，已查看慢／快滚图，慢滚较舒展、快滚存在更明显弯曲（快速图仍有运动模糊，不能代替动态观感验收）。自动编辑器已退出。第一批继续等待用户观感校验，不进入重组批次。

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

### 第二批反馈：同心圆沙尘风墙（2026-09-13，验证中）

- 用户要求将平面沙尘改成风墙同心圆冲击波，已保存关闭编辑器。写入前 WIP `c34a7bc` 保留完整第二批；本轮只修改落地风墙表现与对应验证，不推进第三批。
- 三圈等速向外扩散，间隔 0.65 秒，每圈由接地尘幕、顶部浪脊和后卷层构成，高约 20 米；地面高度仍从落点周围采样。相邻角度使用连续波动，避免分件柱状突变。
- Cue 继续拥有全部临时网格，SandWave 池由 384 改为 1152 实例；两个实例数据分别传递透明度与自身时钟，材质向上流动随暂停冻结。无碰撞、无新增计时器或状态标签。
- 本次 UpdateImpact 沙尘分支与 M_CoreMorph_SandWave 有意偏离源表现；此前 12 函数／314 包哈希一致为迁入时历史证据，不适用于本反馈后的风墙。金属流、重组时序、其余迁入资产保持。
- `wind-wall-build.log` Development Editor Win64 成功；目标材质编辑与实际 PIE 画面检查进行中。

- 风墙反馈最终自审完成：`wind-wall-final-build.log` 编译成功，`wind-wall-final-author.log` 材质保存成功；`wind-wall-final-pie.log` ReassemblyBatch Success／退出码 0。新增实际实例池三圈高度／间距／无碰撞／暂停断言通过；已有完整飞行自动重组、四时点取消、三时点死亡、自然结束和活动重组退出均通过。
- 画面首轮尘色接近地面，已将圈间隔拉开至 0.65 秒，并增加墙面与浪脊明暗。最终 1280×720 实际截图已检查，呈立起、翻卷的尘幕；观感待用户校验。
- `wind-wall-final-cold-audit.log`／`wind-wall-cold-audit.json`：477 资产冷加载，455 分件布局，Cue Registry 名称准确，无源模块依赖／Redirector；只有 M_CoreMorph_SandWave 与原迁入包哈希不同，其余 313 包保持。源 5195 文件 SHA-256 再次一致。
- 自动编辑器全部退出。本轮改动未提交／push；仍停在第二批反馈，用户在原 L_CoreMorphReassembly 中按 V 或 M 复查，未进入第三批。

## 第三批：八足移动与尾刺（2026-09-13，进行中）

- 用户确认“可以接着迁移”，第二批重组及风墙已验收，授权推进第三批。WIP `49c233e` 保存已验收风墙；当前无目标编辑器进程。
- 核实源沙漠实际使用 GiantScorpionLayout 的 64 足链节点、七段八足 CCD、交替四足接触及尾链 FABRIK；不迁入旧四节点步态或额外网格。隐藏 PhysicalScorpion Actor 改为同头领的移动组件。
- 本批专属主 BT 管理形态入口、追近／转向／攻击决策；尾刺 GA 管理蓄力／刺出／收回和取消，GE 管理攻击状态／冷却及伤害。复用阶段技能集，形态不重授能力。完成后用户自行校验，不自动推进三枪／附着／范围伤害去重的第四批。
- 已迁入仅数值的 75 个关节分组／64 足链节点结构，不重复导入巨蝎网格。ScorpionMovement 保留地面采样、交替四足、限角 CCD；ScorpionCombat 复用现有 301 蝎子分件和 FABRIK 尾链，直接同步本头领根位置。
- 尾刺 GA 接管蓄力、刺出和恢复段；攻击状态与冷却由 GE 提供，刺尖真实 Sweep 命中后通过伤害 GE 修改目标 ASC，删除源 ApplyPointDamage 路径。主 BT 已创建并保存可编辑图（scorpion-ai-author-02.log Success），使用当前阶段近距技能集请求 GA。
- 重组组件在移动接管后只更新固定落点的 Cue，避免沙尘尾效覆盖移动姿态。新增检查地图／蓝图引用写入进行中。初次编辑器图编译缺 AIGraph 依赖，已补为仅 Editor 模块；scorpion-build-03.log 成功。该轮尚未完成实际 PIE，不能视为已验收。
- `scorpion-pie-02.log`：ScorpionBatch Success。真实 PIE 验证一个被专属 Controller 控制的头领／一个 ASC／三个技能，移动时旧沙尘不会拉回主体；支撑足不少于四只、已落足无滑动、腿段长度保持；尾刺 GE 单次扣 25、冷却阻止立即重放、墙体挡伤害。
- 三个出招时段 .3／1.05／1.6 秒取消后均无后续伤害并回收；同三个时点死亡清 GA／足尾求解状态／Cue 并停 Tick。目标销毁取消攻击、阶段切换不改形态或重授技能。实时主 BT 自动完整飞行→重组→追近→转向→尾刺并真实碰到检查目标，活动尾刺退出 PIE 清理通过。
- 1280×720 Scorpion-Walk／Windup／Thrust 截图已查看，八足及尾链装配保持；动态自然程度待用户校验。首轮挡墙失败由未设置为 Movable 的测试墙导致，第二轮已构建真实碰撞并通过，不以首轮日志作最终结果。
- `scorpion-regression.log` 旧六项全部 Success：AdaptiveMotion、EditorPlacement、FlightBatch、ReassemblyBatch、RollMotion、RouteReview。加本轮 ScorpionBatch 共七项通过；未修改武器或人形逻辑，不替代第四批最终三枪／人形总回归。
- `scorpion-math-audit.json` 四个关键求解函数在类型／所有权适配后与源一致；`adaptive-source-audit.json` 源 5195 文件不变。最终资产冷审计进行中，之后交用户第三批观感校验。
- `scorpion-cold-audit.log/json` 最终通过：480 项头领资产，75 关节分组／64 足链节点与源数值逐项一致，专属 Controller／主 BT／第一阶段近距尾刺配置正确；一个阶段技能加两个常驻技能，无源模块依赖或 Redirector。原 313 个分件／特效包继续匹配迁入哈希，SandWave 为此前已验收的有意改造。
- 第三批自审完成，所有自动编辑器退出。源码、Blueprint、主 BT、数值数据和新检查地图的改动未提交／push；源工程保持不变。用户校验入口 L_CoreMorphScorpion（V/M，1/2/3，T/C，P/R/F），当前仅局部避障，完整武器适配／复杂寻路不是本轮验收结果。等待用户反馈，不进入第四批。

### 2026-09-13：第三批反馈——专属蓄力雷爆（进行中）

- 用户明确确认“好的动手”：尾刺蓄力光球、红色闪烁范围圈、击地雷爆范围伤害；不复用现有特效，表现拆为 Gameplay Cue。
- 写入前保存本地检查点 `4c1e26e`，源 UE58Blank 保持只读；本轮属于第三批反馈，不开始第四批武器闭环。
- GA 锁定地面落点与半径，默认蓄力 2 秒；地面碰撞触发一次球形查询，按目标 ASC 去重、静态遮挡检测，现有 TailDamage GE 扣血。Cue 不做伤害。
- 新增 TailEffects 组件和 TailCharge/TailBlast 两个 Cue，分别管理尾尖凝聚、固定地面预警及雷爆电弧；材质从零创建。取消、死亡、重置和 EndPlay 清理。
- 首次冷构建发现 Unity 分组变化令已有两个匿名 `Ease(float)` 重名；将重组文件局部函数改名 `ReassemblyEase`，算法未变，正在重编译。尚未声明通过或交付。

- 第一轮 `thunder-pie.log`：ScorpionBatch 与 ReassemblyBatch 均 Success，包含单 ASC 多分件去重、圈外不伤、锁定落点／半径、取消／死亡／退出 PIE，完整主 BT 运行并截图。
- 编辑器脚本首次给关卡实例写 EditDefaultsOnly 材质被拒绝；材质已在 Blueprint CDO 正确保存，改为冷读实例继承值，并只更新实例可编辑蓄力时长，`thunder-assets-02.log` 保存成功。
- 查看实际截图后，预警圈降低发光以避免偏橙，光球增强并加入击地短时膨胀闪光。`thunder-final-build.log` 冷构建 Succeeded；正在最终 ScorpionBatch 复验，追加静态遮挡保护及 GA 结束后死亡清理检查。

### 本轮交付证据

- `thunder-final-build.log`：Development Editor Win64 Succeeded，无新增 C++ warning/error。
- `thunder-final-pie.log`：ScorpionBatch Success，追加的范围内静态掩体保护、技能结束后死亡清理也通过。`thunder-pie.log` 中 ReassemblyBatch 与首轮 ScorpionBatch 均 Success；最终 Scorpion-Windup／Scorpion-Thunder 截图已查看。
- `thunder-cold-audit.log/json`：485 个头领资产；两个新 Cue Asset Registry 名称完整匹配；三个新材质引用、2 秒蓄力、唯一技能授予、关卡回读通过。0 Redirector、0 源依赖，原 313 包不变。`adaptive-source-audit.json` 确认源 5195 文件 SHA-256 全不变。
- 新增三个专属材质、两个 Cue 蓝图、TailEffects 组件及两个 Native Cue；既有 TailDamage GE 被 GA 用作一次范围伤害。没有新增／复用声音资产，没有改武器或人形逻辑。
- 已关闭本轮全部后台编辑器。结果未提交／push，等待用户在 L_CoreMorphScorpion 校验观感；仍不进入第四批。旧全项目启动材质／缺资源提示及无 NavMesh 的检查地图提示未在本轮扩展修复，不声称全项目日志零警告。

### 尾链折角反馈（2026-09-13，进行中）

- 用户截图 `屏幕截图 2026-09-13 212333.png` 显示蓄力末端两节硬折角；用户确认修复关节限制、整链分担弯曲、刚性尾刺沿弧线朝向。
- 写入前 WIP `750d496` 保存上一轮雷爆全部结果；无编辑器占用。本轮仅具体头领尾链求解与必要回归，不改 GA／GE／Cue 或资产。
- 原末端位置 FABRIK 缺少曲率约束，末节吸收过多修正。改为根据尾根—目标距离求解整链统一曲率倍率，沿原解剖弧线分布每节转角并保留全部刚性节长；普通关节最多 20°，尾刺连接最多 16°。尾链平面跟随实际目标与身体上方向；超出可达域保持合法形态，不强拉折返。
- 外部数值检查涵盖蓄力和 3300／3950 距离击地；当前输入均可准确达到。`tail-arc-build.log` Development Editor Win64 Succeeded；实际 PIE 与逐帧角度／节长断言进行中。此前 source AdvanceTail 数学一致证据为本次反馈前历史，不再适用于新的尾链求解；源项目未修改。

- 最终 `tail-arc-pie.log` ScorpionBatch Success：逐帧采样最大关节 15.413°、尾刺连接 14.362°、节长误差输出 0.000000cm；原范围伤害／去重／遮挡、蓄力／刺出／收回取消、各时点死亡、完整主 BT 与活动 GA 退出检查全部通过。
- 已查看实际 PIE 的 TailArc-Windup／TailArc-Thrust 截图，蓄力尾刺连接的硬折角消除，整链呈连续拱形；当前截图保存在 `Saved/CoreMorphMigration/`。用户主观校验待进行。
- `adaptive-source-audit.json` 源 5195 文件 SHA-256 再次全部一致。未改任何 Content、GA／GE／Cue、八足或人形／武器文件。本轮仅尾链实现、对应测试及 harness 记录，结果未提交／push。后台编辑器已退出；不进入第四批。

### 替换特效遗留审查（2026-09-13）

- 用户认可尾链修正，随后要求检查并删除已替换的多余特效；后续替换应同步清理确认不再使用的资产。
- `coremorph-obsolete-effects-audit.log/json` 冷编辑器审查通过：10 个特效材质／网格均能对应到 Reassembly 数值资产或 Boss TailEffects CDO 的实际字段，且源码确有渲染消费者；3 个 Gameplay Cue 通过正式 Tag 自动发现，不能以普通引用数为零判为闲置。
- 旧平面 SandWave 是原资产原位改造，目标没有另存一套旧效果；Dust／Earth／Impact／StreamMetal／StreamSparks 仍用于重组、碎土和落地余效。扫描未发现目标项目其它 CoreMorph／GiantScorpion／MetalStream／SandWave 遗留候选；头领范围 Redirector 为零，485 资产均保留。
- 可删闲置特效数量 0，因此本轮没有删除资产、修改 C++ 或改动源项目；没有为只读检查建立 checkpoint，也未提交之前的尾链改动。编辑器已退出。本轮专项审查不宣称第四批武器接入已完成。

### 用户要求的完整 CoreMorph 回归（2026-09-13）

- `coremorph-user-regression.log`：通过正式离屏编辑器执行 `TheManTest.Enemy.CoreMorph`，7/7 Success：AdaptiveMotion、EditorPlacement、FlightBatch、ReassemblyBatch、RollMotion、RouteReview、ScorpionBatch。
- 覆盖当前已迁入飞行／三路线／双速翻滚、重组及风墙、八足／尾刺／雷爆、范围去重与遮挡、关节角度／节长、阶段与形态、取消／死亡／活动 GA 退出 PIE。尾链最大转角 15.413°，末端 14.362°，长度误差输出 0.000000cm。
- 当前源码与最近已成功冷编译版本一致，本轮无代码／资产修改，不重复构建；编辑器自动退出。记录当前迁入内容回归通过，不代表尚未实施的第四批三枪／附着弹适配已经完成。
