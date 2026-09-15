# 当前工作面板

- Active feature：FEAT-082大厅展示；FEAT-080/081暂停。
- 大厅地图326个专属资源已从`/Game/Maps/SciFiIndustrialBase`迁至`/Game/Maps/Lobby/{Blueprint,Meshes,Materials,Textures,Audio}`。
- 地图本体已归入具体目录：`Maps/Lobby/{LobbyMap,LobbyMap_BuiltData}`与`Maps/Test/{TestMap,TestMap_HLOD0_Instancing}`；Maps根目录资产为0。
- 配置、GameInstance及41处测试路径已同步；冷构建成功，冷启动两图和Lobby/Test实际PIE链路通过。

## 会话交接

- Lobby 1985 Actor，Test 77 Actor；Test新路径有141 ExternalActor与5 ExternalObject，旧外部对象目录和四个旧根路径均已清理。
- 最终地图嵌套结果尚未提交或push。后续若继续整理Enemy／Character，应先建立独立功能条目并按所有权逐批验证。
