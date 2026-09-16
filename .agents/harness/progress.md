# 当前工作面板

- Active feature：FEAT-083 Enemy／Character资产目录整理；FEAT-080/081/082暂停。
- Unreal AssetTools已迁移468项：CoreMorph Manta网格154、Scorpion网格301、MaintenanceWorker旧路径资产12、Humanoid共享Cover蓝图1。
- Character同名资产经引用审计确认用途不同，分别保留；第一人称手臂专用套件归入`Materials/Layers/Arms`、`Materials/Source`与`Textures/Arms`。
- 独立冷启动已加载468项新路径资产，编译CoreMorph与Cover蓝图；旧路径、根目录散落资产和范围内Redirector均为0。

## 会话交接

- 2026-09-15用户授权的A0100外部骨架恢复已完成：`D:\Blender Projects\A0100_RigRecovery`，交付带绑定Blend/FBX与18张贴图；关节测试和FBX重新导入通过。原Max未改、未导入UE、未重定向；详情见 `archive/A0100-external-rig-recovery.md`。

- 地图整理已由本地WIP检查点`64f5d15`保存，未push。
- FEAT-083最终资产与harness改动仍未提交或push；`Saved/Codex`执行脚本和日志为本地忽略证据。
- 本轮未做动画重定向、IK Retargeter或动画生成。
