# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress；FEAT-080 暂停。
- 用户已验收第一批飞行及第二批重组／风墙；第三批八足、尾刺、主 BT 的反馈“专属蓄力雷爆”已实现并完成自审，等待用户观感校验，不自行进入第四批。
- 目标 UE 5.7.4；源 UE58Blank 5.8.2 保持只读，5195 文件 SHA-256 不变；头领无击退／布娃娃。

## 当前完成与待办

- 同一头领拥有 ScorpionMovement／ScorpionCombat／TailEffects，75 组关节／64 足链节点驱动原 301 蝎子分件。八足支撑、限角 CCD 和 FABRIK 尾链保留；阶段与形态独立，仍为 Flight、Reassemble、第一阶段 Near TailStrike 三份唯一技能授予。
- GA_CoreMorphTailStrike 默认 2 秒蓄力，锁定目标脚下落点及 1000cm 半径。TailCharge Cue 管尾尖凝聚光球／粒子与固定红色闪烁预警 Decal；击地添加 TailBlast Cue，生成电弧、冲击圈、膨胀闪光和短时光照。三个材质从零制作，不复用现有特效。无新增声音资产。
- GA 使用同一锁定范围做一次球形查询，按 ASC 去重并检查静态遮挡，既有 GE_CoreMorphTailDamage 扣血；Cue 不做伤害。途中挡墙不引爆。取消、死亡、重置、EndPlay 清理 Cue／组件；正常雷爆 1.25 秒结束，无计时器。
- thunder-final-build.log Development Editor Win64 Succeeded；thunder-final-pie.log ScorpionBatch Success：多分件一次伤害、圈外不伤、静态遮挡、暂停、蓄力／刺出／收回取消、各阶段及 GA 结束后死亡清理、完整主 BT、活动 GA 退出 PIE。thunder-pie.log ReassemblyBatch 及首轮 ScorpionBatch 均 Success。最终蓄力／雷爆截图已查看。
- thunder-cold-audit.log/json：485 资产、两个新 Cue 的 Asset Registry GameplayCueName、CDO 材质引用／2 秒蓄力、单阶段技能、地图、源依赖和 Redirector 均通过。原 313 个迁入包未变；源 5195 文件哈希再次不变。
- 主 BT 仍为飞行→重组→追近／转向／请求阶段技能；仅局部地面避障，无全局 NavMesh 路径。第四批三枪／附着／爆炸去重、完整 GAS 战斗与人形最终回归尚未开始。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphScorpion`：V 启用完整主 BT；M 立即重组后战斗；1／2／3 移动灰球，T 请求尾刺 GA，C 取消；P 暂停，R 复位，F 相机。
- 原 L_CoreMorphReassembly、L_CoreMorphRoutes 保留原入口，不自动启动蝎子战斗。R 是检查复位，不是逆向变形技能。

## 会话交接

WIP `4c1e26e` 保存本轮修改前的第三批；蓄力雷爆结果未提交／push。所有自动编辑器已退出。源、武器及人形逻辑未改。外部脚本／证据位于 `D:/Unreal Projects/CoreMorph57Prep/{Scripts,Saved/Review}`，最新审计脚本 audit_thunder.py；author_thunder.py 首轮已创建 5 个新资产，不能直接重跑，实例 EditDefaultsOnly 材质继承由 finish_thunder_assets.py 冷读确认。refine_thunder_assets.py 已执行。不要重跑 implement_scorpion.py、implement_thunder.py、implement_tail_fx.py、update_thunder_tests.py 等初始生成脚本覆盖最终文件。编译首次 Unity 重名已通过将重组局部 Ease 改名 ReassemblyEase 修复，算法未变。临时 Review／authoring 工具待最终总验收后清理。下一步等待本轮用户反馈；详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
