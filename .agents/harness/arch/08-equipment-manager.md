# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Characters/Components/EquipmentManagerComponent.cpp` | 背包初始化（生成 Actor，初次装备不播蒙太奇，由角色下一帧安全播放）；切换逻辑（Unequip → Equip/挂载；无 Montage 的装备立即显示；有 Montage 的装备先保持隐藏，下一帧确认仍为当前装备后播放，再下一帧姿势已评估后显示），避免 Linked Anim Layer 初始化清掉同帧 Montage、快速滚轮误播旧装备，以及先露出持枪 Idle 再向下混到拔枪起始姿势；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory**（防切角色技能泄漏） |
