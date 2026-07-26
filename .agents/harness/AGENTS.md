# AGENTS.md

TheManTest — UE 5.7.4 单人游戏项目。C++ 负责数据结构、GAS 配置和核心逻辑；蓝图负责组件连接和每个角色的参数配置。

## 启动流程

开始写代码之前，依次执行：

1. 完整阅读本文件。
2. 阅读 `progress.md`，重点看底部的**会话交接**部分，了解上一次会话的遗留状态。
3. 阅读 `feature_list.json`，确认当前活跃功能及其完成标准。
4. 打开 `archive/<当前功能ID>-<名称>.md`，检查是否有未解决的 Bug 或遗留决策。

**框架地图的使用方式（贯穿整个会话）：**
- 需要接触某个系统时，在下方框架地图找到对应 arch 文件路径，用 Read 工具读取后再动手，不得凭印象假设文件内容。
- 不要在启动时一次性读取所有 arch 文件，按需读取，保持上下文干净。
- 遇到跨系统修改，先读 `.agents/harness/arch/cross-system-guide.md`，按顺序读取相关文件后再动手。

如果活跃功能存在阻塞项，先处理阻塞或向用户上报，不得绕过继续开发新内容。

## 工作规则

- **一次只做一个功能。** 从 `feature_list.json` 中选择且仅选择一个 `in_progress` 的功能。当前功能完成并归档之前，不得触碰其他功能。
- **动手前先列方案并等待确认。** 用户在 2026-07-04 明确要求：后续任何新的实现、代码/资产/配置修改、GameMode/UI/输入方案调整，在行动前必须先列出方案，等用户明确说“好/可以/按这个做”等确认后再动手。若只是读取状态或回答问题，可直接进行；一旦要改文件或指导具体落地步骤，先停下来给方案。
- **写入前建立 Git 安全检查点。** 任何代码、配置、蓝图、关卡或资产写入开始前，先运行 `git status`。工作区干净时无需创建空提交；工作区存在可明确识别的未提交改动时，先创建本地 WIP checkpoint（提交信息格式建议为 `WIP checkpoint before <操作摘要>`），再进行写入，以便 MCP 或编辑器操作失败时恢复到操作前状态。若改动范围异常、包含大量删除、归属不明或疑似误改，必须先向用户报告并确认，不得静默提交。纯只读检查不创建 checkpoint。写入后的结果不自动提交，仍等待用户明确说“更新 Git”；不自动 push 或合并分支。
- **C++ 修改必须重新编译。** 修改任何 `.h` 或 `.cpp` 文件后，必须在 Visual Studio（Development Editor / Win64）或 UE 编辑器内编译通过，才能声称改动有效。
- **蓝图修改必须在编辑器内验证。** 蓝图连线、默认值修改、类引用变更，都必须在 UE 编辑器内打开并编译。Claude Code 无法读写 `.uasset` 文件。
- **每个有意义的事件都要更新 Archive。** 功能创建、里程碑完成、发现 Bug、修复 Bug——立刻同步到该功能的 archive 文件中。
- **progress.md 只保留当前工作面板。** `progress.md` 不再承载长期历史流水账；历史细节归各功能 `archive/*.md`、`feature_list.json` 和 `arch/*.md`。更新 `progress.md` 时只保留当前 active feature 状态、最近关键完成项、当前待办/阻塞、最新一次会话交接。旧 session 交接和已归档细节应删除或压缩为 archive 引用。这里的“只保留当前工作面板”不是要求每次重写整份文件，而是清掉已经结束、废弃、无行动价值的旧信息，保留继续当前工作所需的最小面板。
- **范围内操作。** 不得修改与当前功能无关的文件。若发现必要的重构，新建一个独立的功能条目再做。
- **会话结束前更新文件。** 每次会话结束前，必须更新 `progress.md`（含交接部分）和 `feature_list.json`。
- **架构变更时同步更新 arch 文件。** 新增类、删除类、修改组件层级、调整继承链、改变文件路径——任何影响架构的改动，必须同步更新 `.agents/harness/arch/` 下对应的文件。判断标准：如果下一个 agent 按当前 arch 文件操作会走错路，就必须更新。
- **遇到乱码先尝试 UTF-8。** 读取 harness、archive、arch、中文文档或源码注释时如果终端输出出现乱码，先用 UTF-8 重新读取（PowerShell 示例：`Get-Content -Raw -Encoding UTF8 <path>`），不要基于乱码内容做判断或写回文件。
- **查看用户截图。** 用户的所有截图统一存在 `C:\Users\ROG\Pictures\Screenshots`。当用户说"看截图""看图"或需要查看编辑器界面 / 菜单 / 骨骼树 / PIE 效果时，用 Glob 列该目录 `*.png`（按修改时间排序），取**最新**的一张或几张用 Read 打开——用户通常只说"看截图"不给文件名，按截屏时间判断当前要看哪张。本环境是终端，无法在对话框直接粘贴图片，必须走"文件路径 → Read"这条路，不需要网页版。

### Blender 资产与 MCP 规则

- **独立 Blender 项目放在 D 盘专用目录。** Blender 工程统一使用 `D:\Blender Projects\<项目名>\`；每个项目建立独立子目录。
- **不要污染 Unreal 项目。** 不得把独立的 `.blend` 文件、Blender 建模脚本、MCP 下载文件或其他 Blender 工作文件放进 `D:\Unreal Projects\TheManTest`。只有明确准备导入游戏的最终导出资产，才可按 Unreal 项目既有资产目录规范放入本项目。
- **Blender MCP 使用方式。** 本机已安装 Blender MCP。实时控制时必须打开 Blender 图形界面，并在 3D Viewport 的 Blender MCP 侧栏启动 MCP Server；新增或重配 MCP 后通常需要重启 Codex 会话才能加载工具。
- **允许后台批处理。** 不需要交互界面的简单创建、转换或验证，可调用 Blender `--background` 配合 Python API；输出仍须保存到 `D:\Blender Projects\<项目名>\`，不得默认写入 Unreal 项目。

## 项目框架地图（按需读取）

> **使用方式：** 收到任务后、动手前，逐条匹配下方触发条件，命中即用 Read 工具读取对应文件，再开始写代码。启动时不需要全部读取。

| 文件路径 | 何时读取 |
|---|---|
| `.agents/harness/arch/system-overview.md` | 需要了解全局系统关系、组件层级时 |
| `.agents/harness/arch/01-build-config.md` | 新增模块依赖、修改插件列表时 |
| `.agents/harness/arch/02-core-framework.md` | 修改输入处理、角色切换、GameMode / PlayerState 时 |
| `.agents/harness/arch/03-character-base.md` | 修改角色通用行为、组件布局、GAS 初始化流程时 |
| `.agents/harness/arch/04-gas-attributes.md` | 新增或修改角色属性（血量、护甲等）时 |
| `.agents/harness/arch/05-character-data-asset.md` | 新增角色初始数值配置字段时 |
| `.agents/harness/arch/06-animation.md` | 新增动画变量、修改动画状态机驱动参数时 |
| `.agents/harness/arch/07-character-classes.md` | 为某个具体角色新增专属 C++ 逻辑时 |
| `.agents/harness/arch/08-equipment-manager.md` | 修改背包容量逻辑、装备切换规则时 |
| `.agents/harness/arch/09-equipment-system.md` | 新增装备类型、修改装备生命周期时 |
| `.agents/harness/arch/10-gas-abilities.md` | 新增或修改 GAS Ability、调试开火流程时 |
| `.agents/harness/arch/11-enemy-ai.md` | 修改敌人巡逻/感知/战斗状态、行为树、敌人技能集时 |
| `.agents/harness/arch/12-anim-blueprint.md` | 搭建/修改 ABP 层结构、Slot、AimIK 节点图、武器动画扩展时（06 的详细版） |
| `.agents/harness/arch/13-game-flow.md` | 修改回合/倒计时/阶段升级、死亡处理、关卡切换、选角色流程、GameInstance 跨关卡持久时 |
| `.agents/harness/arch/cross-system-guide.md` | 跨系统修改时，参考文件读取顺序 |

---

### 详细触发条件

**`.agents/harness/arch/system-overview.md`**
不清楚某个类在整体架构中的位置、数据如何在系统间流动时；新功能横跨多个系统时先读此文件建立全局认知。

**`.agents/harness/arch/01-build-config.md`**
修改 `TheManTest.Build.cs`（`PublicDependencyModuleNames` / `PrivateDependencyModuleNames`）；向 `.uproject` 添加或禁用插件；新增编译目标时。

**`.agents/harness/arch/02-core-framework.md`**
涉及以下任意类：`ATheManPlayerController` / `ATheManPlayerState` / `ATheManGameModeBase` / `ATheManLobbyGameMode` / `ATheManGameStateBase` / `UTheManGameInstance` / `UCharacterSelectWidgetBase`；修改角色切换 `SwitchCharacter`、输入映射上下文（`IMC_Default`）时。（回合/死亡/大厅/关卡切换的系统逻辑详见 13。）

**`.agents/harness/arch/03-character-base.md`**
涉及 `AFPSCharacterBase`；修改相机 / `ArmsMesh` / `HeadCamera` 组件层级；修改 `PossessedBy` 或 GAS 初始化流程；为所有角色新增共用组件或方法；修改 `SetupPlayerInputComponent` 通用绑定时。

**`.agents/harness/arch/04-gas-attributes.md`**
新增或修改任何 `FGameplayAttributeData`（如 Health / MaxHealth / 护甲 / 耐力）；涉及 `UTheManAttributeSetBase` / `UEnemyAttributeSetBase`；新增 `ATTRIBUTE_ACCESSORS` 宏时。

**`.agents/harness/arch/05-character-data-asset.md`**
修改 `UTheManCharacterDataAssetBase`；向角色数据资产（`DA_*`）新增配置字段；修改 `InitGEClass` 引用关系时。

**`.agents/harness/arch/06-animation.md`**
涉及以下任意类：`UBaseLocomotionAnimInstance` / `UFPSCharacterAnimInstance`（原 `UFPSArmsAnimInstance`，FEAT-041 改名）/ `UHumanoidEnemyAnimInstance` / `UFirearmAnimInstance`；新增动画变量或修改 `NativeUpdateAnimation`；调试 ABP 状态机驱动数据时。

**`.agents/harness/arch/07-character-classes.md`**
涉及具体角色类：`AFPSInfiltrator` / `AFPSMaintenanceWorker` / `AFPSTheExecutive` / `AEnemyBase` / `AHumanoidEnemy` / `APhantom` / `ANightmareEnemy` / `AHumanoidAIController`；为某个角色新增专属能力、组件或输入绑定时。

**`.agents/harness/arch/08-equipment-manager.md`**
涉及 `UEquipmentManagerComponent`；修改背包初始化、`SwitchEquipment`、`Inventory` 数组、`AttachTargetMesh` 赋值逻辑时。

**`.agents/harness/arch/09-equipment-system.md`**
涉及 `AEquipmentBase` / `AWeaponBase` / `AFirearm` / `ABulletBase` / `ARepairGunBullet`；新增武器或装备类型；修改 `Equip()` / `Unequip()` 行为；修改 AnimLayer 链接 / Socket 名称 / `GrantAbilities` / `RevokeAbilities` 时。

**`.agents/harness/arch/10-gas-abilities.md`**
涉及 `UGA_Shoot` / `UGA_InfiltratorScan` 或任何新 `UGameplayAbility` 子类；修改 `TheManGameplayTags.h/.cpp`（`UE_DECLARE/DEFINE_GAMEPLAY_TAG`）；调试 LMB→开火→GAS 事件链路；新增 `AbilityTriggers` 监听时。

**`.agents/harness/arch/11-enemy-ai.md`**
涉及敌人 AI 行为与战斗系统：`AHumanoidAIController` 感知/黑板/行为树；`AHumanoidEnemy` 巡逻/转身/`SetAIState`；`EHumanoidEnemyAIState` 状态机；`AEnemyBase` 技能集（`PhaseSkillSets` 阶段×近中远 / `UseRandomSkill`）；`BTTask_UseCombatSkill` / `BTTask_ResumeNearestPatrol`。（07 偏各角色类的 C++ 逻辑，本文偏 AI 行为与系统关系。）

**`.agents/harness/arch/12-anim-blueprint.md`**
`06-animation.md` 的详细版：搭建/修改 ABP 资产的层结构（Linked Anim Layer）、Slot 蒙太奇插槽（DefaultSlot/UpperBodySlot/FullBodySlot）、`ALI_WeaponAnim` 接口层、`ABP_FirearmBase` AimIK 节点图、武器动画“共享基础+按需覆盖”扩展策略，以及人形怪 ABP 完整节点图。也保留搁置的双骨骼旧系统设计供恢复参考。

**`.agents/harness/arch/13-game-flow.md`**
涉及游戏流程闭环：回合系统（`ATheManGameStateBase` 倒计时/强度波/半场二阶段/`DebugSkipTime`）；死亡处理（`OnDeath` / `OnCountdownExpired` → `HandlePlayerDeath`）；关卡切换与选角色（`UTheManGameInstance` / `ATheManLobbyGameMode` / `UCharacterSelectWidgetBase` / GameMode `GetDefaultPawnClassForController`）；跨关卡持久数据（`SelectedCharacterID` / `CarriedRoundNumber`）；输入模式跨关卡重置时。

**`.agents/harness/arch/cross-system-guide.md`**
任务同时涉及上方两个或以上系统时，先读此文件确认各系统文件的读取顺序，再依次读取。

---

## UE5 代码规范

### 命名前缀

| 前缀 | 类型 |
|---|---|
| `A` | Actor 子类 |
| `U` | UObject / 组件子类 |
| `F` | 结构体 |
| `I` | 接口 |
| `E` | 枚举 |
| `BP_` | 蓝图资产 |
| `DA_` | 数据资产（UPrimaryDataAsset） |
| `GE_` | Gameplay Effect 蓝图 |
| `GA_` | Gameplay Ability 蓝图（C++ 类以 `UGA_` 前缀命名） |
| `ABP_` | 动画蓝图 |
| `DT_` | 数据表 |
| `IA_` / `IMC_` | 输入动作 / 输入映射上下文 |

源文件名必须与类名完全一致。

### C++ 与蓝图的职责划分

- **C++ 负责：** 类定义、UPROPERTY/UFUNCTION 声明、GAS 初始化、构造函数中创建组件、核心游戏逻辑。
- **蓝图负责：** 设置 `EditDefaultsOnly` 属性、动画图表、UI、无需编译期知识的组件连线。
- 蓝图需要调用的函数加 `BlueprintCallable`；蓝图需要重写的函数加 `BlueprintImplementableEvent` 或 `BlueprintNativeEvent`。

### GAS 架构

- ASC 挂载在 `ATheManPlayerState` 上，不得移到 Character。
- 属性在 `UTheManAttributeSetBase` 中用 `ATTRIBUTE_ACCESSORS` 宏声明。
- 新增属性：在 `TheManAttributeSetBase.h/.cpp` 添加 `FGameplayAttributeData` + 宏，再创建对应的 GE 蓝图初始化数值。
- Character 在 `PossessedBy` 中通过 `PlayerState->GetAbilitySystemComponent()` 获取 ASC，不得在此之前缓存。
- **输入 → GAS 的标准链路：**
  ```
  Character::SetupPlayerInputComponent
    BindAction → Character::XxxFire() / ActivateScan() 等
      → ASC->HandleGameplayEvent(TAG_Input_Xxx, &Payload)
        → UGA_Xxx（AbilityTriggers 监听对应 Tag）::ActivateAbility()
  ```
- **技能授予的两个来源：**
  1. **武器技能**：`AFirearm::Equip()` 调用 `GrantAbilities(ASC)`；`PossessedBy` 末尾补授一次（解决 BeginPlay 时序问题）。
  2. **角色专属技能**：在具体角色的 `PossessedBy` 中遍历 `DefaultAbilityClasses` 调用 `ASC->GiveAbility()`，与武器系统完全独立。
- **新增 GAS 技能的标准流程：**
  1. 在 `TheManGameplayTags.h/.cpp` 声明/定义触发 Tag（命名规范：`Input.角色.技能名`）
  2. 新建 C++ 类继承 `UGameplayAbility`，CDO 中用 `AbilityTriggers` 监听该 Tag
  3. 在对应角色的 `DefaultAbilityClasses`（或武器的 `PrimaryFireAbilityClass`）中引用
  4. 在编辑器创建蓝图子类赋值到角色蓝图
- **Gameplay Tag 定义**：所有 C++ Tag 在 `GAS/TheManGameplayTags.h/.cpp` 中用 `UE_DECLARE/DEFINE_GAMEPLAY_TAG` 宏定义，不得硬编码字符串。

### 装备系统

- 继承链：`AEquipmentBase` → `AWeaponBase` → `AFirearm`。
- 新装备类型在对应层级继承，并重写 `Equip(AActor*)` / `Unequip()`。
- `UEquipmentManagerComponent` 挂载在 Character 上，负责管理运行时背包数组。
- 插槽名（`EquipSocketName`、`HolsterSocketName`）、动画蒙太奇、动画层级类，均在蓝图中配置（`EditDefaultsOnly`）。

### 输入系统

**架构：Controller = 纯注册表，Character 自绑（2026-06-08 重构）**

```
ATheManPlayerController
  ├── BeginPlay：AddMappingContext（唯一负责）
  ├── SetupInputComponent：只绑 TestSwitchCharacterAction（Controller 级元操作）
  └── 持有所有 UInputAction* 资产（IA_Move / IA_Look / IA_Scan 等）
        └── 通过 GetXxxAction() 公开 getter 供 Character 读取

AFPSCharacterBase::SetupPlayerInputComponent（Possess 时引擎自动调用）
  └── 绑定通用输入：Move / Look / Jump / Sprint / SwitchEquipment / PrimaryFire / SecondaryFire
       （Sprint：FEAT-039，按住 StartSprint 提速 / 松开 StopSprint 回 WalkSpeed）

AFPSInfiltrator::SetupPlayerInputComponent（override，先 Super）
  └── 额外绑定 ScanAction → ActivateScan()（仅 Infiltrator 响应）
```

**规则：**
- Controller 不得调用任何 Character 方法，不得 Cast 到具体角色类型。
- 新增角色专属输入：在该角色类的 `SetupPlayerInputComponent` 中 override，调用 `Super::` 后追加绑定。
- 新增共用输入（所有角色都需要）：在 `AFPSCharacterBase::SetupPlayerInputComponent` 中添加；同时在 Controller 新增对应 `UInputAction* GetXxxAction()` getter。
- 全部使用增强输入（`UInputMappingContext`、`UInputAction`），回调签名为 `void(const FInputActionValue&)` 或 `void()`。

### 角色切换

- 角色花名册是 DataTable（`DT_CharacterRoster`），行类型为 `FCharacterType`。
- `ATheManPlayerController::SwitchCharacter(FName RowName)` 驱动运行时切换。
- 新增可玩角色：C++ 继承 `AFPSCharacterBase`，创建 BP_ 包装类，在 `DT_CharacterRoster` 添加一行。

## 完成标准

功能**仅在以下所有条件全部满足时**才可标记为完成：

- [ ] C++ 在 Development Editor / Win64 下编译无错误、无新增警告。
- [ ] 蓝图无编译错误（在 UE 编辑器中验证）。
- [ ] 在编辑器内 PIE 运行，主路径行为符合预期。
- [ ] 编译结果摘要和 PIE 测试描述已记录到 `feature_list.json` 和该功能的 archive 文件中。
- [ ] `progress.md` 和 `feature_list.json` 已更新为完成状态。
- [ ] Archive 文件状态改为 `done`，并记录关闭日期。

## 验证命令

```powershell
# 命令行构建（UE 5.7 实测装在 D 盘，非默认 C 盘）
& "D:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" `
    TheManTestEditor Win64 Development `
    "D:\Unreal Projects\TheManTest\TheManTest.uproject" `
    -WaitMutex -FromMsBuild
```

蓝图编译错误和 PIE 测试必须在 UE 编辑器内完成，没有对应的命令行工具。

## Context 接近满载时的处理规则

当用户提示 context 快满（或 Claude 自行判断 context 使用量过高）时，**立即执行以下操作后结束会话**，不得继续做新的代码改动：

1. 将当前进行中的工作状态完整写入 `progress.md`：
   - 已完成的步骤（打勾）
   - 进行中但未完成的步骤（含具体卡在哪里）
   - 下一步推荐操作
2. 更新活跃功能的 archive 文件，补录本次会话的实现日志
3. 更新 `feature_list.json` 中该功能的 `updated` 日期
4. 填写 `progress.md` 底部的**会话交接**部分，**替换掉上一次的交接内容**（只保留最新一次），确保下一会话能无缝接续

> 目的：让下一会话通过读 harness 文件就能完整还原当前状态，不依赖聊天记录。

---

## 升级处理

| 情况 | 处理方式 |
|---|---|
| 架构决策 | 先查本文件和当前功能 archive；仍不明确则询问用户 |
| 蓝图专属阻塞 | 记入 `progress.md` 交接部分；需人工在编辑器中解决 |
| 持续编译失败 | 将完整错误信息记入 archive Bug 日志，标记需人工介入 |
| 发现范围蔓延 | 停止，新建功能条目，回到当前功能 |
| 完成标准不明确 | 重新阅读 `feature_list.json`；没有证据不得标记完成 |
