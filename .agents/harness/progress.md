# 当前工作面板

- Active feature：FEAT-081 CoreMorph头领迁移，in_progress；FEAT-080暂停。
- 已迁入飞行、重组／风墙、八足／尾刺雷爆；Manta核心导弹GA已实现并验证。唯一Actor／ASC／Health，目标UE5.7.4，源UE58Blank5.8.2保持不变。第四批三枪／附着弹适配仍待实施。
- 用户最新要求以后统一复用预留测试按键。已确认 IA_Test 映射为键盘上方1；当前接入Manta导弹测试，冷编译及实际PIE按键链验证通过，待用户观感校验。

## 当前测试入口

- 打开TestMap并进入PIE，按1临时生成Manta及闭合飞行路线，由正式主BT驱动导弹GA；再按1清理。退出PIE或玩家解除占有也清理头领、AIController和路线，地图不保存摆件。
- 入口为ATheManPlayerController::HandleTestInput；保留TestSwitchCharacterAction旧序列化字段名以维持BP的IA_Test引用。以后替换此入口当前测试内容，不新建测试地图、相机或额外按键。长期规则已写入AGENTS.md及arch02。

## 最新清理结果

- 删除7张额外测试地图：CoreMorph×5、GASPTest、VFXTestMap；删除两份地图专属材质。当前只保留TestMap作为测试地图，LobbyMap为正式选角流程保留。
- TestMap删除58个测试摆件：4个人形怪、5个巡逻点、12个可破坏方块、1个交互物、1个验收门、35个Validation地形块。保留77个已加载基础Actor和141个WorldPartition描述符；原地形、天空、灯光、出生点、导航及保留Actor数值Transform不变。
- 删除CoreMorph五份Tests／Authoring cpp及Review h/cpp，共7文件；移除预览／复位函数、ReviewTarget和临时按键相机。正式主BT、路线组件、GA／GE／Cue和490个头领资产保留。Reassembly内部ResetPreview改名ResetAssembly。
- 删除2个依赖预放Phantom的测试和3个预放验收门测试；17个旧VFX房测试源码改用TestMap。其余回归保留，不在地图保存夹具。

## 当前正式技能

- 默认Flight／Reassemble，第一阶段Near=TailStrike、Far=MissileBarrage，共4份唯一授予。阶段选择技能集，形态决定激活条件，强度波次提供伤害倍率。
- 导弹基础20在逐枚发射时快照倍率；尾刺基础25在击地时读取倍率，各自GE范围伤害按ASC去重。主BT支持边飞边轰炸，再变形近战。专属Cue控制表现及清理。
- 已删除原V／T／M／R检查入口和专属地图；以后不能再按旧记录要求用户打开它们。正式Boss行为可通过其战斗组件bEnabled配置，测试Map保持基础环境。

## 验证

- map-cleanup-build.log：Development Editor Win64 Succeeded。
- map-cleanup-regression.log：7/7 Success，三枪配置／切换、准星射击、范围爆炸、原有人形怪布娃娃、两项保留的持久化回归。
- map-cleanup-cold-final.log/json：仅TestMap／LobbyMap、77保留Actor的Transform不变、141描述符、490头领资产、4唯一技能及Cue注册通过；BP_CoreMorphBoss冷编译保存。
- Registry和磁盘清场完成；源码／配置没有旧地图／CoreMorph临时入口引用。全部后台编辑器退出。
- reserved-input-final-build.log：Development Editor Win64 Succeeded。reserved-input-key-verified.log／reserved-input-pie.json：RESERVED_INPUT_PIE_OK，经过引擎One按键模拟→IMC_Default→IA_Test→Controller，确认4个预警圈、导弹／爆炸实例、重按清理及再次启动、活动导弹退出PIE；玩家Pawn未切换，TestMap文件哈希不变。

## 会话交接

WIP `38a495d` 保存已完成的地图／测试代码清理；更早的 `f719242` 保存清理前完整Manta技能／检查设施。本轮预留1键修改未提交／push，无Content改动，后台编辑器已退出。用户现在可在TestMap的PIE按1校验导弹观感；以后统一复用该入口。外部脚本 `cleanup_maps.py` 已执行，不要重跑；按键验证脚本为 `D:/Unreal Projects/CoreMorph57Prep/Scripts/verify_reserved_input.py`，证据在同工程 `Saved/Review/reserved-input-*`。第四批三枪／附着弹适配仍未实施，本轮不宣称该批完成。详情见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
