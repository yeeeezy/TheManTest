# FEAT-078 — Combat HUD 与玩家枪械弹药

**创建日期：** 2026-09-01
**状态：** in_progress

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

- `TheManTestEditor Win64 Development`：Success。
- `TheManTest.Player.CombatHUD.AmmoLifecycle`：1/1 Success。验证默认30/30与备用弹夹3、扣弹同步为29、换弹恢复30且备用弹夹变2。
- 含 UI 的视口截图：`Saved/Screenshots/WindowsEditor/TMT_CombatHUD.png`。中心圆与右下角两行弹药均可见。
- 既有 `FramingCapture` 仍因本功能之前的 FOV=77 与 GripPoint/GripPoint1 断言不一致失败；与 Combat HUD 无关。

## 待办

- 用户前台确认准星半径、线宽和右下角留白是否符合主观观感。
- 换弹输入与换弹动画尚未接入；当前仅提供可调用的 `ReloadMagazine()` 状态接口。
