# 当前工作面板

- Active feature：FEAT-082大厅展示；FEAT-080/081暂停。
- 最新灯光已保存LobbyMap：环境灯强度为原2.5%，固定EV100=3；顶部250lm柔白RectLight、画面左750lm红光、右500lm白光。场景Outliner的LobbyLighting文件夹可调三灯；侧光只照人物/枪械Channel1，顶光Channel0保留较暗落地光区。
- 三枪独立动画、两把80%大厅副本、握持与食指微调保持。WEAPON进入Rifle/近景，CHARACTER随机Relax/远景；只有hover高亮。姿态及挂点0.5秒混合，相机0.7秒。

## 验证与交接

- 冷启动回读及实际LobbyMap PIE验证通过：三灯强度/照明通道、人物与两个枪组件通道、固定曝光均持久化；三枪远近镜头共6张画面已查看，验证退出后地图SHA256不变。Saved/Codex/lobby-lighting-validation.json ok=true，lobby-lighting-validation.log LOBBY_LIGHTING_OK，LobbyLighting-Review.jpg为汇总。没有C++/蓝图资产修改，无需重新编译；后台编辑器退出。
- checkpoint7baac17保存此前0.5秒混合；本轮产品仅Content/Maps/LobbyMap.umap及harness。原Actor变换逐项不变，没有代码、正式武器、BP或材质修改，未最终提交/push。
- 场景原有多DirectionalLight优先级及VSM队列警告记录见archive；本次没有额外处理。下一步根据用户对光比/背景暗度的视觉反馈调整。
