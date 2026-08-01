# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Weapons/_Shared/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Weapons/_Shared/Components/EquipmentManagerComponent.cpp` | 切枪先完整 `Unequip → Equip/链接新层 → 挂载显示`，保证旧技能先回收、新技能后授予。Equip Montage 使用首帧桥接：切换调用当帧立即播放覆盖新层 Idle；下一动画更新完成后由 next-tick 回调确认目标仍是当前装备并从 0 正式重启，避免 Linked Layer 初始化清除同帧 Montage，同时没有最终持枪姿势的可见窗口。Montage 结束后不再 Link/Unlink，因此无 T-Pose；`FinalizeUnequippedEquipment()` 统一处理隐藏/挂回 Holster；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory** |
