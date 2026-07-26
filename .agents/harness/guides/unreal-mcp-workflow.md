# Unreal MCP 操作手册（UE 5.7）

## 适用范围

使用 Unreal MCP 对本项目进行资产检查、导入、移动、重命名、引用验证或轻量编辑器自动化时，先读取本文件。架构事实仍以 `arch/` 为准；本文件只记录可靠的操作流程、验证方法和已知陷阱。

## 当前环境

- Unreal Engine：5.7.4。
- MCP 插件：`ChiR24/Unreal_mcp v0.5.30`，外部目录 `D:\Unreal Plugins\Unreal_mcp-v0.5.30`。
- 插件兼容修复分支：`fix/ue57-c4702`，commit `c9bee30`。
- 原生 MCP 地址：`http://127.0.0.1:3000/mcp`，只监听本机回环地址。
- 编辑器启动并加载项目后，插件会自动启动 MCP；若 Codex 没有加载到工具，通常需要重启 Codex 会话，而不是重启 Unreal 项目。

## 标准短流程

1. 读取 harness 与相关架构文档，明确资产所有权和目标目录。
2. 执行 `git status`；存在已知改动时按项目规则建立选择性的本地 WIP checkpoint，不要混入用户的无关编辑。
3. 确认 Unreal Editor 正在运行且响应正常，MCP `initialize` / `tools/list` 可用。
4. 使用 Unreal AssetTools 或 MCP 资产操作完成导入、移动或重命名；不要在资源管理器里直接剪切 `.uasset`。
5. 只查询本次修改涉及的路径和资产，验证类型、尺寸、材质槽、依赖与引用。
6. 对源目录做定向 Redirector 检查；必要时 fixup，再复查。
7. 保存资产，更新 harness，并检查 Git/LFS 状态。
8. 结果不自动提交；等待用户明确说“更新 Git”。不自动 push 或 merge。

## 资产移动与目录整理

- 移动或重命名必须通过 Unreal AssetTools/MCP，让引擎同步更新软硬引用并生成可处理的 Redirector。
- 先移动资产，再查询目标资产是否存在、旧路径是否消失、使用方 dependencies 是否指向新路径。
- Redirector 扫描限定在本次涉及的目录，例如 `/Game/Weapons/RepairGun`；不要默认递归列出整个 `/Game`。
- 批量整理按“所有权优先、资源类型次之”：共享资源放 `_Shared`，专属资源放具体武器或角色目录，再拆分 `Mesh`、`Material`、`Textures`、`Blueprint`、`Animation`。

## 导入验证

- Blender 源工程、脚本、FBX 和预览图保存在 `D:\Blender Projects\<项目名>\`，不放进 Unreal 项目。
- 导入前确认 Unreal 坐标、前向轴、单位和预期尺寸；导入后从 Static Mesh bounds 验证实际尺寸。
- 验证材质槽数量及每个槽的完整资产路径，不要只确认材质文件“看起来存在”。
- 没有独立 Texture 资产不一定是错误：纯参数材质可以只有 Material/Material Instance。

## 推荐的定向检查

每次只返回回答当前问题所需的字段：

- 资产存在性：目标路径存在，旧路径不存在。
- 资产类型：确认 `StaticMesh`、`Material`、`Blueprint` 等类别。
- Static Mesh：bounds、材质槽名称和材质对象路径。
- 引用关系：查询具体使用方的 dependencies/referencers。
- Redirector：Asset Registry 只扫描相关根目录，并过滤 `ObjectRedirector`。

若工具缺少精确查询，可以通过 MCP 的 `system_control / execute_python` 使用 Unreal Python API，但脚本应保持短小，只打印带固定前缀的最终结果，避免返回大量日志。

## 超时与输出控制

- 普通存在性、依赖和属性查询应使用短调用；单次输出应尽量控制在几十行内。
- 导入、保存、fixup redirectors 可能超过普通查询时间。调用超时不等于操作失败：先检查编辑器是否响应，再检查目标资产和磁盘文件，禁止立即盲目重试。
- `manage_asset list` 递归扫描大目录可能产生巨量输出。优先指定目录、类别和资产名；无法过滤时改用短 Unreal Python 查询。
- 宽范围 `fixup_redirectors` 可能超时。优先处理本次移动的源目录，然后用 Asset Registry 定向确认是否还有 Redirector。

## 已知陷阱

### MCP 调用超时

某些资产操作已在 Unreal 主线程完成，但 HTTP 客户端先超时。正确处理方式是验证结果和编辑器响应，不是立即重复执行写操作。

### 查询输出过大

递归列出 `/Game` 或 `/Game/Weapons` 的所有资产会淹没有效结果，甚至超过会话上下文。用资产路径、类别和固定输出前缀缩小结果。

### 无界面导入后的 Slate 断言

本项目曾出现无界面导入已保存资产后，Content Browser/Slate 路径触发断言。不要仅依据进程退出判断导入失败；重新启动独立只读验证，确认资产可加载、类型和材质槽正常。

### Rider 的 C4702

MCP 插件旧 LevelHandlers 路径在无条件 `return` 后保留不可达代码，Rider 使用严格警告设置时会报 C4702。兼容修复已隔离在插件外部仓库的 `fix/ue57-c4702` 分支，不要把插件源码复制进游戏项目。

### `.uasset` 与 Git LFS

二进制资产由 Git LFS 管理。Git 检查应确认 `filter=lfs`；不得用文本 diff 判断 `.uasset` 内容。选择性 checkpoint 时明确列出路径，避免提交用户正在编辑的其他资产。

## 完成检查清单

- [ ] 目标资产存在且类型正确。
- [ ] 旧路径不存在或只剩待处理 Redirector。
- [ ] 使用方引用已经指向新路径。
- [ ] Mesh bounds、方向和材质槽符合预期。
- [ ] 本次相关目录不存在 Redirector。
- [ ] 编辑器仍响应正常，资产已保存。
- [ ] Harness 已记录操作结果和已知限制。
- [ ] Git 状态中没有误纳入用户的无关改动。
- [ ] 结果等待用户明确触发最终 Git 提交。
