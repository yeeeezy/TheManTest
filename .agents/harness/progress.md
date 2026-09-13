# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；13.4 秒在粒子释放前停住。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

已完成第一批自验，等待用户手动确认，不得标记用户验收或开始第二批。准备脚本及日志位于 D:/Unreal Projects/CoreMorph57Prep，最终证据为 Saved/Review/flight-pie-final.log、cold-audit.json、source-final-audit.json；目标截图在 Saved/CoreMorphMigration。源 5195 个快照文件哈希一致；8 个飞行函数体一致。自动编辑器已退出，无项目升级、提交或 push。后续先处理用户第一批反馈，再推进重组、蝎子及 GAS 战斗。既有玩家材质／AimIK 警告详见 archive，未在本批修改。
