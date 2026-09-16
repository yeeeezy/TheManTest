# FEAT-087 Lobby UI 与开始游戏

状态：done。

- 用户确认预览并授权落地：START / SETTINGS；SETTINGS 保留禁用。角色页采用名字、右箭头、TAB 切换、介绍及 VIEW WEAPONS。武器页增加左侧 START GAME，保留原布局。
- 复用 GameInstance.SelectCharacterAndStart 将展示 CharacterID 带入 TestMap。Weapon Back 回角色页，角色 Back 回菜单。
- 操作前 checkpoint 3870e05 保存上一轮执行官效果。
- 冷审计：三位角色都有独立 BP 与花名册引用，但执行官/潜伏者战斗 BP 的 Mesh、AnimClass、初始装备为空；不是维修工可玩副本。随后用户明确授权补齐临时配置（已完成，见下文）。
- 验证证据位于 D:/UnrealWork/LobbyUI。无新增测试地图或常驻验证设施。
- 首次编译发现 UButton 无 SetIsFocusable setter，已删除该调用；TAB 由父 Widget PreviewKeyDown 接收，不依赖子按钮禁用焦点。

- 用户已授权补齐执行官/潜伏者临时可玩配置。两者保留父类、ID、潜伏者扫描技能，复用维修工身体/手臂/腿/AnimBP/三枪；按维修工运行时参数设置移动速度550/750、加速度2000、减速度750。
- Development Editor Win64 构建通过。UMG 与两项角色 BP 编译保存成功。
- 实际 Windows TAB 事件已验证能选择执行官。导航九项检查通过；跨图已生成执行官/维修工，完整断言仍在继续。
- 修正验证脚本 Python API 名称（get_controlled_pawn、color_and_opacity 属性）与异步截图时序；这些为验证脚本错误，不冒充通过。
- 实际截图发现角色短线保留灰色（脱离 Tree 时 FindWidget 找不到），改为 RuleSize.GetContent；角色名字行高恢复1.0，并校正箭头垂直位置。

- 跨图完整验证 travel.json ok=true：执行官通过 Lobby START GAME 进入 TestMap；潜伏者与维修工通过原有 GameInstance 接口进入。三者独立 BP、ID、身体/手臂动画、三枪装备与输入模式正确；执行官真实移动和切枪通过，潜伏者扫描技能配置保留。
- 鼠标外部注入验证尚未通过：隐藏编辑器取得零尺寸控件几何；不能据此宣称实际鼠标点击通过。已验证 UFUNCTION 导航和实际 TAB 输入。最终截图采用启动沉浸 PIE，避免控制台切换后截到编辑器视口。

## 完成（2026-09-16）

- ui_pie.json ok=true：十项导航、三枪切换、返回层级及维修工 START GAME 实际跨图验证通过，LobbyMap/TestMap 文件哈希不变。
- preview.json ok=true：实际 TAB 输入通过；Final_Main/Character_Worker/Character_Executive/Weapon_Worker/Weapon_Executive00002.png 为最终渲染结果，已检查金色短线、名字/箭头位置与左侧 START GAME。
- Development Editor Win64 冷构建成功，WBP_LobbyPresentation、BP_FPSTheExecutive、BP_Infiltrator 编译保存成功；三角色跨图完整证据见 travel.json。
- 已移入 feature_archive.json，active_feature=null。本次结果不自动提交或 push。正式角色战斗外观及设置页不在本轮范围。


## F11 景深排查（2026-09-16，未保存修复）

用户反馈 F11 与窗口模式角色清晰度/背景虚化不同，授权排查。外部脚本 D:/UnrealWork/LobbyFocus，diagnosis.json 和 isolation.json，实际 Win32 F11 输入、景深开启。

- 复现角色页全屏人物糊、窗口显得清楚；角色镜头40mm、f/2.8、ManualFocusDistance=5898.890625cm（约59米），F11来回参数不变；人物胸部轴向距离约410.27cm。当前地图焦平面明显偏离角色。
- 武器镜头42mm、f/2.8、ManualFocusDistance=731.58496cm，切换F11参数同样不变；本轮矫正对照仅覆盖角色页。
- F11前后 viewport2560x1528 /1543x761，景深质量2、后处理质量2、r.ScreenPercentage87、动态分辨率0保持。不是F11改变了相机对焦数值。
- 临时ShowFlag.DepthOfField=1后窗口观感仍清楚，未支持“窗口景深开关关闭”的猜测。UE景深渲染包含像素半径阈值，但尚未单独隔离内部渲染分辨率/宽高比对差异的贡献，不宣称已证明全部F11渲染机制。
- 仅在PIE副本临时把源镜头手动焦距设至人物胸部轴向平面约410.27cm，保持景深开启；全屏人物恢复清晰、背景虚化，CharacterFocus_Immersive00000.png已检查。正式修复建议使角色页焦平面落在当前角色，武器页分别校准。
- 未修改C++或保存任何资产/地图。开始检查时LobbyMap.umap已存在用户未提交改动，应保留；此前FEAT087结果也仍未最终提交。验证脚本第一次因UTF8 BOM解码失败，已修正并重跑，不影响项目。
