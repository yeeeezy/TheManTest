# FEAT-078 — Combat HUD 与玩家枪械弹药

**创建日期：** 2026-09-01
**状态：** in_progress

## 2026-09-01 session251 — R键 Gameplay Tag 换弹

- 新增 `/Game/Core/Input/Actions/IA_Reload`，在 `IMC_Default` 映射键盘 R，并配置到 `BP_TheManPlayerController.ReloadAction`。
- `AFPSCharacterBase` 绑定 Reload InputAction；按下后只发送 `Input.Weapon.Reload` Gameplay Event，不直接修改弹药。
- 新增共享 `UGA_Reload` 与 `/Game/Weapons/_Shared/GAS/Abilities/BGA_Reload`；Ability 从当前装备取得 `AFirearm`，使用 `CanReload()` 门控并调用 `ReloadMagazine()`。
- `AFirearm` 新增独立 `ReloadAbilityClass/ReloadHandle`，与主/副射击一样在装备时授予、卸下或切角色时可靠回收；`BP_RepairGun` 已配置 `BGA_Reload`。
- 当前为即时换弹，不含换弹动画或延时；后续可在 Ability 内加入 Montage/AbilityTask 后于完成点补弹。
- Development Editor 冷构建成功；相关蓝图在 Unreal 内编译保存成功；`TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success，验证满弹按换弹不扣备用弹夹、空弹经真实 Tag/Ability 链补至30、备用弹夹3→2且 HUD 即时更新。

## 2026-09-01 session271 — RepairGun 成功射击音效

- 外部 WAV 以 RepairGun 专属资源导入 `/Game/Weapons/RepairGun/Audio/S_RepairGun_Fire`，并绑定 `BP_RepairGun.FireSound`；蓝图编译保存与冷启动引用回读通过。
- SoundWave 为 1.654s、Stereo、96kHz，武器 Volume/Pitch Multiplier 保持 1.0。
- 播放点继续位于 `UGA_Shoot` 成功 `ConsumeRound` 之后，因此空弹不会播放实弹枪声；AmmoLifecycle 回归通过。

## 2026-09-01 session272 — 当前弹匣为空时播放 Dry Fire

- 新增共享枪械配置 `DryFireSound/DryFireSoundVolumeMultiplier/DryFireSoundPitchMultiplier`，与成功射击 `FireSound` 完全分离。
- RepairGun 专属资产 `/Game/Weapons/RepairGun/Audio/S_RepairGun_DryFire` 已导入并绑定，时长 0.392s、Stereo、192kHz，Volume/Pitch=1.0。
- `UGA_Shoot` 空弹分支只播放 Dry Fire 并立即结束，所有实弹反馈继续被门控。Development Editor 构建成功，AmmoLifecycle 新增资产绑定断言并通过。

## 2026-09-01 session250 — 中心准星缩小20%

- 中心空心准星半径从57.6px缩小为46.08px，线宽从2.5px缩小为2px，圆周分段从96调整为80。
- 圆心、血量与弹药布局均不变。
- Development Editor 构建与 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；截图已更新。

## 2026-09-01 session249 — HUD 视觉中心对齐

- 用户截图复核确认字体基线对齐仍有视觉错位；改为统一视觉中心线。
- 大号血量与当前子弹共用中心线，小号 `+` 与备用弹夹数分别按自身测量高度居中到该中心线。
- Development Editor 构建与 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；截图已更新。

## 2026-09-01 session248 — HUD 字体基线对齐

- 修正血量图标、血量值、当前子弹与备用弹夹因字号不同而出现的纵向错位。
- 四个文本元素统一使用 Slate 字体度量计算同一条基线；横向位置、字号和准星不变。
- Development Editor 构建与 `TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success；截图已更新并用系统图片查看器打开。

## 2026-09-01 session247 — 参考图血量与弹药布局

- 按用户指定参考图新增底部当前血量显示；只显示当前值，不显示最大血量。
- 弹药区改为大号当前子弹数与右侧小号备用弹夹数，不再绘制弹夹容量或“弹夹”文字。
- Controller 通过 PlayerState ASC 的 Health/MaxHealth 属性变化委托更新 HUD，切角色时正确解绑重绑，不使用 UI Tick。
- Development Editor 构建成功；`TheManTest.Player.CombatHUD.AmmoLifecycle` 1/1 Success，包含默认血量100与受伤后即时更新至75的断言；最终截图恢复100血量，弹药为30、备用弹夹2。
- 最新截图：`Saved/Screenshots/WindowsEditor/TMT_CombatHUD.png`。

## 2026-09-01 session246 — 准星再次放大1.8倍

- 按用户反馈将准星半径从32px乘1.8调整为57.6px（直径115.2px）；线宽提高至2.5px，圆周分段提高至96。
- 弹药布局与所有 gameplay/UI 事件逻辑不变。
- Development Editor 构建与弹药生命周期专项自动化成功；新截图已覆盖 `TMT_CombatHUD.png`。

## 2026-09-01 session245 — 准星放大四倍

- 按用户前台反馈将空心圆半径从8px提高到32px（直径16→64px，四倍）；线宽从1.5px提高到2px，圆周分段从32提高到64以保持平滑。
- 弹药两行布局、颜色、透明度和事件驱动逻辑不变。
- Development Editor 构建与 `TheManTest.Player.CombatHUD.AmmoLifecycle` 复跑成功；`TMT_CombatHUD.png` 已覆盖为放大后的准星截图。

## 目标

- `ATheManPlayerController` 管理本地战斗 HUD 生命周期。
- HUD 显示屏幕中心空心准星，以及右下角两行弹药信息。
- 玩家枪械拥有真实的当前弹药、弹匣容量和备用弹夹数量，并以事件驱动 UI。

## 2026-09-01 session244 — 第一阶段实现

- 新增原生 `UCombatHUDWidgetBase`。内部 Slate 叶控件按实际视口尺寸绘制半径8px、线宽1.5px的白色空心圆；右下角第一行显示 `CurrentAmmo / MagazineCapacity`，第二行显示 `弹夹 SpareMagazineCount`。Widget 为 Hit Test Invisible，不使用 Tick。
- `ATheManPlayerController` 仅在本地创建 HUD；在 BeginPlay/OnPossess/OnUnPossess 中绑定或解绑 Pawn 的 EquipmentManager，并在切枪后切换 Firearm 弹药委托。
- `AFirearm` 新增 `MagazineCapacity=30`、`CurrentAmmo=30`、`SpareMagazineCount=3`、`ConsumeRound`、`ReloadMagazine`、`CanFire/CanReload` 和 `OnAmmoChanged`。
- `UGA_Shoot` 在任何开火反馈前调用 `ConsumeRound`；空弹时不生成子弹，也不播放蒙太奇、音效、Niagara、震屏或后坐力。
- `UEquipmentManagerComponent` 新增 `OnCurrentEquipmentChanged`，首次装备和滚轮切枪均广播；HUD 无 Tick 轮询。

## 验证

- `TheManTestEditor Win64 Development`：Success（session244/session245）。
- `TheManTest.Player.CombatHUD.AmmoLifecycle`：1/1 Success。验证默认30/30与备用弹夹3、扣弹同步为29、换弹恢复30且备用弹夹变2。
- 含 UI 的视口截图：`Saved/Screenshots/WindowsEditor/TMT_CombatHUD.png`。中心圆与右下角两行弹药均可见。
- 既有 `FramingCapture` 仍因本功能之前的 FOV=77 与 GripPoint/GripPoint1 断言不一致失败；与 Combat HUD 无关。

## 待办

- 用户前台确认血量/弹药参考图布局与留白是否符合主观观感。
- 换弹输入与换弹动画尚未接入；当前仅提供可调用的 `ReloadMagazine()` 状态接口。

## 2026-09-01 session275 — 射击回归覆盖补充

- `AmmoLifecycle` 新增通用弹体撞击 Gameplay Cue 的共享声音/Tag，以及 `GE_BulletDamage` 内嵌 EnemyHit Cue 的冷加载断言；原 HUD、弹药、换弹与 Dry Fire 覆盖保持不变并通过。

## 2026-09-01 session276 — 命中 Cue 回归断言纠正

- 回归断言改为验证 RepairGun 专属音效/Cue/子弹 Tag、敌人自有 HitReaction Cue，以及共享 `GE_BulletDamage` 不携带表现 Cue；AmmoLifecycle 冷启动 Success。

## 2026-09-01 session277 — 供应商残留清场回归

- 删除无 Referencer 的 `/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1` 四项遗留资产及整个磁盘目录后，AmmoLifecycle 冷启动仍为 Success，无缺失 Package。
- 后续待用户验收时提醒：提高 `GC_RepairGun_Impact.VolumeMultiplier`；当前值 1.0。
