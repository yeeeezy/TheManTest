# 当前工作面板

- Active feature：FEAT-082大厅角色展示。用户截图102535否决仅修手腕版，指定视觉参考Phantom。已在Blender用实际人物与三枪重做握持并接入，等待用户观感确认。
- FEAT-081／080暂停，详见对应archive。

## 当前实现

- 三枪各两条放松和一条举枪，现有九条按枪命名成品路径保持，Display Weapon Index为0维修枪／1爆破枪／2电击枪。RelaxedIdleIndex选择放松版本，数字1切换放松／举枪。
- Blender对照Phantom掌心／手指形态进行FK摆姿：修lowerarm_l、hand_l、五指前两节共12个旋转轨道；保留上臂与肘部位置，手腕离开机匣，掌心托枪。身体／右手、骨骼局部位移和缩放保留。无IK求解或运行时握持处理。
- Blender工程：D:/Blender Projects/LobbyWeaponGrip/LobbyWeaponGrip_Animated.blend，九个命名人物Action及对应枪Action；静态Phantom对照另存LobbyWeaponGrip.blend。脚本、导出资源、近景、制作数据和验证日志同目录。
- TMIIR负责从Blender制作参数生成／核验最终九条AnimSequence，AssetTools迁入覆盖现有路径。目标本轮没有C++／BP／武器或挂点改动，不迁入参考Phantom资源、Rig或制作文件。

## 验证与交接

- TMIIR辅助模块编译成功。源冷读逐关键帧核对九条动画与Blender旋转修正、全部局部位置及依赖通过；Blender实际骨骼与制作数据误差小于0.000003m。
- validate-grips实际LobbyMap PIE通过2816样本，九条持枪和两条Standing完整循环，标准SingleNode、模型材质缩放、修正后手部输出、肩肘位置及身体／右手保留、One按键链与无效枪索引均通过。测试前后地图及资产哈希不变。
- 已查看LobbyBlenderGrip-*实机两侧近景，并打开维修枪侧视预览；不以数字测试替代用户观感。全部后台制作／测试编辑器已退出。
- checkpoint4d581c3；九条成品动画及harness未最终提交／push。工作区另外有用户保存的LobbyMap.umap变更（10:27:14），保留，不归入本轮动画改动或擅自回退。
