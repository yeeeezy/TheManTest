# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress；FEAT-080 暂停。
- 用户已验收第一批飞行及第二批重组／风墙。第三批八足移动、尾刺及主 BT 已完成自审，等待用户校验，不自行进入第四批。
- 目标 UE 5.7.4；源 UE58Blank 5.8.2 保持只读，5195 文件 SHA-256 再次不变；头领无击退／布娃娃。

## 当前完成与待办

- 同一头领拥有 ScorpionMovement／ScorpionCombat，使用 75 组关节枢轴／64 足链节点，复用已迁入 301 蝎子分件；无隐藏移动 Actor 或第二头领。保持支撑足、限角 CCD 和 FABRIK 尾链，旧落地 Cue 不会覆盖移动姿态。
- GA_CoreMorphTailStrike 管理完整蓄力→刺出→收回，GE_CoreMorphAttacking／TailCooldown／TailDamage 管理状态、冷却与一次伤害。尾刺来自现有第一阶段近距技能集；常驻仍为 Flight／Reassemble，共三个技能，阶段与形态独立。
- ACoreMorphAIController Possess 本头领，BT_CoreMorphBoss 管理蝠鲼 Flight→Reassemble 与蝎子追近／转向／阶段技能。移动沿用源的地面探测、身体扫掠和扇形局部避障，不是 NavMesh 全局寻路。
- scorpion-final-build.log Development Editor Win64 成功；scorpion-pie-02.log ScorpionBatch Success：支撑足／腿长／单次 GE 伤害／挡墙／暂停／三个时点取消与死亡／目标销毁／完整实时主 BT／活动 GA 退出。走路、蓄力、刺出截图已查看。
- scorpion-regression.log 旧六项全部 Success：AdaptiveMotion、EditorPlacement、FlightBatch、ReassemblyBatch、RollMotion、RouteReview。合计七项 CoreMorph 检查通过。初轮挡墙失败是测试墙未设 Movable，已修复并由第二轮覆盖。
- scorpion-cold-audit.log/json：480 资产、Controller／BT／技能集、75 分组／64 节点逐项回读通过；无源依赖、无 Redirector，原 313 个迁入包未变。scorpion-math-audit.json 四个关键求解函数保持源数学。
- 第四批三枪／附着／爆炸去重、完整 GAS 战斗及原有人形最终回归尚未开始。等待本轮用户观感反馈。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphScorpion`：V 启用主 BT 完整飞行→重组→蝎子追近和尾刺，M 立即重组；1／2／3 移动灰球目标，T 请求尾刺 GA，C 取消；P 暂停，R 复位，F 相机。
- 原 L_CoreMorphReassembly 与 L_CoreMorphRoutes 保留原入口，不自动启动蝎子战斗。R 是检查复位，不是逆向变形技能。

## 会话交接

WIP `49c233e` 保存已验收第二批，第三批变更未提交／push。所有自动编辑器已退出；源、武器及人形逻辑未改。外部脚本与证据在 `D:/Unreal Projects/CoreMorph57Prep/{Scripts,Saved/Review}`：author_scorpion.py 配数值资产／Blueprint／新地图；audit_scorpion.py 冷审计。不要重跑 implement_scorpion.py 覆盖最终修正，它只是初始生成快照。显式树创建命令为 TheManTest.Authoring.CoreMorphAI；BehaviorTreeEditor／AIGraph 只依赖于 Editor 构建。临时 Review／authoring 工具待最终总验收后清理。下一步等待用户校验第三批。详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
