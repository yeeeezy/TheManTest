# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress；FEAT-080 暂停。
- 用户已验收第一批飞行及第二批重组／风墙；第三批最新尾链修正已获用户认可；替换特效遗留审查完成，没有可删的闲置特效。第四批尚未实施。
- 目标 UE 5.7.4；源 UE58Blank 5.8.2 保持只读，5195 文件 SHA-256 不变；头领无击退／布娃娃。

## 当前完成与待办

- 同一头领拥有 ScorpionMovement／ScorpionCombat／TailEffects，75 组关节／64 足链节点驱动原 301 蝎子分件。八足支撑与限角 CCD 保留；尾链改为基于目标距离的整链曲率求解；阶段与形态独立，仍为 Flight、Reassemble、第一阶段 Near TailStrike 三份唯一技能授予。
- GA_CoreMorphTailStrike 默认 2 秒蓄力，锁定目标脚下落点及 1000cm 半径。TailCharge Cue 管尾尖凝聚光球／粒子与固定红色闪烁预警 Decal；击地添加 TailBlast Cue，生成电弧、冲击圈、膨胀闪光和短时光照。三个材质从零制作，不复用现有特效。无新增声音资产。
- GA 使用同一锁定范围做一次球形查询，按 ASC 去重并检查静态遮挡，既有 GE_CoreMorphTailDamage 扣血（击地时基础25 × 当前 GetDamageMultiplier()，每次雷爆只读取一份数值）；Cue 不做伤害。途中挡墙不引爆。取消、死亡、重置、EndPlay 清理 Cue／组件；正常雷爆 1.25 秒结束，无计时器。
- thunder-final-build.log Development Editor Win64 Succeeded；thunder-final-pie.log ScorpionBatch Success：多分件一次伤害、圈外不伤、静态遮挡、暂停、蓄力／刺出／收回取消、各阶段及 GA 结束后死亡清理、完整主 BT、活动 GA 退出 PIE。thunder-pie.log ReassemblyBatch 及首轮 ScorpionBatch 均 Success。最终蓄力／雷爆截图已查看。
- thunder-cold-audit.log/json：485 资产、两个新 Cue 的 Asset Registry GameplayCueName、CDO 材质引用／2 秒蓄力、单阶段技能、地图、源依赖和 Redirector 均通过。原 313 个迁入包未变；源 5195 文件哈希再次不变。
- 主 BT 仍为飞行→重组→追近／转向／请求阶段技能；仅局部地面避障，无全局 NavMesh 路径。第四批三枪／附着／爆炸去重、完整 GAS 战斗与人形最终回归尚未开始。

- 最新尾链修正：普通关节上限 20°、尾刺连接上限 16°，全链节长保持；tail-arc-build.log Succeeded，tail-arc-pie.log ScorpionBatch Success。采样最大转角 15.413°／末端 14.362°，节长误差 0.000000cm；伤害、遮挡、取消、死亡及 PIE 退出回归通过。TailArc-Windup／Thrust 截图已查看，源 5195 文件再次未变。本轮没有资产变更。

- 替换特效遗留审查：coremorph-obsolete-effects-audit.log/json 通过，10 个特效材质／网格及 3 个 Cue 均仍在使用；旧 SandWave 为原位改造，无额外旧资产。闲置候选／Redirector 均为 0，未删除资产。后续替换需同步清理确无消费者的旧资源。

- 最新完整回归：coremorph-user-regression.log 中 TheManTest.Enemy.CoreMorph 全 7 项 Success，含三路线、完整重组及最新尾链雷爆；自动编辑器已退出。本轮无代码／资产修改。

- 最新强度倍率接入：tail-strength-final-build.log Succeeded，tail-strength-pie.log ScorpionBatch Success；基础25、x1.2扣30、封顶x2扣50，多分件去重及原战斗清理回归通过。新增敌人伤害必须接倍率的长期规则已写入 harness AGENTS.md／arch10，并纠正 EnemyBase.h 的阶段相关旧注释。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphScorpion`：V 启用完整主 BT；M 立即重组后战斗；1／2／3 移动灰球，T 请求尾刺 GA，C 取消；P 暂停，R 复位，F 相机。
- 原 L_CoreMorphReassembly、L_CoreMorphRoutes 保留原入口，不自动启动蝎子战斗。R 是检查复位，不是逆向变形技能。

## 会话交接

WIP `cba8842` 保存强度倍率接入前的尾链修正及全套回归。当前尾刺倍率接入结果未提交／push，后台编辑器已退出。用户认可尾链；闲置特效审查为0，无资产删除。尾刺现在击地时使用基础伤害 × 当前强度倍率，同一次雷爆统一数值并按 ASC 去重；阶段只切技能集，形态不重置倍率。AGENTS.md 已记录用户要求的长期敌人伤害接入规则，新增／迁入技能务必遵守。最新证据 `D:/Unreal Projects/CoreMorph57Prep/Saved/Review/tail-strength-{final-build,pie}.log`；ScorpionBatch Success，x1.2扣30／x2扣50。无 Content 或源工程改动；公共 EnemyBase.h 仅修正旧注释。外部 implement_tail_strength.py 及以前实现脚本均已执行，不要重跑。第四批三枪／附着弹适配仍待实施；临时 Review 工具保留至总验收后清理。详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
