# 构建配置

**何时读取：** 新增模块依赖、修改插件列表、调整构建目标时。

| 文件 | 作用 |
|---|---|
| `TheManTest.uproject` | 项目配置、插件列表（GameplayAbilities、ModelingToolsEditorMode） |
| `Source/TheManTest/TheManTest.Build.cs` | 模块依赖（EnhancedInput、GameplayAbilities、GameplayTags、BBBAimIK、Niagara、AIModule、NavigationSystem、**UMG**(选角色 UI)、AnimGraphRuntime(装备动画方向计算)、CinematicCamera(选角 Cine Camera 焦距视差缩放)） |
| `Source/TheManTest.Target.cs` | 游戏目标配置 |
| `Source/TheManTestEditor.Target.cs` | 编辑器目标配置 |
