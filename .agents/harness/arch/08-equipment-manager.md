# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Characters/Components/EquipmentManagerComponent.cpp` | 背包初始化（生成 Actor，初次装备不播蒙太奇，由角色下一帧安全播放）；无 Montage 的切换立即完成。Equip Montage 切换时立即回收旧装备逻辑但暂留旧 Linked Layer，新装备逻辑/枪体立即启用但暂不链接新层，在稳定主 AnimInstance 上直接播放 Montage；结束后才解除旧层并链接新层进入 Idle。手臂与枪全程可见，不需要显隐等待，也不会由新层初始化清掉 Montage；过渡期间忽略重复切换。`FinalizeUnequippedEquipment()` 统一处理隐藏/挂回 Holster；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory**（防切角色技能泄漏） |
