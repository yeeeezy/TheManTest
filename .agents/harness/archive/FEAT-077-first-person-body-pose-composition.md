# FEAT-077 — 第一人称手臂与完整身体双 AnimBP 合成

**创建日期：** 2026-08-21
**状态：** needs_improvement

> 2026-09-01 session244：用户转入 Combat HUD/玩家弹药工作；FEAT-077 保留待前台动画主观复核，不再作为 active feature。

## 2026-09-01 session252 — 修复第一人称 JumpStart 无法进入空中循环

- 只读核对外部原版 VFXPack：丝滑跳跃由 `FirstPerson_JumpStart`、`FirstPerson_JumpLoop`、`FirstPerson_JumpEnd` 三段骨骼动画和 `IsFalling` 驱动的状态机混合完成，不是角色蓝图 Timeline 或骨骼物理。
- 本项目已经拥有并实际引用迁移完成的 `AS_MaintenanceWorker_FP_JumpStart/JumpLoop/JumpEnd`，无需重新迁移或重定向。
- 根因是 `TABP_CharacterBase_FirstPerson` 的 JumpStart Sequence Player 错误开启 Loop，同时 JumpStart→JumpLoop 使用自动剩余时间过渡，导致该自动过渡失效并产生编译警告。
- 已将 JumpStart Loop 关闭；JumpLoop 保持循环，JumpEnd 保持不循环。模板和 `ABP_MaintenanceWorker_FirstPerson` 均在 UE 内编译保存，自动过渡警告消失；最终跳跃观感待用户前台 PIE 复核。

## 2026-09-01 session253 — 武器基类保留持枪跳跃 Pose

- 排查确认角色侧 `bIsFalling/Is_InAir/Velocity_Z` 驱动及三条 Jump Asset Override 均存在；实际可见输出被 `TABP_FirstPersonFirearmBase.WeaponUpperBody` 只有 Idle/WalkRun 的完整 Pose 覆盖。
- 在共享武器基类 `WeaponUpperBody` 内新增 `bIsFalling` Pose 选择：地面继续使用原武器 Idle/WalkRun 状态机；空中使用 `UpperBodyInPose`，保留宿主已经播放的 VFXPack 持枪 JumpStart/JumpLoop/JumpEnd。
- `ABP_RepairGun_FirstPerson` 继承该逻辑，无需复制状态机或重新迁移动画。模板和 RepairGun 子 AnimBP 在 UE 内编译保存成功；Development Editor / Win64 构建成功。待用户前台 PIE 验证起跳、坠落与落地观感。

## 2026-09-01 session254 — 删除旧宿主空中旁路生成代码

- 删除 `ConfigureFirstPersonFirearmLinkedLayer()` 在武器 Layer 已存在时向宿主 AnimGraph 插入 `BlendListByBool + bIsFalling` 的旧分支，以及全部 `FP linked jump fallback` 日志。
- 该一次性编辑器迁移路径不参与运行时，并与 session253 已落在 `TABP_FirstPersonFirearmBase.WeaponUpperBody` 内的正式空中选择职责重复。
- 保留新建 Linked Layer 的原始能力，以及 `ConfigureFirearmUpperBodyAirbornePassThrough()` 对现有武器基类的专用配置能力。残留关键字扫描为0；Development Editor / Win64 构建成功。

## 2026-09-01 session255-256 — 回退破坏性节点删除并修复 Jump Transition 驱动

- session255 曾错误删除 `TABP_CharacterBase_FirstPerson` 资产中已有的顶层空中 Blend，导致 `WeaponUpperBody` Pose 链断开、整个上半身无动作。用户前台发现后，已从安全检查点 `3be9fb8` 精准恢复该 AnimBP；旧旁路生成代码仍保持删除。
- 自测 PIE 确认恢复后地面 Idle/持枪上半身正常；强制腾空时 AnimInstance 的 `bIsFalling=True`、`Is_InAir=True`，但 Jump Transition 仍未正确消费空中状态。
- 图表审计发现 3 个 Jump Transition Getter 仍引用模板化前的 `Is_InAir`。已在保持所有 Transition Pin 连线的前提下改绑到原生 `bIsFalling`，模板及维修工/武器相关 4 个 AnimBP 冷启动编译成功。
- 动画资产采样：JumpStart 22帧、JumpLoop 35帧、JumpEnd 14帧；JumpLoop 本身非常接近 Idle，明显变化主要集中在 JumpStart/JumpEnd。PIE 强制腾空验证 `bIsFalling/Is_InAir` 与关键手臂骨骼均发生变化；一次性变量迁移工具已删除，Development Editor 构建成功。

## 2026-09-01 session243 — 按最新截图更新 ArmsViewMesh 默认构图

- 用户再次前台微调后，将 `ArmsViewMesh` C++ 默认 Location 更新为 `(-14.766994,-4.322017,-134.599387)`，默认 Rotation 更新为 `(Roll=5.752239°, Pitch=0.077753°, Yaw=-114.908449°)`；Scale 保持 `(1,1,1)`。
- 不改变静态 Transform 的职责边界：C++ 只提供构造默认，BeginPlay/Tick 不覆盖，具体 BP 可继续覆盖。
- Development Editor 构建成功；冷启动 Native CDO 与当前 `BP_MaintenanceWorker` CDO 均在0.001容差内匹配新截图数值。

## 2026-09-01 session242 — 当前 ArmsViewMesh 构图固化为 C++ 默认值

- 按用户最新截图将 `ArmsViewMesh` 默认 Location 固化为 `(-6.330288,-6.449130,-141.685038)`，默认 Rotation 固化为 `(Roll=0.581459°, Pitch=0.297436°, Yaw=-117.090375°)`。
- 仍不在 BeginPlay/Tick 覆盖静态 Transform；具体角色蓝图可以继续覆盖。当前 `BP_MaintenanceWorker` 原有值与新父类默认完全一致，因此无需改写其 `.uasset` 即可保持画面。
- Development Editor 构建成功；冷启动回读 Native CDO 与 `BP_MaintenanceWorker` CDO 的 Location/Rotation 均与截图值在0.001容差内一致。

## 2026-09-01 session241 — 手臂 Mesh 统一到正式动画 Skeleton

- 用户纠正方案：不恢复短名称动画、不配置 Compatible Skeleton、不从外部项目重新迁移；直接将 `SKM_MaintenanceWorker_FirstPersonArms` 的 Skeleton 从 `SKEL_MaintenanceWorker_FirstPersonArms` 改为正式动画使用的 `SK_Mannequin_Arms_Skeleton`。
- 写入前通过 UE 5.7 `SkeletonModifier` 比较两侧参考骨架：均为68骨，逐骨名称与父骨关系完全一致；随后使用 Editor-only `USkeletalMesh::SetSkeleton` 保存资产，临时工具代码已移除。
- 冷启动回读确认 Mesh、六条 `AS_MaintenanceWorker_FP_*`、`BS_MaintenanceWorker_FP_WalkRun` 与 `ABP_MaintenanceWorker_FirstPerson` 全部指向同一 Skeleton；最终 Development Editor 构建成功。旧 `SKEL_MaintenanceWorker_FirstPersonArms` 暂不删除。

## 2026-09-01 session240 — 删除未使用的短名称第一人称动画副本

- 确认两套动画绑定不同 Skeleton：短名称 `AS_MW_FP_*` 绑定 `SKEL_MaintenanceWorker_FirstPersonArms`；正式完整名称 `AS_MaintenanceWorker_FP_*` 与 `ABP_MaintenanceWorker_FirstPerson` 绑定 `SK_Mannequin_Arms_Skeleton`。这解释了前者只会出现在对应 Skeleton 的兼容资产筛选中，但不代表运行时使用。
- Unreal Asset Registry 加载确认短名称六条 Sequence 与 `BS_MW_FP_WalkRun` 的外部引用均为0；按依赖顺序删除全部七个资产。
- 冷启动复查七个短名称目标均不存在；完整名称六条 Sequence、`BS_MaintenanceWorker_FP_WalkRun` 与 `ABP_MaintenanceWorker_FirstPerson` 全部加载成功。

## 2026-08-23 session239 — RepairGun 切换为 Skeletal Mesh 可见枪体

- `BP_RepairGun` 的继承 `SkeletalMesh` 组件指定 `/Game/Weapons/RepairGun/Meshes/SK_SCFRIFLE`。`AEquipmentBase::BeginPlay` 在蓝图覆盖落定后自动选择表现组件：有 Skeletal Mesh 时显示它，并递归隐藏旧 Static Mesh 与 `StaticMeshOverlay`；旧 Static Mesh 资产继续供 `ShadowStaticMesh` 复制世界空间投影。
- 不修改 SkeletalMesh Relative Transform，用户自行按 `GripPoint` 手调。`AFirearm::GetMuzzleWorldTransform` 已有 Skeletal Mesh Socket 优先、Static Mesh 次之、`MuzzleLocalTransform` 兜底，无需改弹道代码。
- 已确认但按用户要求暂不修复：RepairGun 描边无可见效果；装备消融无可见效果。
- Development Editor 构建成功；PIE 断言确认 Skeletal Mesh 已加载且可见、旧 Static Mesh 已隐藏，`PlayerFramingCurrent.png` 显示新枪体进入画面。FramingCapture 的旧 77° FOV 断言仍与用户当前蓝图值冲突。

## 2026-08-23 session237 — 实证定位蓝图组件预览与 PIE 姿势差异

- 临时自动化探针逐项记录蓝图生成类 CDO 与 PIE：`ArmsViewMesh.RelativeTransform` 完全相同，Rotation `(-0.087114,-82.335250,0.647280)`、Location `(-1.615754,0.000001,-141.738776)`，排除运行时代码覆盖组件 Transform。
- CDO 的 `Update Animation in Editor=false`，角色蓝图组件视口不会持续求值第一人称 AnimBP；PIE 则在 BeginPlay 链接装备动画层、主动 Tick/Refresh 初始 Pose，随后每帧正常求值。
- 对 `AS_MaintenanceWorker_FP_Idle` 第 0 秒进行逐骨组件空间采样：相对参考姿势，`upperarm_r=36.248°`、`lowerarm_r=109.692°`、`hand_r=72.553°`；PIE 最终姿势相对该 Idle 仅差 `0.585°/0.457°/0.610°`。所以截图中的大方向差确定来自编辑器参考姿势与运行时 Idle 的差异，不是统一 Idle 被额外旋转。
- 探针完成后已从源码移除。测试同时暴露用户当前蓝图 FOV 与旧自动化 77° 硬编码断言不一致；这与姿势根因无关。

## 2026-08-23 session236 — 修正为近相机裁切并按手肘遮挡调参

- 用户最新截图证明 session235 的裁切方向反了：需要删除贴近相机的肘部，而不是删除远端手臂。材质公式改为 `(Distance - Arm Near Clip Distance) / Arm Near Clip Fade Width`，经 Saturate 与 `DitherTemporalAA` 输出 Opacity Mask。
- 参数重命名为 `Arm Near Clip Distance` 与 `Arm Near Clip Fade Width`，实例最终使用 40cm / 8cm。独立手臂证据图 `TMT_FPArmNearClip_ArmsOnly_40cm.png` 显示近处肘部主体消失且双手/前臂仍保留；正常持枪 `PlayerFramingCurrent.png` 不再出现整块手肘遮挡。
- 正常 DX12 冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success。额外的 `-NullRHI` 截图尝试因截图命令依赖视口而崩溃，随后已用正常渲染路径成功复跑。

## 2026-08-23 session235 — 第一人称手臂相机距离裁切材质

- 只读审计确认 `SKM_MaintenanceWorker_FirstPersonArms` 只有一个材质槽，已绑定专属 `MI_MaintenanceWorker_FirstPersonArms`，其父级为专属 Masked 材质 `M_MaintenanceWorker_FirstPersonArms`；完整身体/影子不共用该材质。
- 主材质新增 `Distance(CameraPositionWS, AbsoluteWorldPosition)` 距离链；本次最初使用了方向错误的 `Arm Clip Distance - Distance`，该错误已由 session236 修正并替换参数。
- 新增 `TheManTest.Player.Viewmodel.ArmDistanceClipEvidence` 截图测试：仅在证据捕获时临时隐藏武器、关闭 Arms `OnlyOwnerSee` 并补光，捕获后立即恢复，不改正式游戏状态。
- 当时的 180cm/1cm 对照只验证材质链会改变可见性，没有验证裁切方向；用户实拍随后证明该结论不足，正式参数与公式以 session236 为准。
- 写入前检查点 `0d0a33b`；Development Editor 完整构建成功，最终冷启动 `ArmDistanceClipEvidence` 1/1 Success，无 `M_MaintenanceWorker_FirstPersonArms` 编译错误。仍存在本次之前的 `M_UE4Man_Body` 材质层缺纹理警告，与新专属手臂材质无关。

## 2026-08-23 session234 — 移除冗余 SprintPivot，Arms Transform 独占静态构图

- 用户明确所有静态构图只调 `ArmsViewMesh.Transform`，不会用 `ViewmodelRoot` 调静态位置/旋转；因此删除与 Root 原点相同的冗余 `SprintPivot`。
- 最终层级恢复为 `HeadCamera -> ViewmodelRoot -> ArmsViewMesh`。C++ Tick 仅将实际速度驱动的冲刺 Pitch 写入 `ViewmodelRoot`；不读写 `ArmsViewMesh` 的 Location/Rotation，其蓝图 Transform 是唯一静态构图值。
- 自动化已回归断言 Arms 直接挂 Root、Root 位于相机原点且原地 Shift 不旋转。写入前检查点 `8331987`；`TheManTestEditor Win64 Development` 完整构建成功，冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 1/1 Success。

## 2026-08-23 session233 — 恢复蓝图组件 Transform 权威

- 按用户最终决定删除 `ViewmodelOffsetLocation` / `ViewmodelOffsetRotation`；C++ 不再在构造、BeginPlay 或 Tick 覆盖 `ViewmodelRoot` 和 `ArmsViewMesh` 的静态 Location/Rotation，蓝图组件面板 Transform 同时成为预览与运行时权威值。
- 组件层级改为 `HeadCamera -> ViewmodelRoot -> SprintPivot -> ArmsViewMesh`。新增的 identity `SprintPivot` 只承载实际速度驱动的冲刺 Pitch，避免动态逻辑改写用户构图 Transform。
- Framing 自动化已改为验证新层级和 `SprintPivot` identity/原地冲刺不下压，不再硬编码断言用户可编辑的 Arms Transform。
- 写入前检查点 `2bdee43`。用户关闭编辑器后，`TheManTestEditor Win64 Development` UHT/编译/链接成功。冷启动 `TheManTest.Player.Viewmodel.FramingCapture` 首轮暴露旧的世界横轴硬编码断言与用户可编辑 Transform 冲突；删除该过时断言后重编译并复跑 1/1 Success。用户未提交的蓝图/地图改动未覆盖。

## 2026-08-23 session232 — 构图旋转统一到相机中心支点

- 按用户确认删除 `BaseArmsRotation` / `Viewmodel Arms Rotation`，不再依赖导入手臂 Mesh 自身原点调整静态旋转。
- `Viewmodel Offset Rotation` 改为 `EditAnywhere`，默认值合并为原 Arms 基础朝向 `(-3,-90,-1)`；PIE Tick 每帧直接应用到相机中心的 `ViewmodelRoot`，并在此基础上叠加冲刺 Pitch。
- `Viewmodel Offset Location` 仍每帧写入子组件 `ArmsViewMesh`；Arms 自身 RelativeRotation 固定为零。这样不同导入模型共用相机中心旋转支点。
- 写入前检查点 `c7f1532`；`TheManTestEditor Win64 Development` UHT、编译与链接全部成功。待用户前台 PIE 手调并确认最终值。

## 2026-08-23 session231 — PIE 实例构图每帧直接覆盖

- 纠正 session229 对用户要求的误解：删除 `PostEditChangeProperty` 变化判断，不再依赖编辑器属性回调。
- Tick 每帧无条件将 `ViewmodelOffsetLocation` 和 `BaseArmsRotation` 写入 `ArmsViewMesh`，PIE 运行实例修改 Location/Rotation 后可在下一帧生效。冲刺 Pitch 仍独立叠加在父级 `ViewmodelRoot`。
- 写入前检查点 `8fb7f78`；`TheManTestEditor Win64 Development` 完整编译链接成功。用户未提交的 `BP_MaintenanceWorker.uasset` 和 TestMap ExternalActor 删除未被覆盖或纳入本次修改。

## 2026-08-22 session230 — 未解决构图问题停机交接

- 用户明确确认当前枪械位置/朝向构图问题仍未解决，计划次日在 PIE 运行实例中手动调整 `Viewmodel Offset Location` 与 `Viewmodel Arms Rotation` 后复核。
- 测试前需清零旧的父级 `Viewmodel Offset Rotation`；保留当前未提交的 `BP_MaintenanceWorker.uasset` 手调，不覆盖、不提交。
- 本次只记录状态，没有新增实现或验证；FEAT-077 继续保持 `in_progress`，FramingCapture 待手调完成后重跑。

## 2026-08-22 session229 — PIE 实例即时构图编辑

- Location 与 Arms Rotation 改为实例可调；编辑器 `PostEditChangeProperty` 仅在用户修改属性时更新 Arms Transform，Tick 保持无静态构图检测/覆盖。
- 检查点 `0b54ba2`；Development Editor 冷构建成功，用户 BP 手调资产未覆盖。

## 2026-08-22 session228 — Arms 自身旋转参数暴露

- `BaseArmsRotation` 改为 `Viewmodel|Framing` 下显示的 `Viewmodel Arms Rotation`，保持默认 `(-3,-90,-1)`，用于不改变组件 Location 的 Arms 自身静态朝向调整。
- 检查点 `3af3217`；Development Editor 冷构建成功，Shadow/EquipDissolve Success。FramingCapture 因用户 BP 中仍保存旧父枢轴 Rotation/Location 手调而失败，待用户重置旧 Rotation 并改用新参数后复核；未覆盖用户 `.uasset`。

## 2026-08-22 session227 — 实际速度驱动冲刺压枪

- `SprintTransitionAlpha` 仅负责0.2秒 MaxWalkSpeed 切换；`ViewmodelRoot` 压枪改由实际水平速度在550..750映射出的 `SprintVisualAlpha` 驱动。
- 原地 Shift、受阻或速度不超过 WalkSpeed 时不压枪；专项运行时断言通过。写入前检查点 `102ffab`，Development Editor 冷构建及3项玩家回归全部 Success。

## 2026-08-22 session226 — 静态构图停止逐帧覆盖

- ViewmodelRoot 静态位置、Arms 的 Offset Location/Base Rotation 改为 `BeginPlay` 应用一次；Tick 只更新冲刺动态 Pitch 和 AnimBP 输入。
- 写入前 WIP 检查点 `2ba4834`。首次冷链接因运行中的 Unreal Editor 锁定 DLL 报 LNK1104；关闭编辑器后 Development Editor 冷链接成功，3项玩家回归全部 Success。

## 2026-08-22 session225 — Body Sway 插值速度参数化

- 新增 `ViewmodelBodySwayInterpSpeed`（`Viewmodel|Movement`，默认 `6.0`），替代普通移动硬编码 `2.0`；进入倾斜和松键回弹同步加快。冲刺仍使用原版 `8.0`。
- 写入前 WIP 检查点 `2e3b5ad`；Development Editor 冷构建及3项玩家回归全部 Success。

## 2026-08-22 session224 — 恢复用户认可的 session220 表现

- 按用户决定精确恢复 `e1c24eb` / session220 的完整75° Lean/Look 骨骼映射，保留当时倾斜观感及其已知上下平移。
- 删除随后加入的装备 Actor 独立 Roll、Arms 位置补偿、输入符号修正和专项测试；三份相关源码与目标检查点定向比较一致。
- Development Editor 冷构建及恢复后的3项 `TheManTest.Player` 回归全部 Success。

## 2026-08-22 session223 — A/D 倾斜符号纠正

- session222 只验证旋转幅度与枪口稳定，遗漏视觉方向；用户前台发现方向相反。
- 按原版运行记录 A→正 Lean、D→负 Lean，将侧移目标改为 `-MoveInput.X`。专项测试新增 D 输入 `Lean_Sides_Amount < -5°` 断言，并以真实 D 输入截图对照原版 `TMT_VFXPack_StrafeRight.png`，两者枪身均为左上→右下。
- Development Editor 构建及全部4项 `TheManTest.Player` 回归 Success。

## 2026-08-22 session222 — VFXPack 骨骼倾斜与枪口枢轴补偿

- 撤销 session221 的装备根节点独立 Roll；按原 VFXPack 恢复 `spine_03` Component Space Additive Roll、`hand_l` 半倍率 Roll，以及 `hand_r/GripPoint` 随骨骼 Pose 带枪。
- 删除 75° Roll/Pitch 交叉映射，纯 A/D 不再写入 `Look_Up_Amount`。专项审计先复现无补偿时枪口约 `-18.6cm` 高度弧线，再以中性 `spine_03→muzzle` 向量计算圆弧并补偿 Arms 构图位置。
- 最终自动化实测 `spine_03=6.92°`、`hand_r=7.82°`、枪口高度变化 `-0.006cm`；装备 Actor 相对 Transform 不变。Development Editor 构建及全部4项 `TheManTest.Player` 回归 Success，待用户前台 PIE 观感验收。

## 2026-08-22 session220 — 恢复原枪体绕轴旋转

- 用户澄清枪体必须保持原地，目标是保留修改前沿枪管前向轴的倾斜旋转；session218/session219 的位置滞后与旋转拆分属于误解，已全部撤回。
- 玩家源码及测试精确恢复至 `e1c24eb`：ViewmodelRoot 零位移，完整 75° Lean/Look 映射恢复。临时最终枪体旋转探针及错误位置专项测试均已删除。
- Development Editor 构建和三项玩家动画回归通过。

## 2026-08-22 session219 — 侧移 Roll 幅度恢复

- 修正 session218 遗留的侧移 Roll 衰减：不再乘 `cos(75°)`，恢复原始 `±8°` 绕枪轴倾斜；A/D 到 Look/Pitch 的串扰继续保持为0。
- MovementLagDirections 现同时断言平滑后 Roll >5°、Pitch <0.05°；构建、专项测试和三项玩家动画回归均通过。

## 2026-08-22 session218 — 移动位置滞后与骨骼旋转解耦

- 隔离采样确认 A/D 上下偏移主要来自侧移 Lean 经 75°基差交叉写入 Look/Pitch；WalkRun 仅贡献约0.23cm，组件 Location 原本没有任何移动滞后。
- 正式实现保留侧移的枪械绕轴 Lean/Roll，只删除侧移到 Look/Pitch 的交叉项；新增独立 `ViewmodelRoot` 相机局部 XY 反向位置滞后，Z 永远为0。默认 A/D 3cm、W/S 2cm、插值速度6。
- 新增 MovementLagDirections 自动化；四向符号、零Z、Roll保留、Pitch隔离均通过，原三项玩家动画回归及 Development Editor 构建也通过。

## 2026-08-22 session217 — 枪械 WalkRun 专用 Blend Space Player

- 按用户审查纠正 session216：枪械模板 Idle 使用空 Sequence Player，WalkRun 使用空 Blend Space Player，不能用 Run Sequence 代替移动 BlendSpace。
- WalkRun 的 `Speed` 已连接 Blend Space X；RepairGun 子类通过 Parent Asset Override 指定 `BS_WalkRun_RepairGun`，模板不直接引用该具体资产。
- 冷导出结构审计、Development Editor 构建和三项玩家动画冷启动回归全部通过。

## 2026-08-22 session216 — 枪械模板断线节点清理

- 冷导出确认 `TABP_FirstPersonFirearmBase` 的 Idle、WalkRun 实际输出均为无具体资产的 `Sequence Player`；WalkRun 内另有旧方案遗留且完全断线的 `Blend Space Player` 和 Speed Getter。
- 删除上述两个孤立节点。`ABP_RepairGun_FirstPerson.ParentAssetOverrides` 继续分别为两个 Sequence Player 提供 Idle、Run 资产，模板对 RepairGun 目录具体资产依赖为 0。
- Development Editor 构建成功；三项玩家动画自动化均通过。截图型测试须使用渲染冷启动，不能使用 `-nullrhi`。

## 目标

- `ArmsViewMesh` 独立运行第一人称 AnimBP，不接收身体 root/pelvis locomotion。
- `CharacterMesh0` 保留完整身体 locomotion，并在自身 AnimBP 中从上半身混入 `ArmsViewMesh` 的最终局部骨骼 Pose。
- `CharacterMesh0` 继续作为唯一完整隐藏影子来源，不增加第三个 Shadow Mesh 或第三套动画职责。

## 已确认方案

1. `ArmsViewMesh` 使用独立 `ABP_CharacterBase_FirstPerson`。
2. `CharacterMesh0` 使用 `ABP_CharacterBase_Body` / `TABP_BodyLocomotion` 完整身体状态机。
3. Body AnimBP 以自身 locomotion 为 Base Pose，以 `Copy Pose From Mesh(ArmsViewMesh)` 为 Blend Pose，从 `spine_01`（最终以骨架审计为准）以上执行 `Layered Blend Per Bone`。
4. 不复制第一人称 root/pelvis；第一人称组件的相机空间 Transform 不进入身体影子。
5. 通过组件 Tick prerequisite 保证 Arms 先求值、CharacterMesh0 后合成。

## 2026-08-21 session210

- 用户确认按上述双 AnimBP 合成方案实施。
- 写入前检查点：`b382f95`。
- FEAT-076 自动化已完成但仍待用户主观验收，状态改为 `needs_improvement`；本功能成为唯一活动功能。

## 实施结果

- `UFPSCharacterAnimInstance.FirstPersonPoseSource` 在运行时指向角色的 `ArmsViewMesh`。
- `TABP_BodyLocomotion` 最终输出新增 `Copy Pose From Mesh -> Layered Blend Per Bone(spine_01)`；复制使用局部骨骼空间，不接收 Arms 组件的相机空间 Transform，root/pelvis 保留 Body locomotion。
- `ArmsViewMesh` 固定先于 `CharacterMesh0` Tick；后者再合成完整影子，避免上一帧 Pose。
- `BP_MaintenanceWorker` 明确分配两类：Body=`ABP_CharacterBase_Body_C`，Arms=`ABP_CharacterBase_FirstPerson_C`。
- Equipment Linked Layer 与 Montage 仅进入 Arms AnimInstance；Body 不再重复 Link/Unlink 或重启武器层。
- 自动化增加 Body/Arms 不同类、Copy Pose 来源指针、`spine_03/hand_r/hand_l` 局部旋转一致性断言。

## 验证

- `TheManTestEditor Win64 Development`：Success（三轮增量/冷链接均通过）。
- `TheManTest.Player.Shadow.UpperBodyEvidence`：Success。
- `TheManTest.Player.Viewmodel.FramingCapture`：Success。
- `TheManTest.Player.Viewmodel.EquipDissolveEvidence`：Success。

## 2026-08-22 session221：枪械前轴 Roll 独立于骨骼滞后

- A/D 的上下漂移确定来自 AnimBP 上游骨骼 Modify Bone 旋转使 `GripPoint` 沿弧线位移，而不是 ViewmodelRoot/ArmsViewMesh 的组件 Location。
- Lean/Look 骨骼滞后输入暂时关闭；A/D 改为只对当前装备根节点施加即时本地 X/Roll `±8°`，同时把该根节点相对 Location 固定为零。
- 自动化实际注入左右输入并断言 Location=0、Roll=`-8°/+8°`；Development Editor 构建及三项玩家回归均 Success，待用户前台观感验收。
- 待用户前台实际输入复核切枪、Idle/移动/跳跃/开火/冲刺观感后决定是否关闭。

## 正式目录与命名整理

- 第一人称正式运行类移除素材来源名：`ABP_VFXPack_FirstPerson` → `ABP_CharacterBase_FirstPerson`。
- 身体正式运行类从技术性的 `Animations/Skeleton` 迁至功能目录：`Animations/Body/Logic/ABP_CharacterBase_Body`。
- 第一人称类保留于 `Animations/FirstPerson/Logic/`；无骨架身体模板继续位于共享 `Animations/Logic/`。
- 迁移通过 Unreal AssetTools 完成并修复引用；旧 `Animations/Skeleton` Asset Registry 为 0 且磁盘目录已删除。

## 2026-08-21 session211 — Linked Anim Layer 模板化与目录归属

- 第一人称常驻宿主改为 `ABP_MaintenanceWorker_FirstPerson`（`UCharacterBaseAnimInstance`），地面持枪 Pose 从 `ALI_WeaponAnim.WeaponUpperBody` 获取；当前枪械模板尚无 Jump 状态，因此 `bIsFalling` 时显式切回宿主原 Jump 状态机，避免相对改前退化。通用骨骼晃动继续位于最终输出端。
- 枪械模板改名 `TABP_FirstPersonFirearmBase`（`UFirearmAnimInstance`），维修枪骨架子类改名 `ABP_RepairGun_FirstPerson`。武器切换继续只替换 Linked Layer。
- C++ 层级统一为 `UBaseLocomotionAnimInstance -> UFPSCharacterAnimInstance -> UCharacterBaseAnimInstance -> UFirearmAnimInstance`；速度、方向、垂直速度、腾空、移动状态和通用 Lean/Look 不再由素材包 EventGraph重复计算。
- 角色专属 AnimBP 已从 `CharacterBase` 移至 `MaintenanceWorker/Animations/{FirstPerson,Body}/Logic`；无引用空模板 `TABP_CharacterBase` 删除。
- Development Editor 构建通过；三项冷启动自动化 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 全部 Success。

## 2026-08-22 session213 — 身体模板提升至 CharacterBase

- 纠正模板所有权：无骨架 `TABP_MaintenanceWorker_BodyLocomotion` 不绑定 MaintenanceWorker Skeleton 或动画资产，其职责是提供玩家身体 Locomotion 状态机与上半身 Pose 合成框架。
- 通过 Unreal AssetTools 改名并迁移为 `/Game/Characters/CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion`。
- `ABP_MaintenanceWorker_Body` 仍留在具体角色目录，继续承担 MaintenanceWorker Skeleton 与动画 Asset Override；迁移后已重新编译保存。
- 写入前 WIP 检查点：`a3da38b`。
- 新模板与具体 Body AnimBP 资产加载验证通过；旧路径 Asset Registry 为0且无 Redirector，新模板唯一确认引用方为 `ABP_MaintenanceWorker_Body`。
- 关闭编辑器后 `TheManTestEditor Win64 Development` 冷构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。

## 2026-08-22 session214 — 第一人称宿主模板化

- 新增无骨架 `/Game/Characters/CharacterBase/Animations/FirstPerson/Logic/TABP_CharacterBase_FirstPerson`，承载原宿主完整 AnimGraph、基础 locomotion/Jump、武器 Linked Layer 路由和最终 Lean/Look。
- `ABP_MaintenanceWorker_FirstPerson` 保留具体 Skeleton 与原资产路径，ParentClass 改为新模板生成类并删除本地 AnimGraph，成为纯具体角色子类。
- 冷审计确认模板 `TargetSkeleton=None` 且 AnimGraph 存在；子类 Skeleton 仍为 `SK_Mannequin_Arms_Skeleton`、本地 AnimGraph 为 None，并明确依赖模板。
- 写入前 WIP 检查点 `20be43d`；Development Editor 冷构建及三项玩家动画回归均 Success。

## 2026-08-22 session215 — 模板播放器占位与具体资产下沉

- 将 `TABP_CharacterBase_FirstPerson` 的状态机正式命名为 `FirstPersonLocomotionSM`。
- Body 与 FirstPerson 两个 CharacterBase 模板均只保留无具体资产的 `Sequence Player` / `Blend Space Player` 结构；冷启动依赖审计确认对 `/Game/Characters/MaintenanceWorker/` 的依赖均为 0。
- Body 模板原已采用子类 Asset Override，无需迁移。FirstPerson 模板的 7 个播放器绑定迁入 `ABP_MaintenanceWorker_FirstPerson.ParentAssetOverrides`：Idle、Still 两处、JumpStart、JumpLoop、JumpEnd 和 WalkRun BlendSpace。
- 因模板 Sequence Player 不再绑定资产，Jump 的 3 条剩余时间 Getter 改为状态机原生自动剩余时间过渡，模板与具体子类均重新编译保存。
- `TheManTestEditor Win64 Development` 构建成功；冷启动 `Shadow.UpperBodyEvidence`、`Viewmodel.FramingCapture`、`Viewmodel.EquipDissolveEvidence` 均为 1/1 Success。仍待用户前台主观复核。

## 2026-09-01 session257 — 第一人称完整 Locomotion 归属武器层

- 用户确认不同武器必须能整体替换 Idle、WalkRun 与 JumpStart/JumpLoop/JumpEnd，因此五状态状态机从 `TABP_CharacterBase_FirstPerson` 迁入 `TABP_FirstPersonFirearmBase.WeaponUpperBody`。
- 宿主模板删除本地状态机和重复空中 Blend，只保留 Linked Layer 输出与通用 Lean/Look；武器层删除旧的外层 `bIsFalling` 二次旁路。
- `ABP_RepairGun_FirstPerson` 持有 Still/WalkRun/JumpStart/JumpLoop/JumpEnd 的具体 Parent Asset Overrides；未来武器通过自己的 Linked Layer 子类替换整套动画。
- 新增 `WeaponOwnedLocomotion` 结构回归和 `WeaponOwnedJumpRuntime` PIE 回归；后者实际 LaunchCharacter，验证武器层收到空中状态且 `hand_r` 进入 JumpStart 后发生姿态变化。两项及 `Shadow.UpperBodyEvidence` 均 Success；Development Editor 构建成功。

## 2026-09-01 session258 — 武器 Locomotion 状态图清理

- 删除无当前职责的 `Reset Animation` 状态和 6 条相关转换，并将 Entry 明确接入 Idle。
- `Run / JumpStart / JumpLoop / JumpEnd` 重命名为 `Move / Jump Start / Fall Loop / Land`；新增 `Grounded` State Alias，同时代表 Idle 与 Move，并复用唯一的起跳转换。
- 冷启动回读确认状态图为 5 个实际状态、1 个 Grounded Alias、7 条转换，共14个节点且不再含 Reset Animation。
- `WeaponOwnedLocomotion`、真实 PIE `WeaponOwnedJumpRuntime`、`Shadow.UpperBodyEvidence` 均 Success；Development Editor 构建成功。

## 2026-09-01 session259 — 第一人称枪械纯位置观察滞后

- 按用户选择只恢复位置滞后：鼠标观察输入驱动 `ViewmodelRoot.RelativeLocation`，默认水平 1.8cm、垂直 1.2cm、进入速度 12、回正速度 16、死区 0.01；不添加任何旋转滞后。
- 删除已停用且不参与输出的 `bArmsPitchFollow`、`ArmsPitchFollowAmount`、`ArmsPitchInterpSpeed`、`CurrentArmsPitch` 与旧鼠标输入缓存。
- `PositionLagRuntime` 在 PIE 中验证右看左移、上看下移、X 轴不变、Yaw/Roll 不变以及停止后回零；跳跃和影子回归同时 Success。Development Editor 构建成功。

## 2026-09-01 session260 — 修正位置滞后覆盖原构图

- session259 首版把 `ViewmodelRoot.RelativeLocation` 直接写成滞后偏移，错误覆盖蓝图原有构图。现改为 BeginPlay 缓存 authored RelativeLocation，每帧只写“原构图 + 滞后偏移”，回正也回到原构图。
- 自动化断言同步改为保存实际运行时基准并比较相对变化；`PositionLagRuntime` 单独冷启动 Success，Development Editor 构建成功。

## 2026-09-01 session261 — 第一人称枪械位置弹簧

- 将机械式目标插值替换为鼠标 delta 速度冲量 + 欠阻尼弹簧回正；默认冲量 7/5、刚度 85、阻尼 12，并保留 1.8/1.2cm 位移上限。
- 按 Enhanced Input 实际轴向修正垂直符号，向上看时枪械先向下位移；仍只叠加在蓝图 authored 构图基准上，不添加旋转。
- Development Editor 构建与 `PositionLagRuntime` 冷启动 PIE 回归均 Success。

## 2026-09-01 session262 — 玩家可见文本英语规范

- 按用户确认项目为英语游戏，将当前所有 C++ 屏幕 Debug 文本统一改为英语；倒计时显示为 `[Round N | Phase N] Time Remaining: M:SS.s`。
- `UCharacterSelectWidgetBase` 的默认结束文案改为 `GAME OVER`，并扫描确认 Content 资产中没有对应中文序列化文本。
- 项目 harness 新增长期规则：玩家可见 UI、HUD、通知和屏幕 Debug 输出必须使用英语。Development Editor 构建成功。

## 2026-09-01 session263 — 手臂灰模与腿部近相机裁切

- 手臂裁切父材质新增 `Viewmodel Base Color=(0.18,0.18,0.18)` 并接入 Base Color，替代纯白输出；原 40cm / 8cm Opacity Mask 链保持不变。
- `SKM_MaintenanceWorker_LowerBody` 冷读确认只有一个材质槽；新增并绑定独立腿部 Masked 灰模实例，使用 55cm / 12cm 距离裁切。完整身体/影子仍使用原材质。
- 冷启动资产回读正确且无材质错误；`ArmDistanceClipEvidence`、`WeaponOwnedJumpRuntime` Success。`FramingCapture` 的失败仍为已知旧 FOV/GripPoint 断言，不属于本轮回归。

## 2026-09-01 session264 — 以原材质为基底重做相机裁切

- 用户指出手臂来自外部 VFXPack、腿部原材质也未缺失。只读源项目确认手臂原槽实际绑定 `MI_Placeholder_Lambert_INST`；其 Instance、Master、`MF_Disintegration` 与 `T_VFX_Noise_SoftPerlin_03` 四项完整依赖已通过 Unreal AssetTools 正式迁入。
- 手臂本地专属材质改为原 Master/Instance 的副本，完整保留浅灰金属参数及 Noise/Dissolve，40cm / 8cm Dither 裁切乘在原 Opacity Mask 后。
- 腿部专属材质改为项目现有 `M_UE4Man_Body` 的副本，保留其原图并只追加 55cm / 12cm 裁切；撤销 session263 的统一灰模外观。
- 冷启动绑定/参数审计无错误；正常渲染 `ArmDistanceClipEvidence` 与 `WeaponOwnedJumpRuntime` 均 Success。
