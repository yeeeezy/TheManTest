# 资产目录规范

## 核心原则

- 顶层与所有者层按功能所有权组织；所有者内部再按资源类型组织。
- 具体角色、武器、敌人或 Actor 的专属资源必须留在其所有者目录，不得堆入 `CharacterBase`、`WeaponBase` 或 `_Shared`。
- `_Shared` 只接收至少两个已存在使用方明确共同依赖、并且计划保持共同生命周期的资源。
- 即使当前多个具体对象临时使用相同表现资产，只要后续需要独立替换，就为各所有者保留独立资产，不提升为 `_Shared`。
- 不为目录对称创建空目录；资产实际出现时再创建对应分类。

## 统一分类名

所有者目录按需使用以下名称，不得混用单复数变体：

- `Blueprint`
- `Data`
- `Animations`
- `Meshes`
- `Materials`
- `Textures`
- `Audio`
- `Effects`
- `AI`
- `GAS/Abilities`
- `GAS/Effects`

禁止长期保留 `Animation`、`Mesh`、`Material`、`DataAsset`、`GameplayAbility`、`GameplayEffect` 等同义目录。

## 角色

```text
/Game/Characters/
├─ CharacterBase/
│  ├─ Blueprint/
│  ├─ Data/
│  └─ GAS/
│     ├─ Abilities/
│     └─ Effects/
├─ <ConcreteCharacter>/
│  ├─ Blueprint/
│  ├─ Data/
│  ├─ Animations/
│  │  ├─ Body/
│  │  └─ FirstPerson/
│  ├─ Body/
│  │  ├─ Meshes/
│  │  ├─ Materials/
│  │  └─ Textures/
│  ├─ FirstPerson/
│  │  ├─ Meshes/
│  │  ├─ Materials/
│  │  └─ Textures/
│  ├─ Audio/
│  ├─ Effects/
│  └─ GAS/
└─ _Shared/
```

`CharacterBase` 只包含与抽象角色基类直接绑定的 Blueprint、默认数据和 GAS 基础设施，不包含具体身体、手臂、动画、材质或贴图。无骨架、由多个具体玩家角色继承的 Template AnimBP 属于抽象角色架构，可放在 `CharacterBase/Animations/{Body,FirstPerson}/Logic`；具体 Skeleton、Sequence、BlendSpace 和最终子 AnimBP 仍必须留在具体角色目录。

动画资产数量较少时直接放入 `Animations/Body` 或 `Animations/FirstPerson`。只有目录已明显拥挤时，才继续拆分 `Logic`、`Locomotion`、`Actions`。Skeleton 与 Physics Asset 跟随对应 Skeletal Mesh 放在 `Meshes`。

## 武器、敌人和 Actor

```text
/Game/Weapons/<Weapon>/...
/Game/Enemy/<Family>/<Enemy>/...
/Game/Actors/<ActorFeature>/...
```

同样遵循所有者优先和统一分类名。武器动画可按需分为 `Animations/FirstPerson` 与 `Animations/World`；敌人共享 AI 放在最近共同语义根的 `_Shared/AI`。

## 新建、导入与迁移

1. 先确定唯一所有者，再选择资源类型目录。
2. 外部素材不得以供应商或素材包目录作为正式路径。
3. 移动和重命名必须通过 Unreal AssetTools，不能直接移动 `.uasset` 文件。
4. 迁移后检查目标存在、旧路径消失、引用更新、定向 Redirector 为零以及磁盘空目录清理。
5. 相关 Blueprint/AnimBP 必须编译保存；涉及硬编码路径时同步更新 C++、测试、脚本和架构文档。
