# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- 用户反馈编辑态 Capsule 远离主体，已修正并完成自验。Actor 改为主体原点，编辑态相对分件、运行态固定编舞坐标；检查地图 154 分件世界位置变化为 0，根距刚性核心约 0.77 cm。编译、FlightBatch 与 EditorPlacement（移动／旋转／缩放／重构造／实际复制）均通过，待用户复查。
- 用户进一步要求全身根据路线／实时状态适配并加入随机性。已接入 FlightMotion：身体朝向与侧倾、翼部收展／扑动、尾部三维轨迹跟随均由实际运动驱动；加入可控平滑随机，旧 TailMotion 合并删除。可摆放 CoreMorphFlightRoute 编辑 Spline，Flight 组件选择路线和速度。编译、AdaptiveMotion／EditorPlacement／实际 PIE FlightBatch 三项通过，待用户校验观感。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；未指定 FlightRoute 时默认 13.4 秒在粒子释放前停住。自定义 Spline 到末端结束或闭合循环。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

第一批全身路线适配和随机变化已实现并自验，等待用户复查，不进入第二批。此前爬升尾部反馈保存为本地 WIP checkpoint `bf812e8`；本轮修改未提交／push。外部准备工程 Saved/Review/adaptive-build.log 编译成功，adaptive-validation.log 的 AdaptiveMotion／EditorPlacement／FlightBatch 三项 Success（含真实 PIE 两条镜像 Spline、循环、悬停、路线销毁和原生命周期）；adaptive-source-audit.json 证明源 5195 文件未变、5 个保留的位置轨迹函数与源一致。FlightMotion 无固定动作时点，FlightPath 只提供旧对照路线位置，TailMotion 已合并删除。FlightRoute 选可摆放的 CoreMorphFlightRoute，Spline 首点起飞；MotionRandomness 默认 .18、0 关闭，MotionSeed 为 0 时复位选新种子、非零可复现。完整操作见 FEAT-081 archive。目标 5.7.4／源 5.8.2 保持不变，自动编辑器已退出；观感交由用户在 L_CoreMorphFlight 校验，最终验收才清理临时检查工具。
