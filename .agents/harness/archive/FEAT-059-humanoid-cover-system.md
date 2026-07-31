# FEAT-059 — 通用掩体系统与测试掩体资产

**状态：** done
**完成：** 2026-07-30

通用掩体 Actor/查询接口与测试资产。Blender 源工程放 `D:\Blender Projects\PhantomCover\`，TheManTest 只导入最终 Mesh/材质/贴图。

## 实现与验证

- `AEnemyCoverPoint` 提供 CoverMesh、阻挡 Box、StandPoint 和距离/背向/Visibility 遮挡评分。
- Blender 源 `.blend`、FBX、PNG、脚本及 800×600 预览均在外部 PhantomCover 目录；预览已打开检查。
- 最终资产位于 `/Game/Enemy/_Shared/Cover`，含 BP、Mesh、三材质和 ColorMask 贴图。
- 自动化瞬态 Cover 射线遮挡选择通过，未保存 TestMap。
