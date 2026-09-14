# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- 用户反馈编辑态 Capsule 远离主体，已修正并完成自验。Actor 改为主体原点，编辑态相对分件、运行态固定编舞坐标；检查地图 154 分件世界位置变化为 0，根距刚性核心约 0.77 cm。编译、FlightBatch 与 EditorPlacement（移动／旋转／缩放／重构造／实际复制）均通过，待用户复查。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；13.4 秒在粒子释放前停住。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

第一批 Capsule／主体原点反馈已修好，等待用户复查，不得进入第二批。修正前按 harness 建立本地 WIP checkpoint `0516b0d`；本次修正结果未提交／push。外部准备工程 Saved/Review/origin-before.json 保存原布局，origin-fixed.json 证明 154 件世界位置完全不变；origin-build.log 编译通过，origin-validation.log 的 FlightBatch Success，origin-editor-verified.log 的 EditorPlacement Success。源工程未修改，仍为目标 5.7.4／源 5.8.2。自动编辑器均已退出。最终验收才清理临时检查工具，保留有价值的自动回归与源对照备份。
