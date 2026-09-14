# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- 用户反馈编辑态 Capsule 远离主体，已修正并完成自验。Actor 改为主体原点，编辑态相对分件、运行态固定编舞坐标；检查地图 154 分件世界位置变化为 0，根距刚性核心约 0.77 cm。编译、FlightBatch 与 EditorPlacement（移动／旋转／缩放／重构造／实际复制）均通过，待用户复查。
- 用户要求尾巴根据爬升自行摆动，已改用实际世界速度平滑驱动尾部波幅，移除固定秒数上下甩尾；路线、翅膀与横向盘旋弧形保留。编译、TailMotion／EditorPlacement／实际 PIE FlightBatch 三项通过，待用户校验观感。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；13.4 秒在粒子释放前停住。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

第一批爬升尾部反馈已完成，等待用户复查，不进入第二批。写入前将已验 Capsule 修正保存为本地 WIP checkpoint `441b533`；尾部修改未提交／push。外部准备工程 Saved/Review/tail-build.log 编译成功，tail-validation.log 的 TailMotion／EditorPlacement／FlightBatch 三项 Success，tail-source-audit.json 证明 5195 源文件未变、7 个轨迹／调度函数及翼部计算保持一致（SourcePose 尾部是本次有意修改）。新 FCoreMorphTailMotion 由 FlightComponent 实际世界位移驱动，暂停／取消／死亡冻结、重播清零；尾部横向转弯仍为固定编排，不是任意路径跟随。目标 5.7.4／源 5.8.2 未变，自动编辑器已退出。截图视口窄，尾部观感交由用户在 L_CoreMorphFlight 校验；最终验收才清理临时检查工具，保留有价值的自动回归与源对照备份。
