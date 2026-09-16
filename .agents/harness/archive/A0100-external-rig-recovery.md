# A0100 外部骨架恢复 — 2026-09-15

- 用户授权检查 A0100、恢复原绑定并导出，随后安装 3ds Max 2027.2。
- 全部资产工作位于 `D:\Blender Projects\A0100_RigRecovery`；未导入或修改 Unreal Content，未执行 IK 或动画重定向。
- Max 实际打开确认 CAT 骨架和 Skin：57 个引用骨骼、51,879 顶点，未绑定顶点 0、权重和异常 0，最大影响数 7。副本导出前仅在内存中唯一化重复辅助节点名称。
- 原文件未修改，SHA256：`DB6A8FDD6570FC4C54350F1CAFCCA70BCA401E0982715F1D6251BC2CD14D2AFC`。
- 交付 `Sci_Fi_Soldier_Rigged.blend`、直接 Max FBX、接回材质的 `Sci_Fi_Soldier_Rigged_PBR.fbx` 和18张贴图。原V-Ray插件缺失，材质按原Blend面索引与贴图重建；AO/位移仅随包保留。武器仍为独立未绑定网格。
- Blender：59 个总骨骼、56 个有效权重组，未绑定顶点和未解析组均0；6项临时关节旋转均产生变形，恢复误差约6.7e-8米。最终FBX重新导入最大近邻点误差3.26e-6米，51,879顶点和59骨骼保留。
- 两张预览已查看，姿势测试图已用系统图片查看器打开。最终Blend保存原姿势，测试姿势未写入交付文件。
- 更正此前仅根据文件摘要的判断：Max实际时间轴0-100帧，逐帧对象变换无变化；未恢复走跑/射击动作。交付是CAT原绑定恢复，尚未做人形模板适配或UE导入。
- 证据：外部目录内 `max_inspection.json`、`max_export.json`、`validation.json`、`roundtrip.json` 和日志。无需C++构建或PIE；UE目录整理 active feature 不变。结果未提交或push。
