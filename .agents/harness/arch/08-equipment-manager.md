# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Weapons/_Shared/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Weapons/_Shared/Components/EquipmentManagerComponent.cpp` | 切枪先完整 `Unequip → Equip/链接新层 → 挂载`，保证旧技能先回收、新技能后授予。新武器先隐藏一帧等待 Linked Layer 求值，再显示并调用 C++ `PlayEquipEffect()`，以 VFXPack 同款材质溶解显现替代手臂 Equip Montage；0.45 秒效果期间锁定重复切枪输入。`FinalizeUnequippedEquipment()` 统一处理隐藏/挂回 Holster；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory** |
