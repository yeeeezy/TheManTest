# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 新增三路线检查地图 `/Game/Maps/CoreMorph/L_CoreMorphRoutes`：Play 自动起飞，1 缓弯爬升（20 秒）／2 左右 S 弯（22 秒）／3 盘旋俯冲（27 秒），数字键切换重播；P 暂停、R 复位、V 播放、F 相机。编译、保存重开、RouteReview 实际 PIE 三条完整播放和原 FlightBatch 回归均通过，等待用户评价自然程度。

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- 用户反馈编辑态 Capsule 远离主体，已修正并完成自验。Actor 改为主体原点，编辑态相对分件、运行态固定编舞坐标；检查地图 154 分件世界位置变化为 0，根距刚性核心约 0.77 cm。编译、FlightBatch 与 EditorPlacement（移动／旋转／缩放／重构造／实际复制）均通过，待用户复查。
- 用户进一步要求全身根据路线／实时状态适配并加入随机性。已接入 FlightMotion：身体朝向与侧倾、翼部收展／扑动、尾部三维轨迹跟随均由实际运动驱动；加入可控平滑随机，旧 TailMotion 合并删除。可摆放 CoreMorphFlightRoute 编辑 Spline，Flight 组件选择路线和速度。编译、AdaptiveMotion／EditorPlacement／实际 PIE FlightBatch 三项通过，待用户校验观感。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；未指定 FlightRoute 时默认 13.4 秒在粒子释放前停住。自定义 Spline 到末端结束或闭合循环。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

按用户要求提供三条可直接观看的路线，新增 L_CoreMorphRoutes，等待用户观感反馈，不进入第二批。全身适配成果已在写入前保存为 WIP checkpoint `c8b8e0e`；本轮地图／Review 控件／测试／文档未提交或 push。外部准备工程 Saved/Review/routes-tests-build.log 编译通过，routes-author-02.log 和 routes-authored.json 验证保存重开／全部控制点，routes-validation.log 的 RouteReview 与 FlightBatch 两项 Success。三条路线真实世界 Tick 分别完整飞行 20／22／27 秒并正确结束，切换复用同一个头领和 GA。Play 自动开始第 1 条，1／2／3 重播切换；原 L_CoreMorphFlight 保持手动 V 与原对照路线。初次直接 World 复制触发 Python 引用导致的编辑器内存清理断言，已改用 NewLevelFromTemplate 并释放 Python 引用后解决。源项目／目标版本／运动求解器与头领正式资产未改，自动编辑器已退出。新地图属于最终验收后清理的临时检查工具。
