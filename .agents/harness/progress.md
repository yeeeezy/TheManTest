# 当前工作面板

- Active feature：FEAT-082 大厅角色展示。正式LobbyMap、维修工摆放和预留测试键已完成自动验证，等待用户前台主观校验。
- FEAT-081／080暂停；详见对应archive。

## 正式大厅与展示

- `/Game/Maps/LobbyMap`现在是恢复的FEAT-045选角场景；旧同名空地图已删除。场景依赖保留在SciFiIndustrialBase目录，BuiltData位于`/Game/Maps/LobbyMap_BuiltData`。
- GameInstance原有`LobbyMapName=LobbyMap`已直接接入正式场景；World Settings仍为BP_CharacterSelectGameMode。
- `MaintenanceWorker_LobbyDisplay`唯一实例位于角色焦点原点，初始Relaxed持枪。维修工Lobby目录包含两种Standing、两种Relax和一种Rifle动画。
- 大厅`IMC_CharacterSelect`复用IA_Test／One；按键上方数字1在Relaxed与Rifle间切换。战斗地图的同名测试入口保持原逻辑。
- Development Editor构建成功；实际LobbyMap PIE两次按1切换、Controller、唯一实例和退出哈希检查通过，证据为`lobby-display-input-pie.json`及日志`LOBBY_DISPLAY_INPUT_PIE_OK`。

## 会话交接

操作前checkpoint为`7439d2a`。本轮地图、BuiltData、输入资产、PlayerController C++与harness改动未提交／push。用户接下来可直接打开`/Game/Maps/LobbyMap`并PIE，按键盘上方数字1检查Relax／举枪观感。
