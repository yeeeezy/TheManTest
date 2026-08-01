# 角色数据资产

**何时读取：** 新增角色初始数值配置字段（如初始护甲值、移速上限）时。

| 文件 | 关键内容 |
|---|---|
| `Source/TheManTest/Public/Characters/CharacterBase/Data/TheManCharacterDataAssetBase.h` | `InitialMaxHealth` / `InitialHealth`（EditDefaultsOnly，在 DA_ 资产中配置） |
| `Source/TheManTest/Private/Characters/CharacterBase/Data/TheManCharacterDataAssetBase.cpp` | — |

> 敌人不使用 DataAsset，`InitialMaxHealth` / `InitialHealth` 直接在蓝图中配置。
