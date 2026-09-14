# 当前工作面板

- Active feature：FEAT-082 大厅角色展示。新版选角大厅已从Git删除前记录恢复；角色展示与举枪接口完成自动验证，等待用户在恢复地图中主观校验。
- FEAT-081／080暂停；详见对应archive。

## 最新恢复

- 当前/Game/Maps/LobbyMap确认是弃用旧空地图，全部Git历史只有这一版本。
- 真正新版大厅是/Game/Maps/SciFiIndustrialBase/Maps/SciFiIndustrialBase，曾在2026-08-01提交a03f30d中与整套资源一并删除；已从a03f30d^恢复328个文件，未覆盖旧LobbyMap。
- restored-character-select-map验证通过：328资产全部加载，地图1976 Actor，含CharacterSelectCameraSwitcher与4个CineCameraActor。Autosaves／Backups／回收站未发现其他LobbyMap版本。
- 当前只恢复地图内容，没有修改GameInstance的LobbyMapName；旧流程可能仍打开弃用LobbyMap，后续是否接到新版地图等待用户决定。

## 大厅角色展示

- 维修工Lobby目录共40资产，两种空手Standing、两种持枪Relax、一种Rifle举枪。
- BP_MaintenanceWorker_Lobby使用正式RepairGun模型／材质，Relaxed／Rifle各有校准偏移。
- ALobbyCharacterBase提供SetWeaponReady(false/true)与IsWeaponReady，所有大厅人物可复用。
- 冷编译、实际PIE反复切换、退出清理和冷资产审计通过，详见[FEAT-082](archive/FEAT-082-lobby-character-presentation.md)。

## 会话交接

恢复前checkpoint197788a保存举枪批次。恢复的Content/Maps/SciFiIndustrialBase整目录当前未提交／push；不要清理。地图验证证据在D:/Unreal Projects/CoreMorph57Prep/Saved/Review/restored-character-select-map.*。所有编辑器已退出。下一步用户可打开/Game/Maps/SciFiIndustrialBase/Maps/SciFiIndustrialBase确认。不要将弃用LobbyMap误当新版；若用户确认接入正式流程，再修改LobbyMapName／对应GameMode并完整验证。
