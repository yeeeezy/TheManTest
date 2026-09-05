# 装备管理组件

**何时读取：** 修改背包容量、装备切换或初始化流程时。

- `Source/TheManTest/Public/Weapons/_Shared/Components/EquipmentManagerComponent.h`：InitializeEquipment、SwitchEquipment、GetCurrentEquipment、Inventory、AttachTargetMesh。
- 同路径 Private 实现：切换先 Unequip 回收技能，再 Equip、链接新层并挂载。首次装备与后续切换统一调用 QueueEquipPresentation，等待一帧姿势求值后调用装备 PlayEquipEffect 并显示，标记一次 Camera Cut 清理旧画面历史。
- 共用 VFX 由每个 AEquipmentBase 的 UEquipmentEquipEffectComponent 播放，时序/材质状态归属 `Weapons/_Shared/EquipmentBase/Effects/`，Manager 不再持有重复的 0.5 秒定时器。额外滚轮输入在待显示帧或当前装备效果播放期间被忽略。
- 可选装备动画由装备自身 bPlayEquipAnimation / EquipMontage 控制，和共享 VFX 同时播放；EquipmentAnimLayerClass 保持各自配置。
- FPSCharacterBase 只指定 ArmsViewMesh 并初始化背包、预热动画姿势，已移除 PlayInitialEquipEffect 及重复的首装视觉逻辑。
- FinalizeUnequippedEquipment 负责隐藏或挂回 Holster；EndPlay 先 Unequip 回收能力再销毁 Inventory。
