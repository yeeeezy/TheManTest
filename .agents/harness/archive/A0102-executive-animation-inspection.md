# A0102 执行官模型动作检查（2026-09-15）

- 用户指定执行官模型来源：`D:\游戏\游戏资产\模型\角色\A0102\A0102-科幻狙击手`。本轮只读检查，无资产导入或动画制作。
- Blender：三个Mesh，无Armature、顶点组、Action或NLA轨道；1–250时间范围不代表有动作。
- FBX：三个Mesh，无骨骼/蒙皮Deformer，无AnimationCurve或AnimationCurveNode；仅一个AnimationStack和Layer空容器，不能作为动作使用。
- Max：原文件有CAT骨架和Skin；加载时间范围0–100，逐帧采样101帧，所有对象变换均未变化。未发现可播放的持枪或Relax持枪动画。
- 素材附图展示持枪姿势，仅可作为外观/姿势参考，不能证明动作数据存在。C4D未通过原生应用检查；结论限已读取的Blend/FBX/Max。
- 证据：`D:\Blender Projects\A0102_Inspection\inspection.json`、`max_inspection.json`及检查脚本/日志。原文件未保存修改。
