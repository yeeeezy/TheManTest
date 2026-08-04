# FEAT-075 — Nightmare FlyingBug2 程序化游荡怪

**状态：** done

**创建：** 2026-08-04

**关闭：** 2026-08-04

## 实现

- 保留空的 `ANightmareEnemy : AEnemyBase` 基类；新增具体 `ANightmareFlyingBug`。
- 启用 UE 5.7 Locomotor 插件并增加 `Locomotor` 模块依赖；具体虫使用 `FVectorDamper` 平滑程序化飞行速度，在可调半径/高度内随机三维游荡。
- 从批准的 TMIIR 项目只迁移原生最终 Mesh、Skeleton、PhysicsAsset、skin1 材质/纹理和 walk1 动画；未在 TheManTest 创建 IK Rig、Retargeter、源骨架或重定向工作目录。
- 正式资产整理到 `/Game/Enemy/Nightmare/FlyingBug2/{Blueprint,Animations,Mesh,Materials,Textures}`；供应商目录、Redirector、未使用 idle/back/strafe 动画均已删除。
- `BP_NightmareFlyingBug2` 继承具体 C++ 类；`RoamAnimation` 显式引用 walk1，BeginPlay 使用 `PlayAnimation(..., true)`，不依赖编辑器预览状态。

## 验收

- Development Editor 冷构建成功。
- PIE 读回 `MOVE_Flying`、约 220 cm/s 速度且位置持续变化；运行时 SingleNode Asset 为 walk1。
- 间隔 0.7 秒的骨骼采样中 Hips/tail1-tail7 位置与旋转均发生变化，证明循环动画实际播放。
- 最终目录 10 个必要资产、0 Redirector；目录资产验证通过；截图：`Saved/Screenshots/WindowsEditor/TMT_NightmareFlyingBug_Runtime.png`。
