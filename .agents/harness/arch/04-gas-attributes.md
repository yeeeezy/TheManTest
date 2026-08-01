# GAS 属性集

**何时读取：** 新增或修改角色属性（血量、耐力、护甲等）时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/CharacterBase/TheManAttributeSetBase.h` | `Health` / `MaxHealth` 声明，`ATTRIBUTE_ACCESSORS` 宏用法示例 |
| `Source/TheManTest/Private/Characters/CharacterBase/TheManAttributeSetBase.cpp` | 属性默认值 |
| `Source/TheManTest/Public/Enemy/EnemyAttributeSetBase.h` | 继承 `UTheManAttributeSetBase`，怪物专属属性扩展点 |

> 新增属性后，还需要创建对应的 `GE_` 蓝图来初始化数值，并在角色蓝图的 `InitGEClass` 中引用。
