# 当前工作面板

- Active feature：FEAT-082大厅角色展示。用户已接受Blender三枪握持，随后授权CHARACTER／WEAPON菜单与镜头调整。FEAT-081／080继续暂停。

## 当前实现

- /Game/UI/Lobby/WBP_LobbyPresentation：左侧纵向半透明深灰按钮、英文CHARACTER／WEAPON、细金色选中／悬停描边。UMG布局可编辑，显式引用Engine Roboto字体。BP_CharacterSelectGameMode创建此菜单；旧选角开局UI保持独立。
- CHARACTER为全身展示，WEAPON为枪与双手中近景，复用原Far／Near相机。Switcher以0.7秒smoothstep同步位置／旋转／焦距／对焦，不再累加弹簧推进；同目标点击忽略，反向切换从当前画面开始。鼠标横／纵5／3cm，近景乘0.5，过渡期间暂停鼠标视差。
- 两台相机统一36×20.25mm画幅，Far26mm、Near40mm，构图避开左侧菜单并为爆破枪枪口留空间。保留用户人物位置(0,0,24.121307)与场景其他对象。
- 按钮只切展示镜头；背景点击不切换。三枪已有独立Blender成品动画保持，数字1仍切放松／举枪；DisplayWeaponIndex=0维修枪／1爆破枪／2电击枪。

## 验证与交接

- Development Editor Win64编译通过。UMG、GM已编译保存；正式LobbyMap实际PIE按钮OnClicked绑定、双向过渡、同目标重复点击、途中反向、鼠标位移和退出包哈希验证通过。最终留边调整后验证完成：540样本、六组持枪／12次双向切换，json ok=true、log为LOBBY_MENU_VALIDATION_OK。全部后台编辑器已退出。
- 早期取景过紧和字体缺字已在实机截图发现并修正。最终图为Saved/Codex/Final-LobbyMenu-Character00002.png及Final-LobbyMenu-Weapon-{0..5}00002.png，已逐张查看，三枪枪口均留有空间。
- 写入前checkpoint cf805ce已保存前轮动画和用户地图改动；当前菜单、C++、地图与harness变更未最终提交／push。动画／正式武器资源本轮未修改。测试脚本／截图位于Saved/Codex，不新增永久测试关卡。
- 旧握持制作工程与完整2816样本验证见D:/Blender Projects/LobbyWeaponGrip及FEAT-082 archive。


## 最新交接：场景相机一致性修复

- 每帧使用Far／Near相机组件世界变换，补齐宽高比／轴约束、过扫描、后处理等同步，保持原0.7秒切换和小幅鼠标视差。相同视口、鼠标居中且切换结束时与场景相机取景一致。
- Development Editor Win64编译通过；lobby-camera-parity.json四组实际PIE GetCameraView对照通过，含运行中移动Actor／组件、修改镜头参数和返回远景。Rig／PlacedCamera两张实机对照截图已查看。地图未保存且哈希不变，后台编辑器退出。
- 当前checkpoint82e1b44；此次只改Switcher.cpp与harness，无资产变化，未最终提交／push。前文固定取景坐标为菜单初版记录，之后用户调整以当前场景相机为准，不按旧数值覆盖。

## 远端提交授权

用户已明确要求提交文件到远端。本次提交相机一致性修复与交接文档，并正常推送main上此前28个未发布提交及其LFS资产；fetch确认远端无新增提交，不重写历史。编译与四组PIE验证已通过，本轮发布不改产品代码或资产。
