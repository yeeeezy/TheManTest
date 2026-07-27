# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Characters/Components/EquipmentManagerComponent.cpp` | 背包初始化与切枪统一使用完整 `Unequip → Equip/链接新层 → 挂载显示 → 下一帧 PlayEquipMontage` 顺序。下一帧播放前会确认目标仍是当前装备，避免快速滚轮误播；Montage 结束后不再 Link/Unlink，因此无参考姿势空档。旧武器技能在新武器授予前完整回收，避免技能混用；`FinalizeUnequippedEquipment()` 统一处理隐藏/挂回 Holster；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory**（防切角色技能泄漏） |
