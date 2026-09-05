# 当前进度

## Latest continuity repair (2026-09-05)

- Authorized arm+leg twitch repair installed. Stable anatomical bend frames, with two symmetric three-frame leg smoothing passes. Back arm peak99.2 to11.62deg/frame; leg45.1 to21.44. Root/torso/head and timing unchanged.
- External/target cold checks, actual PIE runtime and both attachment regressions passed (MixamoContinuity logs). Final knee flex<78deg; numerical continuity checks do not constitute visual acceptance.
- Preview opened: D:/Blender Projects/HumanoidHitReactions/Humanoid_Mixamo_Rifle_Continuity_Normal.gif. Checkpointbd91655; four final assets and harness uncommitted. Do not stage user maps/audio/VFX. Earlier leg-fix notes below describe the previous version; current knee planes follow pelvis-relative base anatomy.

## Active Feature

- FEAT-080，in_progress。用户接受Mixamo官方动作并授权持枪适配/接入，明确删除旧受击Rig。本轮实现、冷读与PIE/回归验证已完成，待用户实战观感反馈。

## 当前受击系统

- 最新腿部修正：取消为了追远脚位而额外蹲低骨盆，改为收缩超出腿长的水平脚目标，膝盖朝向来自源动作。Back骨盆下降32.3→13.5cm、最大屈膝125.9→78.7°；四条时长和Root轨道不变。预览Humanoid_Mixamo_Rifle_LegFix_Normal.gif已打开。步幅修正不是严格接触锁脚，自然度待用户反馈。
- 4条AS_Humanoid_BlastRifle_Front/Back/Left/Right位于Phantom/Animations/Reactions，绑定现役70骨；1.3667/1.6667/1.5667/1.6333秒、30fps，保留Mixamo原节奏，适配右手握枪/左手释放回握/腿长差异。外部源工程与预览位于D:/Blender Projects/HumanoidHitReactions，Unreal源成品生成在TMIIR/ReactionPrep/MixamoFinal。
- BP_Phantom→ExplosionHitReaction→BodyAnimations保留6个部位槽，当前共用4方向动画。命中骨骼/Actor局部入射方向仍决定分类与方向；只让爆炸弹附着的存活目标播放，范围伤害不变。不是24种新独立动作。
- 旧Rig运行链、参数、原生RigUnit、Frame结构、专用测试和安装脚本均已删除。共享后处理是Humanoid/_Shared/Animations/Logic/ABP_Humanoid_HitReaction，只有SequenceEvaluator与全身/上半身混合，无Rig或模式选择。旧ControlRig目录已删除。
- ApplyAnimationRootMotion默认true：非下落时消费动画Root水平位移并扫掠胶囊，反应期间暂存并暂停CharacterMovement模式，结束或关闭恢复；死亡不恢复。force_root_lock避免骨骼重复位移；下落保留上半身且不消费根位移。关闭此参数可原地播放。碰墙限制位移，不进行步态重规划。
- BlendIn=.035、BlendOut=.18、PlayRate=1，按游戏时间；重复命中不重启动作。旧29条动画资产尚保留，但本组件已无引用。
- 武器仍挂hand_r_wepSocket、单位缩放；主AnimBP不换。Phantom StationaryHitTest仍开，正式AI测试需恢复先前测试配置。
- 死亡布娃娃、所有子弹致命击飞、尸体寿命、附着弹/血痕清理保持原行为。子弹时间仍是爆炸击杀或爆炸弹直接命中GeometryCollection；波及Chaos不触发。声音、VFX、震屏未改。
- 用户EnemyExplosionEffect=None、电击弹Damage=30等配置保持。

## 验证

- 本轮MixamoLegFixCold（外部）、MixamoLegFixTargetCold及MixamoLegFixRuntime通过，四条Root/时长与上版逐帧相同；定向AttachedLimbReaction/StickyBodySurfaces回归均成功（MixamoLegFixAttachment.log，2/2）。仅修改动画成品和外部脚本，无C++/蓝图改动。
- 最终MixamoCompletedBuild的Development Editor Win64编译通过；外部MixamoHeadingCold和目标MixamoTargetCold逐帧70骨校验通过，重编译配置持久化、依赖与旧Rig目录清理通过。
- MixamoRuntime：24部位方向×静止/移动选择和姿态、枪挂点、重复命中、结束恢复通过；Root位移无墙144.61cm/碰墙33.49cm/关闭0cm，结束与关闭恢复移动模式通过。Flying用于隔离地板，不等同真实Walking测试。
- MixamoRegression的AttachedLimbReaction、EnemyDeathRagdoll、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup成功；另两项因启动参数-nosound导致声音断言失败，恢复音频后MixamoAudioRegression的ExplosionRadialDamage、StickyExplosionAndBlood均成功。相关7项最终全部通过，声音链路未修改。

## 会话交接

- 最新检查点5c73314保存此前Mixamo接入及Rig删除。本轮四条动画腿部修正未最终提交/push；不要stage用户地图/ExternalActor/声音/VFX/电击弹资产。
- 安装/验证：Scripts/VFX/import_mixamo_reactions.py、install_mixamo_reactions.py、validate_mixamo_reaction_assets.py、validate_mixamo_reaction_runtime.py。旧Rig/旧动作安装验证脚本已清理，勿按历史记录运行已删除脚本。
- 外部adapt_mixamo_rifle.py、create_mixamo_external_assets.py、validate_mixamo_external.py、render_mixamo_rifle.py；数据mixamo_rifle_motion.json；预览Humanoid_Mixamo_Rifle_Normal.gif。本轮成品只有4方向，部位细分可在用户反馈后扩展。
- 持枪版Humanoid_Mixamo_Rifle_Normal.gif已按最终朝向重生成、检查阶段图并打开，架构文档已同步。测试编辑器均退出，无地图保存。历史细节见FEAT-080 archive。
