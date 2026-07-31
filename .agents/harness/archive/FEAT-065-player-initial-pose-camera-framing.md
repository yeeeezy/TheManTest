# FEAT-065 — 玩家初始持枪姿态与摄像机构图对齐

**状态：** done  
**创建：** 2026-07-31  
**完成：** 2026-07-31-session126  
**参考截图：** `C:\Users\ROG\Pictures\Screenshots\屏幕截图 2026-07-31 092924.png`

## 目标

在 110° FOV 下，将玩家初始持枪构图调整到参考截图：枪体位于右下、枪管向左上延伸、手臂自然进入画面，同时不改变 gameplay 观察点、不破坏身体/影子同步、武器 Socket、移动与开火。

## 实现

- 保持 `HeadCamera` 的位置、旋转继承和 110° FOV 不变；构造与 `BeginPlay` 继续强制 FOV，避免角色蓝图旧序列化值覆盖。
- 在 `AFPSCharacterBase` 增加可配置的 `ViewmodelOffsetLocation/Rotation`，由 `ApplyViewmodelFraming()` 在构造、`OnConstruction` 与 `BeginPlay` 统一应用到 `ViewmodelRoot`。
- 最终采用 `ViewmodelOffsetLocation=(0,0,-7)`、`ViewmodelOffsetRotation=(0,0,0)`：保留原始手臂/武器侧面轮廓与骨架导入旋转，只将整套 viewmodel 下移到参考高度。
- 没有修改 `ArmsViewMesh` 基础旋转、Skeleton、动画或重定向资产；装备仍由 `EquipmentManager` 挂到 `ArmsViewMesh` 的声明 Socket。
- 新增确定性 1920×1080 离屏截图命令：PIE 初始化后停止初始 Equip Montage，再从 `HeadCamera` 同位置、110° FOV 的 SceneCapture 捕获，避免不同动画相位污染构图比较。
- 截图测试同时断言 `HeadCamera -> ViewmodelRoot -> ArmsViewMesh`、FOV、最终 Transform、初始装备与 Socket 挂接。

## 迭代结论

- 大幅 X 平移会改变尺度并增加手部遮挡；拒绝。
- 大幅 Y 平移会产生明显视差，使枪身正对镜头、丢失参考图的侧面轮廓；拒绝。
- 根节点额外偏航会进一步放大透视问题；拒绝。
- 最终方案只做 Z=-7 的构图下移，枪口高度与参考一致，同时保持枪身、双手和装备 Socket 的可靠关系。

## 验证证据

- `TheManTestEditor Win64 Development`：Succeeded（UE 5.7，session126 最终构建）。
- `TheManTest.Player.Viewmodel.FramingCapture`：Success；结构、110° FOV、`(0,0,-7)`、零旋转、装备 Socket 全部断言通过。
- 最终 1920×1080 截图：`Saved/Screenshots/PlayerFramingCurrent.png`。
- 最终日志：`Saved/Logs/PlayerViewmodelRegressionFinal.log`，1/1 tests performed，Success。
- Phantom 完整回归同时通过，证明玩家构图改动未回归敌人系统：`Saved/Logs/PhantomFullRegressionFinal3.log`，7/7 Success。

## 约束确认

- 未移动 gameplay 相机，未把构图写入 `ArmsViewMesh` 基础校正。
- 未进行 IK Retargeter、动画重定向或生成中间骨架资产。
- 未修改用户已有的 MaintenanceWorker/TestMap External Actor 工作区改动。
