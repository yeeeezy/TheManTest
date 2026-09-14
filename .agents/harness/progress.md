# 当前工作面板

- Active feature：FEAT-081 CoreMorph头领迁移，in_progress；FEAT-080暂停。
- 已迁入飞行、重组／风墙、八足／尾刺雷爆；Manta核心导弹GA已实现并验证。唯一Actor／ASC／Health，目标UE5.7.4，源UE58Blank5.8.2保持不变。第四批三枪／附着弹适配仍待实施。
- 用户最新要求清理所有额外测试地图，TestMap恢复基础场景；并删除依赖旧摆件的专项测试、保留其他回归测试。本轮清理已完成，取代旧“总验收后再清理”安排。

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

## 会话交接

WIP `f719242` 保存清理前完整Manta技能／检查设施；当前清理结果未提交／push。用户已确认删除旧摆件专项测试、保留其他回归。长期约定已写入AGENTS.md，禁止再创建独立测试地图或保存临时摆件。外部脚本 `cleanup_maps.py` 已执行，不要重跑；证据位于 `D:/Unreal Projects/CoreMorph57Prep/Saved/Review/map-cleanup-*`。下一步按用户新需求推进正式功能，第四批武器适配仍未实施。详情见 [FEAT-081 archive](archive/FEAT-081-core-morph-boss-migration.md)。
