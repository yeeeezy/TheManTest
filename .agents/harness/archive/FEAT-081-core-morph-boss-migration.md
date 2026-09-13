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

三枪最终适配尚未完成：现有枪口遮挡与附着弹偏向 skeletal mesh；爆炸需按头领身份去重并选择真实表面检查遮挡。阶段技能重复授予风险、BT 取消边界在战斗批次处理。第一批不宣称蝎子／重组／完整战斗或三枪命中已验收。
