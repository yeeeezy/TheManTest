# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。FEAT-080 暂停。
- 用户已验收第一批飞行、路线适配和双速侧滚，明确“接着导入吧”；当前第二批变形重组。每批自审后由用户校验，不能自行进入第三批。
- 目标 UE 5.7.4，源 UE58Blank 5.8.2 保持只读；源文件 5195 项 SHA-256 不变。头领不做击退／布娃娃。

## 当前完成与待办

- 第二批已迁入 301 蝎子网格、6 种材质和 7 项必要特效资产，共 314 包，AssetTools 迁移校验通过。
- 一 Actor／ASC／Health 保持；GA_CoreMorphReassemble 驱动解体、金属流落地和蝎子构建，Transforming／Scorpion GE 管理 Tag，GC_CoreMorph_Reassembly 管理实例网格和灯光，完成后保留有界沙尘尾效。
- 解体从当前蝠鲼的真实分件姿态与速度衔接。取消恢复该姿态，完成进入蝎子，复位不回血、不改变战斗阶段、不重授技能；死亡关闭全部查询面及特效。
- reassembly-final-build.log 冷编译成功。reassembly-validation.log 中 AdaptiveMotion／RollMotion／EditorPlacement／FlightBatch／ReassemblyBatch／RouteReview 六项全通过，包含真实 PIE 全飞行→重组→沙尘尾效、四时点取消、三时点死亡、暂停及活动重组退出。
- reassembly-cold-audit.log/json：477 项头领资产冷加载通过，Cue AssetRegistry 名称准确，无源模块依赖、无 Redirector；314 迁移包哈希一致。reassembly-math-audit.json：12 个源构建／金属流／沙尘函数逐项一致。
- 金属流取景已按可见粒子包围盒修正；reassembly-final-pie.log 再次 Success／退出码 0，已查看完整金属流、构建和成型截图。第二批自审完成，自动编辑器已退出，等待用户校验。
- 第三批的八足移动、尾刺和专属 BT 未迁入；第四批三枪／附着／爆炸去重及完整 GAS 战斗仍待后续。

## 用户校验入口

- `/Game/Maps/CoreMorph/L_CoreMorphReassembly`：V 播放完整飞行并自动接重组，M 从当前姿态直接变形，P 暂停，R 复位，F 相机。完成后停在蝎子静态装配。
- 原 `/Game/Maps/CoreMorph/L_CoreMorphRoutes` 保留：1／2／3 路线，Q 慢滚／E 快滚，P 暂停，R 复位，V 播放，F 相机。
- R 是检查复位，不代表逆向变形技能；源实际仅蝠鲼→蝎子。

## 会话交接

写入前 WIP `cc72c0f` 保存已验收第一批；第二批变更尚未提交／push。源和引擎版本未改，未动武器或人形逻辑。外部准备／迁移／审计脚本与日志位于 `D:/Unreal Projects/CoreMorph57Prep/{Scripts,Saved/Review}`。首次 C++ TObjectPtr 推导及 Python 编辑器属性读取错误已修正，以后续成功日志为准。第二批已完成冷编译、资产冷审计、实际 PIE 及画面自审，自动编辑器已退出。下一步等待用户第二批验收，确认后再进入八足移动／尾刺／专属 BT。最终总验收后再清理临时 Review 工具，不提前删除。详情见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
