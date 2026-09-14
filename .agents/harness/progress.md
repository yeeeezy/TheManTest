# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 当前进行中：用户要求加速前大力挥翼与随机轻柔／快速翻转卷翼。加速意图与先发力再提速代码、测试已准备，尚未编译／PIE；“翻转”是侧滚还是水平掉头待用户说明，翻转未实现。用户前台编辑器 PID 33624 仍开着，已请求保存关闭，未自行关闭。

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

最新请求是加速前发力、随机轻柔／快速翻转且翅膀卷起。三路线成果已先保存为 WIP `3938497`；当前 FlightMotion／FlightComponent／测试的加速意图修改未编译、未提交／push，不能宣称已生效。Motion.AccelerationIntent 驱动 PowerStroke，Spline 先显现挥翼再按推力累积速度，源位置路线保留并前瞻 .4 秒驱动发力。翻转方向待用户回复（侧滚／水平掉头），翻转尚未实现。用户启动的前台 TheManTest 编辑器 PID 33624 未关闭，已异步请求保存关闭以冷编译，不能擅自终止。后续完成机动卷翼后跑 AdaptiveMotion／FlightBatch／RouteReview，并更新验证证据。上一轮三路线实际 PIE 验证见 routes-validation.log；新地图 L_CoreMorphRoutes 的 1／2／3 自动切换入口继续保留。源和目标引擎不变，不进入第二批。
