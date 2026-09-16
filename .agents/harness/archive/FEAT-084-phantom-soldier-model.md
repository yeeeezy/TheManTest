# FEAT-084 Phantom 科幻士兵外观

## 用户反馈待修（2026-09-15，明确暂缓）

- 用户最新截图`C:/Users/ROG/Pictures/Screenshots/屏幕截图 2026-09-15 225133.png`显示鞋底有离地观感；尚未测量运行时鞋底、胶囊底及地面高度，具体成因未确认。之前动作验证未充分覆盖接地外观。
- 用户认为腿太细；需对照源模型区分原比例和蒙皮适配的影响。下次优先评估腿部体积、膝踝轮廓及脚底接地，先提供对比预览，再接入修订。未确认改用重定向。
- 用户明确“先这样，这个先不急，记下来下次修”；保留现状，本轮不改模型/蓝图/动画。

## 2026-09-15 用户授权与范围

- 用户选择将本地A0100绑定到Phantom现用骨架、复用现有动画，并明确要求动手接入。
- 操作前WIP：`551ff37`，仅保存上一轮A0100恢复交接；未push。
- 外部工作目录：`D:\Blender Projects\PhantomSoldier`。未执行IK或动画重定向。现有动画只读导出用于蒙皮验证。
- 冷读取BP_Phantom：原SK_Mannequin、UE4_Mannequin_Skeleton、ABP_Phantom_OriginalRifle；组件位置Z=-90、Yaw=-90、缩放0.9。

## 实施

- 按原CAT权重和解剖对应关系将A0100参考网格适配到现有70骨层级，保留目标骨架比例。处理4节CAT脊柱到现有3节脊柱、手指及装备辅助骨权重映射。51,879顶点均有权重。
- 外部生成Phantom_Soldier.blend/SK_Phantom_Soldier.fbx；已查看瞄准、跑步、换弹、开火、受击预览。未改动动画曲线或制作重定向资产。
- 导入 `/Game/Enemy/Humanoid/Phantom/Meshes/SK_Phantom_Soldier`，使用原UE4 Skeleton及原PhysicsAsset；三套专属材质、12张实际使用贴图位于Phantom的Materials与Textures/Soldier。
- BP_Phantom仅替换Mesh并清理旧材质覆盖。原AnimBP、组件变换、武器和战斗代码保持。
- 修正Unreal Python结构体数组迭代副本导致材质槽未保存的问题；正在最终冷启动验证。

## 验证状态

- 首轮实际PIE：70骨名称顺序一致，5条动画逐骨位置误差约0.00012cm；原AnimBP运行、武器仍挂hand_r_wepSocket；隐身切换/恢复、布娃娃模拟通过。该轮材质为空，不能作为最终外观通过证据。
- TestMap仅使用未保存的临时验证对象，未新增地图；地图SHA256未变化。最终材质修复后的PIE、渲染检查、真实死亡回归及冷引用检查尚待完成。
- 无C++修改，无需重新构建。状态：in_progress；未最终提交或push。

## 最终完成（2026-09-15）

- 最终材质修复后重新启动实际TestMap PIE：五条原动画正常播放，70骨逐骨位置差小于0.000122cm，原AnimBP、hand_r_wepSocket挂枪保持。全部三槽材质存在，瞄准/跑步/换弹/开火/受击渲染已检查。
- 隐身与恢复通过并检查蓝色半透明截图。枪仍可见，属于既有SetCloaked仅替换身体材质的行为，本轮未修改玩法。
- PIE布娃娃模拟断言通过；UE_Ragdoll截图主体已移出构图，不作为外观证据。独立真实死亡回归TheManTest.Player.Weapons.EnemyDeathRagdoll为Success，覆盖实弹致命伤、爆炸、尸体冲量和清理。
- final_audit.json ok=true：16个新资产冷加载成功，三套材质依赖完整，原Skeleton/PhysicsAsset/AnimBP、组件变换正确，范围内无Redirector。原Skeleton SHA256=A26876F78160788D499C885D63DA5DBE962EE68481FCF6E05F1109316DF947E7。
- pie_validation.json ok=true/map_unchanged=true；临时对象已清理，编辑器退出，未保存TestMap。无C++修改，无需重新构建。
- 外部源文件、执行脚本、日志、JSON与预览统一在D:/Blender Projects/PhantomSoldier；仅正式网格/材质/贴图进入TheManTest。没有重定向或新增动画。
- 本轮完成，状态done，条目移入feature_archive.json；启动索引回到此前FEAT-083，不继续其他功能。结果未最终提交或push。
- 归档索引检查：FEAT-084全局仅一项。发现FEAT-080在两个索引重复，git HEAD已存在同样重复，非本轮引入；保留原状态供独立整理，两个JSON均可解析。
