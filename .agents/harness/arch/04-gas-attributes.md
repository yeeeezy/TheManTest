# GAS 属性集

**何时读取：** 新增或修改角色属性（血量、耐力、护甲等）时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/CharacterBase/TheManAttributeSetBase.h` | `Health` / `MaxHealth` 声明，`ATTRIBUTE_ACCESSORS` 宏用法示例 |
| `Source/TheManTest/Private/Characters/CharacterBase/TheManAttributeSetBase.cpp` | 属性默认值 |
| `Source/TheManTest/Public/Enemy/EnemyAttributeSetBase.h` | 继承 `UTheManAttributeSetBase`，怪物专属属性扩展点 |

> 新增属性后，还需要创建对应的 `GE_` 蓝图来初始化数值，并在角色蓝图的 `InitGEClass` 中引用。

Combat HUD 由 `ATheManPlayerController` 绑定 PlayerState ASC 的 Health/MaxHealth 属性变化委托；界面当前只绘制取整后的 Health，不绘制 MaxHealth。

敌人头顶血条由 `AEnemyBase` 统一持有 `UWidgetComponent`，使用原生 `UEnemyHealthBarWidgetBase` 绘制黑底红条。`AEnemyBase::BeginPlay` 在 InitGE 后绑定自身 ASC 的 Health/MaxHealth 变化委托并立即刷新，因此 Phantom、Nightmare 及后续所有 `AEnemyBase` 子类自动继承，无需在各敌人 Blueprint 重复配置。
