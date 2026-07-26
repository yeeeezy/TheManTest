# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Characters/Components/EquipmentManagerComponent.cpp` | 背包初始化（生成 Actor，初次装备不播蒙太奇，由角色 reveal 时播）；切换逻辑（Equip + `PlayEquipMontage()` 立即播 / Unequip）；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory**（防切角色技能泄漏） |
