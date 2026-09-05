# 当前进度

## Active Feature

- FEAT-080，in_progress。用户否定原24条腿部动作，要求网上找动作参考并复刻“受冲击→迈步踉跄→站稳恢复”。本轮先交付4条完整踉跄样片供确认；当前游戏仍运行下文24条旧版，不能称其已获用户验收。

## 最新样片（未接入游戏）

- 最新为 Humanoid_Rhythm_Staggers.blend / Humanoid_Rhythm_Staggers_Normal.gif（外部同目录）。用户指出上一版即使加速仍节奏均匀，本版保持1.3秒，首个运动帧达到约87.5%躯干幅度，峰值后短暂保留失衡姿态，两次追步各约.133秒，第二次落地后承重停顿约.133秒，最后缓慢恢复。四方向支撑脚目标误差<.00005cm。已查看阶段图并生成正常速度预览；待用户评价节奏，未接入游戏。
- 参考MorStudios HitReact Pro官方展示 https://www.youtube.com/watch?v=Qoq9pzQ_tA4 的84–89秒Stumble_B，已实际查看分帧：上身先失衡，双腿交替迈步承重，站稳后抬身恢复。四方向是视觉重建及持枪适配，不是视频精确动捕提取。
- 外部D:/Blender Projects/HumanoidHitReactions/Humanoid_Referenced_Staggers.blend有4条AS_Humanoid_StaggerRef_*，2.2秒/30fps，两步主要接重心+一步小调整，步高约10cm，支撑脚锁点，固定膝盖弯曲平面；前后根位移76cm、左右62cm，最终留在新位置恢复。
- 预览Humanoid_Referenced_Staggers_Normal.gif和Humanoid_Referenced_Staggers_Slow.gif，参考索引References/Showcase_Index.jpg、Stumble_Timing.jpg及REFERENCE.md。新动作尚未导入TheManTest或替换运行时；后续需处理根位移/胶囊碰撞，不能直接套旧无RootMotion后处理。
- 自动审批拦截参考视频裁GIF并打开的命令，仅返回blocked by policy，已告知用户；未重试该操作。自己制作的角色动画预览正常生成。

## 当前行为和入口

- BP_Phantom → ExplosionHitReaction → Body Animations：6组Torso/Head/LeftArm/RightArm/LeftLeg/RightLeg，每组Front/Back/Left/Right，共24条AS_Humanoid_Blast_*。ClassifyHitBone按BoneMapping中臂/腿/颈骨骼祖先分类；具体动画在Phantom/Animations/Reactions，兼容当前70骨Skeleton。共享组件和ABP无Phantom资产依赖，其他骨架须在外部适配自己的动画。
- 命中时保存Actor局部入射方向和解析出的Mesh骨骼；敌人倒计时期间转身不改变选择。爆炸伤害后只对AttachedHitActor调用，Strength=1。被波及者正常扣血但无此动画，致死直接进入布娃娃。
- 24条动作1.4秒/30fps，快速冲击、头手滞后、小幅反摆、慢恢复。手臂可以甩开支撑手；腿部骨盆下沉18cm、受击脚抬起约27cm、支撑脚IK。BlendIn=.035、BlendOut=.18、PlayRate=1。游戏时间采样，随子弹时间放慢；动作期间后续伤害正常，但不重启动作。
- 静止全身；非腿移动时上半身，腿部受击在非下落移动中也混全身；IsFalling时仍上半身。没有暂停移动/RootMotion/存活胶囊击退，移动中的脚滑仍可能存在。原主AnimBP和握枪socket不换。
- Reaction Mode默认Animation；原5条RifleHit序列保留为配置缺失时回退，原共享ControlRig链保留，切ControlRig才启用。旧MaxAngleDegrees等参数仅影响Rig。
- Blender工程和脚本在D:/Blender Projects/HumanoidHitReactions，主工程Humanoid_Blast_Limb_Reactions.blend；Limb_Reactions_24.png和Humanoid_Blast_Limb_Preview.gif已生成并自动打开。外部TMIIR的ReactionPrep/LimbFinal完成创建/验证/FBX导出，目标项目只导入成品动画。
- BP_Phantom武器挂hand_r_wepSocket（hand_r_wep动画骨骼），WeaponMesh相对缩放1，Mesh体型.9。不要恢复静态hand_rSocket_Aim或按状态切Socket。
- Enemy|Death：CorpseLifetime默认5游戏秒；ProjectileHitImpulse默认5000 kg cm/s。ProjectileKillKnockbackSpeed=250cm/s、ProjectileKillUpwardSpeed=120cm/s仅直接致命枪击添加全身速度，两个设0关闭；打旧尸体不重复全身击飞。范围爆炸继续径向400cm/800速度冲量，墙体遮挡。
- 子弹时间：本次爆炸杀Enemy，或爆炸弹首次直接命中GeometryCollection，在Fuse结束触发；不要求实际破碎，范围波及破碎不触发。旧ExplosionOutcomeSubsystem保留无消费者。声音/VFX/震屏及用户时间参数不改。
- 死亡切布娃娃，挂弹/身体血痕随骨骼；寿命到期EndPlay清理待爆弹和身体血痕，地面血迹独立寿命。
- Phantom|Testing.Stationary Hit Test仍默认开启。取消后重新PIE恢复原逻辑，正式AI还需恢复先前测试树和零速度配置。
- 用户当前EnemyExplosionEffect=None、电击弹Damage=30；保持不改。测试通过实例参数覆盖来检查零伤害反馈，允许用户关闭Enemy VFX。

## 验证

- 最终Development Editor Win64构建成功。LimbReactionImport/Install成功；LimbReactionColdFinal逐帧检查24条×70骨，六组引用重编译后保留，共享ABP无Phantom依赖、动画目录无Redirector。ACLPlugin压缩设置为正常引擎插件依赖。
- LimbReactionRuntime：24 standing + 24 moving通过，实际部位骨骼位移、全强度混合、方向、Actor旋转、主AnimClass、枪挂点、恢复及重复不重启均通过。移动采用Flying+150cm/s隔离地板，确认非下落分支；真实Walking附着由既有MovingEnemyAttachmentCleanup覆盖。
- LimbReactionRegression六项Success：AttachedLimbReaction、EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。ExplosionRadialDamage初次因用户关闭Enemy VFX而旧断言失败；允许该设置后重新构建，LimbReactionDamageFinal单项Success，相关7项最终均通过。
- AttachedLimbReaction通过实际Mesh表面解析确认左腿附着；目标转90度后仍播放LeftLeg_Back，旁边Enemy扣20血但不播，致命爆炸走布娃娃。
- 前轮条件子弹时间/致命枪击、Relax挂枪、准星附着精度验证详见archive，不重复列历史日志。

## 会话交接

- 当前检查点5b79c06仅保存上轮参考样片的harness文档。最新外部脚本create_rhythm_staggers.py、render_rhythm_staggers.py，数据stagger_rhythm_motion.json；此前2.2秒参考版和1.3秒Blast版均保留对比。本轮无C++/UE资产修改；下一步以Rhythm版反馈为准，暂不扩展24条或迁入。
- 最新检查点f27cd3e保存上一轮24条动作/配置/源码/harness作为恢复点。当前仅外部四方向参考样片和harness更新，未改游戏C++/资产，不需要新编译。等用户看四方向样片后再继续扩展与接入；原24条仅是技术验证通过，视觉已被用户否定。

- 最新检查点8afe766保存前轮代码/harness和已知动画BP状态；本轮源码、24资产、BP配置、脚本与文档未最终提交/push。用户地图/ExternalActor/音效/血纹理/电击弹/Explosion Cue改动不得全量提交或覆盖。
- 本轮脚本Scripts/VFX/import_limb_reactions.py、install_limb_reactions.py、validate_limb_reaction_assets.py、validate_limb_reaction_runtime.py；新原生测试AttachedReactionTests.cpp。旧安装脚本仍保留旧五条配置，但不应拿它覆盖本轮部位配置。
- 没有在TheManTest做重定向，未导入源骨架/模型/IK资源。测试编辑器均退出，没有保存测试地图。实现已完成，下一步用户试力度/动作观感。
