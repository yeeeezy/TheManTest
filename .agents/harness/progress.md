# 当前工作面板

- Active feature：FEAT-081 蝠鲼／蝎子头领分批迁移，in_progress。
- 用户授权按批迁入，每批自审后由用户校验。本轮仅外观与飞行，下一批等用户反馈。
- FEAT-080 暂停；起点 58817b1；其他遗留工作见对应 archive。

## 当前完成与待办

- 本轮已实现：加速前大力挥翼、慢／快两种沿飞行方向侧滚一圈；翼根／翼尖由实际旋转驱动弹性滞后、卷曲与回摆。随机侧滚基于种子、飞行距离和状态；Q 慢滚／E 快滚可直接对比。冷编译及 AdaptiveMotion／RollMotion／EditorPlacement／FlightBatch／RouteReview 五项全通过；1280×720 离屏三路线复查也通过，自动编辑器已退出，等用户校验观感。

- 三路线检查地图 `/Game/Maps/CoreMorph/L_CoreMorphRoutes`：Play 自动起飞，1 缓弯爬升／2 左右 S 弯／3 盘旋俯冲，数字键切换重播；Q 慢滚、E 快滚、P 暂停、R 复位、V 播放、F 相机。原标称约 20／22／27 秒之外，现在还包含起步发力与加速耗时。编译、保存重开、实际 PIE 完整路线和原飞行回归均通过，等待用户评价自然程度。

- 已完成源项目外部快照、5.7 准备工程、154 蝠鲼网格与 5 材质重建及 AssetTools 迁入。
- 第一批外观与飞行已完成自验和审查，等待用户校验。单头领、单 ASC／Health、飞行 GA、形态 GE、Blueprint 和检查地图已就位。
- 用户反馈编辑态 Capsule 远离主体，已修正并完成自验。Actor 改为主体原点，编辑态相对分件、运行态固定编舞坐标；检查地图 154 分件世界位置变化为 0，根距刚性核心约 0.77 cm。编译、FlightBatch 与 EditorPlacement（移动／旋转／缩放／重构造／实际复制）均通过，待用户复查。
- 用户进一步要求全身根据路线／实时状态适配并加入随机性。已接入 FlightMotion：身体朝向与侧倾、翼部收展／扑动、尾部三维轨迹跟随均由实际运动驱动；加入可控平滑随机，旧 TailMotion 合并删除。可摆放 CoreMorphFlightRoute 编辑 Spline，Flight 组件选择路线和速度。编译、AdaptiveMotion／EditorPlacement／实际 PIE FlightBatch 三项通过，待用户校验观感。
- Development Editor 编译、资产冷加载、`CoreMorph.FlightBatch` 实际 PIE 均通过；三枪基线／切枪／准星／范围爆炸／人形死亡 5 项现有回归通过。
- 检查地图 `/Game/Maps/CoreMorph/L_CoreMorphFlight`：V 播放、P 暂停、R 复位、F 相机；未指定 FlightRoute 时默认 13.4 秒在粒子释放前停住。自定义 Spline 到末端结束或闭合循环。蝎子与重组尚未迁入。
- 目标保持 5.7.4；源 5.8.2 保持只读。头领不做击退／布娃娃。
- 详见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。

## 会话交接

用户已明确侧滚含义及保存关闭编辑器；无需再次追问。WIP `6add0f3` 保存上轮未编译发力代码；本轮实现双速侧滚、耦合翼部惯性与 Q／E 预览。roll-build.log 编译成功，roll-validation.log 五项全部 Success，含实际 PIE 两种侧滚取消、死亡、三路线机动和退出清理。源 5195 文件校验不变，五个参考位置函数仍一致。针对旧截图过窄，只在 RouteReview 自动测试临时固定渲染表面为 1280×720，结束恢复；roll-review-build.log 编译成功、roll-review-visual.log 再次 Success／退出码 0。已查看慢／快滚截图；快速图存在运动模糊，动态自然程度等用户评价。自动编辑器已退出，当前改动未提交／push；源和目标引擎不变，未修改 Content 资产，不进入第二批。下一步等待用户在 L_CoreMorphRoutes 观感校验；最终验收后再清理临时 Review 工具。
