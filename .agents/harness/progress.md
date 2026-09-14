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
