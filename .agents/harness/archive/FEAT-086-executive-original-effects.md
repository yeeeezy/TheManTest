# FEAT-086 执行官原版红色效果

状态：done，关闭日期2026-09-16。

- 用户暂缓UI调整到明天，要求执行官更接近原版展示图的红色效果，并明确授权完成、保存、检查后关机。
- 对照A0102目录原版展示图与实际大厅：当前面罩偏灰、胸口缺少红光晕、全息较实。素材有Armor/Sniper Emission与Hologram Emission/Opacity，原全息网格已有。
- 保留UI、动画和模型比例。调整专属发光材质、全息透明度，补充跟随面罩和胸口的局部红光；真实大厅Relax和Ready都需验证。
- 操作前本地checkpoint392d853保存已完成的角色选择／举枪功能，未push。预览与证据在D:/Blender Projects/ExecutiveLobby/effects_*。
- AI生成的UI参考图可能改变人物与光照，不能用作资产一致性证据。本批以素材原版展示图和UE实际截图为准。

## 实现

- 原Armor/Sniper/Hologram发光贴图分别接可调EmissionStrength=20/8/5；全息Opacity额外乘0.45，保留原平面网格与纹理、透明无光照材质。
- 新增专属AExecutiveLobbyCharacter，BP_Executive_Lobby改为其子类；FaceGlow和ChestGlow跟随头／胸骨。面罩6lm/24cm衰减/10cm光源半径/Specular0.3；胸口1.2lm/22cm/6cm/Specular0.05。均无阴影、仅Channel1、无间接照明；隐藏角色同步关闭两灯，返回恢复。
- 首轮70lm面罩明显过亮，已淘汰；调整位置到面罩前方并降低强度，保留面罩细节。所有试验截图保留外部，Final_开头才是最终截图。
- effects-build-final.log Development Editor Win64 Succeeded；effects_author.json保存3材质+BP，并编译消费者；effects-refine.log将面罩最终收敛到6lm。原有UI／枪与动画／地图保持。

## 最终验收与关机交接

- effects-pie-final.log／effects_pie.json：13项实际Lobby PIE检查通过，包含Relax、Ready、往返切换、灯随骨骼位置误差<0.01cm、切维修工关闭灯／重选执行官恢复、原三枪回归；地图SHA256未变。
- effects_audit.json：32资产冷加载，三个材质发光倍率和透明度持久化正确，Native子类正确；无源工作依赖／重定向资产／Redirector。Final_Character_Executive_UI.png、Final_Weapon_Executive_UI.png已查看，是最终真实UE截图。
- 这是原版风格的实时效果重建；原版棚拍与大厅光照不同，不承诺像素一致。没有用AI图作验证，没有增加原素材未证实的旋转粒子。
- 用户明确要求完成后自行关机；完成保存／编译／验证／交接后调用Windows正常关机，不加/f强制丢弃未保存文档。项目资产及代码均已落盘，未做最终Git提交／push。检查点392d853可恢复操作前版本。
- UI留到明天：最新Weapon参考是黄色WEAPON标题与黄色短线；Character下一位／开始游戏设计尚未落地。Phantom腿细和浮空继续暂缓。
