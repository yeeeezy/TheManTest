# 装备管理组件

**何时读取：** 修改背包容量逻辑、装备切换规则、装备初始化流程时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/Components/EquipmentManagerComponent.h` | `InitializeEquipment()`、`SwitchEquipment(int32 Direction)`、`GetCurrentEquipment()`、`Inventory` 数组、`AttachTargetMesh` |
| `Source/TheManTest/Private/Characters/Components/EquipmentManagerComponent.cpp` | 背包初始化（生成 Actor，初次装备不播蒙太奇，由角色下一帧安全播放）；切换逻辑（Unequip → Equip/挂载；无 Montage 的装备立即显示；有 Montage 时收起旧装备并暂时隐藏新装备与 `ArmsViewMesh`，下一帧播放，再下一帧姿势评估完成后同帧显示手臂和新装备），避免 Linked Anim Layer 初始化清掉同帧 Montage、快速滚轮误播以及最终持枪 Idle 闪帧；`CharacterMesh0` 不隐藏，影子动画持续；`FinalizeUnequippedEquipment()` 统一处理隐藏/挂回 Holster；**EndPlay 先 `Unequip()` 回收技能再销毁 Inventory**（防切角色技能泄漏） |
