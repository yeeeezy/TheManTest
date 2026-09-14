# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress；FEAT-080 暂停。
- 第一批飞行／双速侧滚已获用户验收。第二批重组已自审，当前处理用户沙尘风墙反馈，每轮由用户校验，不自行进入第三批。
- 目标 UE 5.7.4；源 UE58Blank 5.8.2 只读，5195 文件 SHA-256 复核不变；头领不做击退／布娃娃。

## 当前完成与待办

- 第二批 301 蝎子网格、6 材质及 7 特效资产已迁入，共 314 包；头领目录合计 477 资产。
- GA_CoreMorphReassemble 驱动实际飞行姿态解体、金属流落地和蝎子构建；Transforming／Scorpion GE 管理 Tag；Cue 管理临时实例网格和灯光。一 Actor／ASC／Health，取消恢复姿态、完成不回血、不改战斗阶段、不重授技能。
- 当前沙尘改为三圈高约 20 米的风墙，间隔 0.65 秒等速向外扩散；包含接地墙身、顶部浪脊及后卷层，材质向上流动并增强明暗。1152 个无碰撞实例，随组件时钟暂停、取消和死亡清理。
- `wind-wall-final-build.log` 冷编译成功；`wind-wall-final-pie.log` ReassemblyBatch Success／退出码 0。实际 PIE 检查三圈高度和间距、暂停、全飞行→变形→尾效结束、四时点取消、三时点死亡、活动重组退出。已看本轮渲染截图。
- `wind-wall-final-cold-audit.log`／`wind-wall-cold-audit.json` 冷审计通过：477 资产、455 分件和 Cue Registry 正确，无源依赖／Redirector；仅 M_CoreMorph_SandWave 哈希变化，其余 313 包一致。原 12 函数／314 包完全一致属于风墙反馈前历史证据；本轮有意改造 UpdateImpact 沙尘分支和 SandWave 材质。
- 第三批八足移动／尾刺／专属 BT 以及第四批完整 GAS 战斗、三枪／附着／爆炸去重尚未迁入。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphReassembly`：V 完整飞行并自动重组，M 立即变形，P 暂停，R 复位，F 相机；完成后静态蝎子。
- `/Game/Maps/CoreMorph/L_CoreMorphRoutes`：1／2／3 路线，Q 慢滚／E 快滚。R 仅检查复位，不是逆向变形技能。

## 会话交接

WIP `c34a7bc` 保留本轮风墙修改前完整第二批；当前风墙反馈修改未提交／push。源、武器和人形逻辑未改。外部脚本与日志位于 `D:/Unreal Projects/CoreMorph57Prep/{Scripts,Saved/Review}`，当前脚本 author_wind_wall.py／audit_wind_wall.py；后者只豁免已知 SandWave 哈希变更，其余 313 包严格匹配。自动编辑器已全部退出，等待用户风墙／第二批校验。临时 Review 工具保留到最终总验收后清理。详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
