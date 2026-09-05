# 构建配置

- FEAT-080 Chaos Cube：Private依赖GeometryCollectionEngine、Chaos、PhysicsCore、FieldSystemEngine。UGeometryCollection组件负责真实破碎，ExplosionGunBullet向其提交径向Strain/Impulse；不新增插件或外部模块。

**何时读取：** 新增模块依赖、修改插件列表、调整构建目标时。

| 文件 | 作用 |
|---|---|
| `TheManTest.uproject` | 项目配置、插件列表（GameplayAbilities、ModelingToolsEditorMode）；`test/unreal-mcp-5.7` 分支额外引用 `D:/Unreal Plugins/Unreal_mcp-v0.5.30/plugins` 并启用 Editor-only `McpAutomationBridge`，仅用于 UE 5.7.4 兼容性测试，尚未合入 `main`。外部插件基于 tag `v0.5.30`，当前检出本地分支 `fix/ue57-c4702` / commit `c9bee30`，修复 Rider 严格构建的不可达代码错误 |
| `Source/TheManTest/TheManTest.Build.cs` | 模块依赖（EnhancedInput、GameplayAbilities、GameplayTags、BBBAimIK、Niagara、AIModule、NavigationSystem、**UMG**(选角色 UI)、AnimGraphRuntime(装备动画方向计算)、CinematicCamera(选角 Cine Camera 焦距视差缩放)） |
| `Source/TheManTest.Target.cs` | 游戏目标配置 |
| `Source/TheManTestEditor.Target.cs` | 编辑器目标配置 |

## 2026-08-04 Locomotor

- `TheManTest.uproject` 启用引擎实验性 `Locomotor` 插件。
- `TheManTest.Build.cs` 增加 `Locomotor` 模块，用于 Nightmare FlyingBug2 的 `FVectorDamper` 程序化速度平滑。
- Editor 构建额外依赖 `AnimationBlueprintLibrary`，供 `UTheManAnimationAssetLibrary` 创建装备身体专用 Anim Layer 并写入动画资产覆盖；运行时构建不引入该编辑器模块。
- Editor 构建额外依赖 `Kismet`、`Slate`、`SlateCore`，供 `UTheManAnimationAssetLibrary` 打开数据型角色蓝图的完整组件编辑器，并以确定性的 Front/Side/Top 正交视图生成组件布局验收证据；运行时构建不引入这些模块。
