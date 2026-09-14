# 当前工作面板

- Active feature：FEAT-081 CoreMorph 头领迁移，in_progress；FEAT-080 暂停。
- 第一批飞行、第二批重组／风墙、第三批八足／尾刺雷爆／尾链修正已获用户认可，尾刺强度倍率已接通。用户最新要求 Manta 核心导弹轰炸，本轮执行此技能。
- 目标 UE 5.7.4；源 UE58Blank 5.8.2 只读，5195文件SHA-256再次一致。唯一Actor／ASC／Health，不做人形击退或布娃娃。

## 当前实现与验证

- 第一阶段 Near=TailStrike、Far=MissileBarrage；DefaultAbilities=Flight／Reassemble，共4份唯一技能。阶段决定技能池，强度决定倍率，形态决定激活条件。
- Manta 默认4枚导弹，逐枚从实时能量核心发射，弧线飞向提前锁定的随机地面区域；红圈间留空。每枚发射时快照基础20 × 当前强度倍率；GE范围伤害按ASC去重及静态遮挡。取消／变形／死亡／EndPlay清理。
- 专属 MissileBarrage Cue 管新制作的导弹、尾焰、红圈、爆炸与灯光；4个新材质，无新增声音。主BT在飞行期间并行请求Far，结束等待当前技能再重组；手动M立即取消导弹。
- missile-clean-build.log Succeeded；missile-pie.log MissileBatch Success，4落点、实时核心发射、x1.2快照24、多分件去重、暂停／取消／变形／死亡／活动GA退出通过。
- missile-full-regression.log 首轮8项CoreMorph全Success。截图发现原有蝎子后R复位身体仍隐形，已修复 Flight::ResetPreview 显隐／溶解／查询碰撞；补充断言和主BT并行远程、基础20／封顶40检查。missile-final-build.log Succeeded；missile-final-pie.log 的 FlightBatch／MissileBatch／ReassemblyBatch／ScorpionBatch 全Success，含基础20／增强24／封顶40、复位可见性及正式BT并行远程。最终飞行／爆炸截图已查看。
- missile-cold-audit.log/json 通过：490头领资产、4份唯一技能、4个Cue注册准确，新材质／地图引用正确，Redirector／源依赖0，红圈材质冷编译正常。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphMantaCombat`：V闭环飞行，T远程轰炸，C取消，M变形，P暂停，R复位，F相机。
- `/Game/Maps/CoreMorph/L_CoreMorphScorpion`：V正式主BT（飞行+远程→重组→近战）；M立即重组；1／2／3移动灰球，T尾刺，C取消，P／R／F。
- 原重组／三路线地图保留。R是检查复位，不是逆向变形技能。Review工具保留至总验收后清理。

## 会话交接

WIP 84ec77a 保存本轮前已验收内容；当前Manta实现未提交／push。外部脚本位于 D:/Unreal Projects/CoreMorph57Prep/Scripts；implement_missiles、implement_missile_fx、integrate_missiles、author_missiles 已执行，不要盲目重跑。实现、PIE和冷资产审查均完成，待用户在新地图校验观感；后台编辑器已退出。最后仅恢复Tags头文件中文注释，missile-handoff-build.log 冷构建 Succeeded；已验证逻辑不变。源哈希已再次确认5195文件未变。第四批三枪／附着弹适配仍待实施。详情见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
