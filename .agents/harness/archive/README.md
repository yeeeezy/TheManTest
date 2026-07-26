# Archive 目录说明

`feature_list.json` 中的**每一个功能条目都必须在此目录下有一个对应的 archive 文件**。

Archive 文件贯穿功能的整个生命周期——从规划、实现、Bug 发现、Bug 修复，直到最终关闭。它是功能历史的唯一真相来源。

---

## 命名规范

```
FEAT-NNN-short-kebab-name.md
```

示例：
- `FEAT-001-firearm-reload-system.md`
- `FEAT-002-character-switch-ui.md`
- `FEAT-003-gas-damage-attribute.md`

`NNN` 与 `feature_list.json` 中的 `id` 字段保持一致。

---

## 更新时机

以下事件发生时，**立即**更新对应的 archive 文件：

| 事件 | 动作 |
|---|---|
| 功能加入 `feature_list.json` | 创建 archive 文件，填写规划部分 |
| 开始实现 | 填写「实现日志 — 开始」 |
| 完成一个里程碑 | 追加「实现日志 — 里程碑」条目 |
| 发现 Bug | 在「Bug 记录」中新增条目，状态 `open` |
| Bug 修复 | 更新对应 Bug 条目，状态改为 `fixed`，填写修复描述 |
| 完成标准全部满足 | 填写「验证证据」表格，状态改为 `done` |
| 功能关闭 | 填写「最终备注」，记录关闭日期 |

---

## 模板

复制 `_template.md` 到此目录，按命名规范重命名后开始填写。

---

## 功能索引

> 每新增一个 archive 文件，在下表追加一行。

| 文件 | 功能名称 | 状态 | 创建日期 |
|---|---|---|---|
| [FEAT-001-firstpersonmesh-as-camera-root.md](FEAT-001-firstpersonmesh-as-camera-root.md) | FirstPersonMesh 作为相机父级 | done | 2026-06-06 |
| [FEAT-002-camera-pitch-clamp.md](FEAT-002-camera-pitch-clamp.md) | 相机俯仰角限制 | done | 2026-06-06 |
| [FEAT-003-bbbaim-ik-integration.md](FEAT-003-bbbaim-ik-integration.md) | BBBAimIK 插件集成 | in_progress | 2026-06-06 |
