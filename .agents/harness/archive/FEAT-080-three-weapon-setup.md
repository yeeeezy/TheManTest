# FEAT-080 RepairGun、电击枪与爆炸枪统一动画和独立 VFX

## 2026-09-05 用户停止自制并指定Mixamo资产

- 后续用户登录并将Mixamo标签页置前，成功用Windows UIAutomation读到官网控件并下载，无需访问登录凭据。检查点a8a524c。下载5条官方X Bot源FBX：Hit Reaction（描述While Holding A Rifle）、Standing React Large From Front/Back/Left/Right；下载设置FBX Binary、With Skin、30fps、none。保存Downloads并复制至D:/Blender Projects/HumanoidHitReactions/Source/Mixamo。Back下载延迟导致重复一份(1)，未删除用户Downloads。
- Blender实际导入五条成功，每条65骨；时长依次2.3/1.3667/1.6667/1.5667/1.6333秒。Mixamo_Source_Review.blend及mixamo_source_report.json保存；90帧30fps正常速度原动作预览Mixamo_Original_Reactions_Normal.gif生成，阶段图已查看。四方向Large均有全身失衡/脚步，持枪版更克制；尚未完成目标骨架适配或修改，未替换游戏。原始帧时间保留，各动作按各自时长播放完后停留。
- 用户认为生成动作均不合适，先要求找现成资产，随后明确指定Mixamo寻找并修改。此前参考/Blast/Rhythm均未验收，停止继续制作这些样片。检查点968ca73保存此前文档。
- 检索到Hit Reaction（持枪受击描述）、Big Hit To Head和Stumble Backwards候选；候选名称来自外部索引，未完成官方预览，不宣称适合或已获得。只读访问Mixamo官方首页/公开JS确认产品接口；未认证GET /api/v1/products返回403，未尝试绕过认证。当前工具无浏览器控制能力，Downloads无匹配FBX。Adobe官方文档确认登录后选择角色/动画并下载的工作流。
- 后续需用户通过已登录官网提供源FBX，优先持枪Hit Reaction和重击候选；取得源文件后保留真实原动作节奏，外部适配骨架、握枪和首尾。没有下载第三方重新分发文件，没有修改游戏资产；重定向仅允许TMIIR/FPSShooter1。

## 2026-09-05 爆炸样片速度与节奏修订

- 检查点5b79c06保存上一轮harness，未纳入用户资产。用户授权更快更猛，先制作1.3秒Blast版，随后用户指出关键是节奏仍过于均匀。继续同一外部样片任务，保留全部旧版。
- 最新create_rhythm_staggers.py取消起始躯干缓入，首个运动帧约87.5%幅度，.0667秒峰值，头部延迟；峰值姿态短暂保留，再急促追步。两次主步分别.10–.2333秒、.30–.4333秒；落地压低身体并保留至.5667秒，随后小调整步与慢恢复，总时长仍1.3秒。脚摆动采用快速出脚/减速落脚，区别于旧版所有阶段相似的smoothstep节奏。
- 4条AS_Humanoid_StaggerRhythm_*保存至外部Humanoid_Rhythm_Staggers.blend和stagger_rhythm_motion.json。前后86cm/侧向72cm根位移，步高约14cm，支撑脚目标误差<.00005cm；阶段图Rhythm_Stagger_Phases.jpg已检查。Humanoid_Rhythm_Staggers_Normal.gif为30fps正常速度，循环含首尾停留共2秒。未导入游戏，未修改C++/蓝图/声音/时间系统；主观节奏待反馈，后续接入仍需处理根位移与碰撞。

## 2026-09-05 用户否定原腿部动作，改为网络参考踉跄样片

- 用户反馈24条动作腿部奇怪，需要受冲击后迈步踉跄再恢复，所有受击都应有腿部参与。确认先做四方向完整踉跄预览，再看观感决定部位扩展/游戏替换；随后明确要求先网上找动图或视频参考再复刻，不能继续凭空设计。
- 检查点f27cd3e保存上一轮24条动作及其配置/源码/文档，仅作为历史恢复点，不代表用户验收。用户地图/音效/VFX/电击弹参数不纳入。
- 已搜索并读取MorStudios官方Fab的HitReact Pro，下载公开YouTube Showcase（Qoq9pzQ_tA4）供逐帧参考，查看总览和84–89秒Stumble_B动作：上身先失衡、交替迈步接住身体、落脚后恢复。参考资料和时间索引放D:/Blender Projects/HumanoidHitReactions/References。尝试裁出参考GIF并打开的命令被自动审批拦截，仅返回blocked by policy，未重试该操作，已向用户说明。
- 外部create_reference_staggers.py根据视觉参考制作4条AS_Humanoid_StaggerRef_{Front,Back,Left,Right}，2.2秒/30fps，交替两步接重心+一步小调整。固定解剖膝盖朝向、每只脚落地后锁点；步高约10cm，前后位移76cm/左右62cm，站稳于新位置不滑回起点。支撑脚误差<.00005cm。这里是视觉重建/持枪适配，非从视频恢复精确动捕数据；侧向和正面按所见背部受击步态调整。
- Humanoid_Referenced_Staggers.blend和stagger_reference_motion.json已生成，四方向预览渲染中。新样片未导入游戏，当前游戏仍是用户否定的24条版本；后续若接入，需要处理真实移动/碰撞和根位移，不能直接按旧无RootMotion后处理播放。
- 已生成并打开Humanoid_Referenced_Staggers_Normal.gif与Slow.gif（正常/半速，含首尾停留），Referenced_Stagger_Phases.jpg阶段对照已查看。预览最初带有导入枪械的UCX碰撞壳，已在渲染中隐藏并重渲染；不影响骨骼数据或游戏资产。四条样片均有交替脚步与位移、落脚锁点、最终站稳，待用户确认自然度后再扩展部位。没有本轮C++/UE资产修改，未最终提交/push。

## 2026-09-05 强爆炸部位方向动作（实现与自验完成）

- 用户授权：只让爆炸弹附着的敌人播放存活受击动画，按四肢/躯干/头部和命中方向选择，幅度明显增强；旧Rig链保留关闭。写前检查点8afe766保存前轮源码/harness及已知动画BP状态，未纳入地图/音效/电击弹设置。
- BodyAnimations为6部位×前后左右配置，骨骼通过BoneMapping的臂/腿/颈祖先分类。爆炸弹在首次命中、伤害转向之前记录Actor局部入射方向，爆炸时只给AttachedHitActor传原骨骼及方向、Strength=1；范围伤害/物理/子弹时间不改。腿部动画在地面移动中也走全身混合，非腿移动仍上半身，空中保留原上半身规则。
- 外部D:/Blender Projects/HumanoidHitReactions/create_limb_reactions.py制作24条AS_Humanoid_Blast_{Torso,Head,LeftArm,RightArm,LeftLeg,RightLeg}_{Front,Back,Left,Right}，1.4秒/30fps，快速冲击/滞后头手/小幅反摆/恢复。腿部18cm骨盆下降、受击脚约27cm抬起，支撑脚IK；手臂可松开支撑手。原5条动作作为兼容回退保留。
- Blender工程Humanoid_Blast_Limb_Reactions.blend及24动作对照图Limb_Reactions_24.png已生成、查看并自动打开。TMIIR的ReactionPrep/LimbFinal创建最终动画并导出FBX；LimbReactionCold逐帧70骨冷读全部通过。目标项目只导入最终动画，没有源骨架/模型/重定向工作资源。第一次C++构建已通过，资产安装和PIE验证正在进行。
- LimbReactionImport/Install成功，BP_Phantom六组24条引用保存；共享ABP仅重编译，原图和Rig连线不改。目标LimbReactionColdFinal逐帧24条动作、70骨、六组配置冷读/再编译、依赖与无Redirector通过。初次校验未允许既有ACLPlugin压缩设置导致断言，补充该引擎插件允许路径后通过；不是外部源骨架残留。
- LimbReactionRuntime输出LIMB_RUNTIME_OK 24 standing + 24 moving：所有部位/方向/Actor旋转选择正确、Alpha>.95、旧Rig输出0、主AnimClass保留、枪挂点误差<.1cm、动作结束恢复、重复命中不重启。移动为隔离地图地板的Flying+150cm/s、非下落条件，确认腿部full-body而非腿upper-body；真实Walking附着由既有MovingEnemyAttachmentCleanup覆盖，不宣称脚步无滑动。动态预览Humanoid_Blast_Limb_Preview.gif已生成打开，1.4秒动作预览放慢播放。
- LimbReactionRegression中AttachedLimbReaction、EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood六项Success。新增附着测试确实解析到左腿，命中后转90度仍播LeftLeg_Back；旁边Enemy扣20血但无动画，致死直接布娃娃。ExplosionRadialDamage仅因用户EnemyExplosionEffect=None而旧测试强制非空失败，保留用户关闭设置，将该配置允许为空后重新构建成功，单项重验中。
- 最终LimbReactionDamageFinal.log的ExplosionRadialDamage Success，相关7项最终均通过。最终Development Editor Win64成功；所有测试编辑器已退出，未保存地图或改用户VFX/声音参数。本轮24条成品动画和配置已正式接入，结果未最终提交/push。完整FEAT-080仍in_progress，主观自然度/力度待用户实战反馈。

## 2026-09-05 直接Chaos命中子弹时间与致命枪击击飞（实现与自验完成）

- 用户确认动手：子弹时间条件改为本次爆炸击杀Enemy，或爆炸弹直接命中GeometryCollection；统一在Fuse结束时请求。命中时记录真实Hit组件分类，不再监听范围内Chaos Break。直接命中无需实际破碎，波及破碎仍正常但不触发慢动作。旧ExplosionOutcomeSubsystem保留但爆炸弹不再调用。
- Enemy|Death新增ProjectileKillKnockbackSpeed=250cm/s、ProjectileKillUpwardSpeed=120cm/s。ABulletBase仅在伤害前存活、伤害后死亡时给模拟身体添加全身速度，保留原部位点冲量；所有弹体直接致命一击统一，已有尸体中枪不重复全身击飞，范围爆炸仍走原径向冲量。两个速度均可设0关闭。
- 写前检查点cdf510c仅保存已知前轮源码/脚本/harness；用户地图、ExternalActor、音效/血纹理/电击弹参数与二进制资产未纳入。本轮不写资产。扩大ExplosionOutcomeBulletTime至12场景，EnemyDeathRagdoll增加击飞/关闭/尸体不重复上抛检查，正在编译与PIE验证。
- 首次Development Editor Win64成功；DirectChaosLethalLaunch.log中EnemyDeathRagdoll、ExplosionChaosGround、ExplosionOutcomeBulletTime（12场景）、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup均Success。StickyExplosionAndBlood因用户Electric弹Damage已改30而旧测试假定0出现3条连带失败；仅将零伤害场景的测试实例显式设Damage=0，保持生产蓝图30不改，重新编译/单项验证。
- 最终Development Editor Win64再次成功；DirectChaosStickyFinal.log的StickyExplosionAndBlood Success，相关6项最终均通过（首轮5项+修正测试后单项）。三类子弹致命击杀骨盆前向速度303~454cm/s、向上81~118cm/s；旧尸体再中枪向上-16~2cm/s，无重复全身上抛。两个参数设0回原点冲量，寿命/骨骼附着清理通过。12项条件覆盖直接命中抗破碎/碎块/关闭ChaosRadius仍慢动作、开关关闭不触发、范围真实破碎不触发和击杀/墙后/非致命。测试编辑器退出，未写资产/地图，未最终提交/push；主观力度待用户试手感。

## 2026-09-05 动画替换默认受击、保留Rig分支（实现与自验完成）

- 用户直接授权启用新动作并保留旧Rig链路但默认不启用。选择性检查点4d0e0a1保存五条样片与前轮文档，不含地图/ExternalActor/用户Explosion Cue。
- EnemyHitReactionComponent新增ReactionMode=Animation/ControlRig（默认Animation）、五条具体Sequence配置、HeavyFrontMinStrength=.9、BlendIn=.06/BlendOut=.18/PlayRate=1。按Actor局部爆炸来源选四方向，近距离强正面可选HeavyTwist；播放期间新的反应不重启已有动作。使用游戏时间采样，不新建蒙太奇，不改变主AnimInstance/AI/伤害/音效。
- 共享后处理ABP保留原ControlRig节点与参数连线，新增Input缓存、动态SequenceEvaluator、静止全身混合/移动上半身spine_01混合及模式选择。Animation分支绕过Rig，旧Rig仍可切回；Shared ABP/Rig不增加Phantom动画引用，具体引用只配BP_Phantom组件。死亡仍由既有禁用PostProcess及布娃娃流程接管。
- 首轮C++编译修复TObjectPtr的auto*推导和BlendList私有数组访问后通过，ABP图编译保存成功。首次实际PIE发现组件动画引用冷读为空；排查为原ExplosionHitReaction没有显式UPROPERTY成员持有，直接编辑CDO的子对象不能可靠持久化。正在补充同名原生组件成员引用并重新验证保存，尚未宣称接入完成。
- 最终补充AHumanoidEnemy.ExplosionHitReaction的UPROPERTY成员持有后，AnimationReactionInstall4保存成功，AnimationReactionDefaults4冷读五条引用全部存在；AnimationReactionCold再次编译BP/共享ABP后配置仍保留，共享ABP/Rig没有Phantom资产依赖且无Redirector。没有创建第二个组件，原组件名称/身份保留。最终Development Editor Win64构建成功，无新增C++警告。
- AnimationReactionRuntime3.log输出ANIMATION_REACTION_RUNTIME_OK：前/后/左/右/重击及Actor转90度后的局部左侧选择正确，实际头位移18.41/40.52/28.38/9.19/33.42cm；Rig输出为0、主AnimInstance类不变、枪械挂点误差0。Enabled关闭、动作结束还原、重复爆炸不重启、切回ControlRig实际骨骼弯曲均通过。独立相同行走动画输入/等速Flying物理位移对照中，动画分支切上半身，骨盆/大小腿/双脚与未受击对照误差<.5cm；Flying仅用于隔离地板依赖，实际Walking附着由下一回归验证。
- AnimationReactionRegression.log最终7/7 Success、exit0：EnemyDeathRagdoll、EnemyExplosionControlRig（测试显式切旧模式）、ExplosionOutcomeBulletTime、ExplosionRadialDamage（真实爆炸触发动画且墙后无反应，Rig保持0）、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup（非Phantom原生Humanoid配置同骨架动画并实际Walking）、StickyExplosionAndBlood。未改死亡/伤害/子弹时间/音效/特效参数，未写地图。
- Scripts/VFX/install_animation_hit_reaction.py安装并强制保存具体BP组件配置；validate_animation_reaction_defaults.py冷读与再编译校验；validate_animation_reaction_runtime.py独立PIE验证。所有验证编辑器已退出。结果未最终提交/push，用户可在BP_Phantom的ExplosionHitReaction组件→Reaction Mode切换Animation/ControlRig；默认Animation。完整FEAT-080仍in_progress，待主观实战反馈。

## 2026-09-05 现有骨架受击动作样片

- 用户恢复受击动画任务，授权使用TMIIR候选参考或调整，适配现有骨骼。选择性检查点195a15c保存前置握枪修复；用户地图、ExternalActor及Explosion Cue未纳入。
- 审计RifleAnimsetPro四条Hit、Wraith四方向HitReact/Knockback及Belica候选。RifleAnimsetPro为68骨；现役Rifle_01为70骨，多hand_r_wep/hand_l_wep，手腕/脚参考姿势不同，不能只换Skeleton。Wraith四方向是带参考动画的Local Additive，提取原始绝对轨道后在外部处理差值。
- 最终样片为Wraith Front/Left/Right/Back四方向和Rifle_Hit_C_1的HeavyTwist变体。外部按现役Relax基准重建受力变化；保留手指及武器骨骼局部姿势，双臂握枪约束、双腿解析IK和脚掌高度限制，首尾精确返回同一Relax姿势。四方向旋转变化倍率1.35，重击1.0。此处是动画制作，不是运行时Rig替换；大幅重击只供对比。
- Blender工程/脚本/预览/最终FBX全部位于D:/Blender Projects/HumanoidHitReactions。外部create_external_assets.py只允许TMIIR运行，成品Sequence位于/Game/ReactionPrep/Final。未在TheManTest进行重定向、轨道生成或导入源骨架/模型/IKRig。
- 已导入5条AS_Humanoid_RifleHit_Front/Left/HeavyTwist/Right/Back至Phantom/Animations/Reactions；这是绑定Phantom当前Rifle_01骨架的具体Sequence样片，不提升为无骨架共享资源，也没有把公共Rig重新绑定Phantom。普通人形若用不同Skeleton仍需外部适配。没有修改ABP、受击触发、声音/特效/布娃娃或用户挂点设置。
- 时长分别1.0/.8667/1.6333/.8667/1.0秒，30fps、70轨、非Additive、非RootMotion。ReactionCreateFinal.log生成导出5条；ReactionColdFinal.log和目标ReactionImportCold.log逐帧核对全部轨道位置误差0、旋转点积最低0.99999939。ReactionImportFinal.log导入0错误0警告；现有Skeleton骨骼列表未改变。
- 已渲染并查看5动作多个阶段，Humanoid_RifleHit_Preview.gif为40帧同步相位对比，已自动打开。预览统一相位播放便于比较，不表示5条时长相同。首张枪错误朝向来自预览遗漏hand_r_wepSocket自身90度旋转，已修预览；UE挂点未改。初版Rifle动作握距不可达已修，最终左手数值误差小于0.00003cm。静态预览Reactions_Selected.png；主工程Humanoid_RifleHit_Reactions.blend有5个命名Action。
- 外部初次NullRHI导出Mesh崩溃改为实际编辑器只读导出；36fps不兼容默认30fps改为30；移除未暴露notify_populated后生成成功。add_bone_track只有弃用警告。独立PIE第一次NullRHI生成场景Actor触发引擎异常，改用D3D继续验证，尚未将该失败标为通过。
- 最终ReactionPreviewPIEFinal.log输出REACTION_PREVIEW_PIE_OK 5：实际BP_Phantom播放五条动画，头部位移32.07/26.84/32.85/7.90/38.44cm，枪与hand_r_wepSocket位置误差全程0。ReactionAssetsFinal.log输出REACTION_ASSETS_OK 5，首尾70骨与现役Relax基准匹配、5资产只有现有Skeleton依赖、无Redirector或外部项目依赖。没有C++/BP修改，无需新构建；验证未保存地图，测试编辑器已退出。右侧动作弱于其他方向，保留源动作差异供用户比较，不宣称所有方向力度一致或最终观感已验收。
- 本轮交付为5条独立可播放的成品样片与外部Blender工程/动态图；尚未把游戏中的爆炸触发改成Montage，也未替换共享Control Rig。需先由用户看动作后决定运行时组合，不能宣称游戏内受击反馈已经切到新动画。结果未最终提交/push。

## 2026-09-05 Phantom Relax持枪挂点修复（实现与自验完成）

- 用户截图112822显示Relax左手偏离护木。初次仅看Aim/Relax两Socket差异误判需状态切换；用户要求核对源工程后纠正：TMIIR/Rifle_01/Overview的5把示范枪全部挂hand_r_wepSocket，单位缩放/零局部位移旋转，包括Aim_To_Relaxed。该Socket父骨骼hand_r_wep本身带动画，Aim/Relax手枪相对运动已由动画驱动，不需要自行切两个静态手部Socket。
- 原Phantom CDO保存WeaponAttachSocket=hand_rSocket_Aim、WeaponMesh.Scale=.9，失去武器骨骼动画；武器无grip_l，左手IK无自动补偿。用户明确请求直接修复。选择性检查点a1d338f保存上轮布娃娃工作，不含用户地图/Explosion Cue。
- 仅BP_Phantom恢复WeaponAttachSocket=hand_r_wepSocket、WeaponMesh.RelativeScale3D=(1,1,1)。Scripts/VFX/fix_phantom_weapon_mount.py显式安装，-MountValidateOnly冷只读。PhantomMountInstall.log编译/保存通过，PhantomMountCold.log独立冷读挂点/父骨骼/单位缩放通过。无C++/动画轨道/骨架修改或重定向。实际PIE Relax/Aim/返回Relax截图验证进行中。
- 最终PhantomMountVisualFinal.log输出MOUNT_PIE_OK：实际BP_Phantom在PIE中Patrol→Aim→Patrol，Actor与AnimInstance状态一致，三次枪械位置到hand_r_wepSocket误差均0；相对缩放1，角色Mesh自身世界缩放.9，因此枪世界缩放同样.9，未改变角色体型。已查看TMT_PhantomMount_Relax/Aim截图：Relax枪呈斜横持握、左手回到护木，Aim可正常抬枪。临时测试Actor/相机/灯光清理，地图未保存，测试编辑器正常退出。
- 预览最初ExecutePythonScript自动退出、Python缺少GameplayStatics生成API、Transient Actor不进入PIE均已避开，最终使用未保存的临时场景Actor进行正常PIE；首张过曝截图已重拍。结果仅BP_Phantom两项配置，未最终提交/push。受击动画准备暂缓，等用户恢复该任务。

## 2026-09-05 死亡布娃娃与统一弹体冲量（实现与自验完成）

- 用户确认动手：死亡切布娃娃，所有子弹命中位置施加冲量，爆炸推动刚死/已有尸体，可配尸体寿命。写前选择性检查点b2bf304；地图/ExternalActor/用户Explosion Cue设置未纳入。
- EnemyBase死亡停止技能/AI/移动/角色Tick、清自身计时器及波次订阅，关闭胶囊；有PhysicsAsset时Mesh关闭PostProcess、暂停动画，启用全身物理。保留Pawn对象类型用于既有子弹查询。CorpseLifetime默认5游戏秒（最小.1），ProjectileHitImpulse默认5000 kg cm/s，Enemy|Death可配；重复中枪不延期。人形AIState置Dead，Phantom取消隐身。
- ABulletBase在伤害/转向前定位Mesh PhysicsAsset命中骨骼/局部点，伤害后对模拟身体AddImpulseAtLocation，覆盖致命一枪和后续尸体射击（含0伤害）。无模拟身体不施加，尸体不再扣血。Explosion普通物理移除Enemy类型排除，保留GeometryCollection独立路径；先范围伤害启动布娃娃再径向冲量，不因推动已有尸体触发子弹时间。
- 附着弹与身体血痕跟随尸体，到Owner真正EndPlay时清理；保留弹体原Fuse，可在尸体上继续爆炸。正在验证三种正式弹体致死/实际飞行打尸体、爆炸致死击飞、尸体寿命及移动/血痕清理/旧子弹时间回归。
- 首次构建因局部Mesh遮蔽ACharacter成员触发C4458，已改BodyMesh；Development Editor Win64随后构建成功。第一轮六项中五项通过，致命枪击骨盆方向断言失败；补充全身质量加权动量检查以区分关节回摆和总冲量方向，尚待最终验证。
- 后续定位：不是单纯骨盆回摆，EnemyRagdollMomentum.log显示致命枪击总X动量约-3300。关闭Humanoid手持WeaponMesh在死亡后的碰撞后，EnemyRagdollWeaponCollision.log专项Success，三枪总X动量约4780~4815（设定冲量5000），实际飞行0伤害弹再击尸体的骨盆速度约103~116cm/s，爆炸致死骨盆X速度约487cm/s。修复为正式死亡流程的一部分，未改PhysicsAsset或武器存活碰撞。
- 尸体正伤害弹跳过扣血后显式发一次Enemy Hit，保留肉体声/血迹，GCN_EnemyHit对死人不启动新痛呼；没有新增或改音频资产/音量。最终Development Editor Win64构建成功，无新增C++警告。EnemyRagdollFinal.log全套回归待收尾。
- 最终EnemyRagdollFinal.log六项6/6 Success：EnemyDeathRagdoll、EnemyExplosionControlRig、ExplosionOutcomeBulletTime、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。三种正式弹体致死后立即启物理，真实飞行弹打尸体仍施力；爆炸刚杀死的目标受径向冲量。分别2秒/.8秒尸体寿命测试通过，死亡到期前骨骼附着与身体血痕保留，最终Actor/挂弹/身体Decal清理。条件子弹时间九场景与存活Rig/普通物理/原血迹声音回归通过。测试编辑器正常退出，未写资产或地图，结果未最终提交/push。


## 2026-09-05 共享人形全身受击、死亡清理与物理爆炸（实现与自验完成）

- 用户批准合并实施：共享人形Rig归属/映射，更大更自然的全身含腿受击；真实移动/转身附着验证；敌人死亡清除悬空子弹/身体血痕；普通SimulatePhysics物体受爆炸冲量且不因此触发子弹时间。检查点8406b19保存上轮仰射及用户现有ABP状态，地图/ExternalActor/其他参数不纳入。
- 原生新增HumanoidReactionFrame和可配骨骼映射，38度/.85秒、头肩延迟跟随、7cm屈膝缓冲与双腿解析解保持输入动画脚位；保留胶囊不动。Humanoid基类自动设置共享后处理软类。Rig/ABP已通过UE移动到Humanoid/_Shared/Animations/ControlRig，清Rig预览Mesh、ABP转骨架无关模板，原Phantom Mesh后处理槽清空，改由基类接入。安装保存成功后审计None依赖返回导致脚本异常，已修仅日志遍历，不重复移动。
- 爆炸弹监听附着Enemy.OnEndPlay，非自身正在爆炸时销毁；自身爆炸击杀不打断Detonate。身体血痕改为Character拥有的Decal组件，死亡自动清理，地面血迹保持寿命。普通物理组件排除Enemy/Chaos、去重/Visibility墙体筛选后施加线性衰减速度冲量，独立PhysicsImpulseRadius400/Strength800，不请求子弹时间。
- Development Editor Win64当前构建通过。共享Rig实际PIE、腿部/第二人形使用方、移动附着/死亡清理与物理飞散验证尚在进行，未宣称完成。
- 最终Development Editor Win64成功，无新增C++编译警告。SharedReactionFinal.log五项5/5 Success：EnemyExplosionControlRig、ExplosionOutcomeBulletTime（九种条件结果）、ExplosionSimulatedPhysics、MovingEnemyAttachmentCleanup、StickyExplosionAndBlood。共享Rig四方向头部位移21.27~21.64cm、髋部下降4.8cm、膝部位移10cm，脚位/胶囊保持，头肩包络与躯干错开，恢复和部位受击通过。已目视检查TMT_EnemyRig_Directions.png，四方向全身受力清晰。
- 移动测试使用非Phantom的原生AHumanoidEnemy，复用测试模型/locomotion但靠基类自动接共享后处理；CharacterMovement实际行走约425cm并转45度、触发全身受击，子弹/血痕保持骨骼局部附着。两轮分别主动死亡与另一颗爆炸弹致死，挂载弹体与身体Decal均销毁。首次测试没有Controller时默认MOVE_None导致未移动，已在测试显式启用无Controller物理及MOVE_Walking，未用Actor位移冒充行走。
- 普通物理近100cm/远250cm测试速度约599/300cm/s，墙后/范围外保持静止，不触发子弹时间。Chaos独立回归证明未重复破坏或改条件触发。身体Decal淡出不会销毁Owner，环境血迹按原寿命保留。
- SharedReactionColdVerified.log输出SHARED_REACTION_COLD_OK：ABP只依赖共享Rig，Rig无/Game依赖；无Phantom预览Mesh、SourceHierarchyImport、SourceCurveImport或TargetSkeleton引用，旧路径Registry/磁盘为空，无Redirector。首次冷验发现Rig保留两个骨架导入源引用，已清除并独立冷验。Phantom实际默认回读38度/.85秒恢复/.045秒跟随/7cm腿压缩；骨骼映射可配置，不同层级仍需Rig兼容适配。原Phantom模型PostProcess槽清空，防止重复叠加。
- Scripts/VFX/install_shared_humanoid_reaction.py及validate_shared_humanoid_reaction.py为共享入口；Audio配置脚本不再生成旧Phantom Rig。当前Phantom固定靶开关、用户Explosion Cue效果/声音/震屏设置和地图/ExternalActor均未改。结果未最终提交/push，整体FEAT-080留用户观感验收。

## 2026-09-05 仰射仍偏下：实际敌人瞄准漏检（复现与修正完成）

- 用户反馈仰射明显偏下，继续同一修复；写前检查点f7f7287保存上轮骨骼附着。冷读爆炸弹半径15cm、AttachmentOffset4cm、Mesh本地零偏移，模型范围X±10.9cm/YZ±5.16cm。
- 新增真实PrimaryFire→Phantom的StickyUpwardAim，1/2米各0/30/60度仰角，独立相机射线求身体表面。StickyUpBefore.log复现5/6位置检查失败，画面竖直方向偏下22~27cm；弹道方向几乎CameraForward，说明旧Visibility/complex瞄准查询没有认出敌人而选了100m远点。1米场景同时出现胶囊提前挡枪立即附着。上轮六方向测试只有水平直接发射，没有覆盖准星到敌人的完整链路，不能支持此前完整修好判断。
- GA_Shoot改以子弹ObjectType/CollisionResponse进行simple瞄准查询，命中Character后沿准星射线细化Mesh PhysicsAsset目标。枪口遮挡中的Character只在实际Mesh穿过镜头→枪口线段时立即命中，排除胶囊假阻挡并继续查询后方墙体。没有调小用户子弹球半径或改VFX/声音/伤害。构建成功，仰射与四项回归进行中。
- 最终Development Editor Win64构建成功，无新增C++警告；StickyUpAfter.log 5/5 Success：StickyUpwardAim、StickyBodySurfaces、ProjectileCrosshairAim、StickyExplosionAndBlood、ThreeWeaponPIESwitch。六个实际PrimaryFire场景修正后竖直偏差分别-1.106/+2.850/-3.760/-1.106/+2.850/+3.542cm，对比此前-22~-27cm明显下降；1米场景均不再因胶囊立即触发附着。保留AttachmentOffset4cm；1米60度时横向差10.608cm，实际枪口→目标线仍会先碰邻近身体碰撞部位，未绕过真实身体遮挡，不宣称像素级准星重合。其他五场横向差<0.06cm。仍用PhysicsAsset表面，未改成蒙皮三角形判定。没有资产写入，结果未最终提交/push。

## 2026-09-05 爆炸弹背部附着定位（实现与自验完成）

- 用户确认动手。检查点0d9b19f保存上轮准星汇聚；不纳入用户地图/ExternalActor/Explosion Cue设置。原附着从胶囊接触点沿actor forward追踪固定40/120cm，失败静默回落胶囊。
- 当前实现：伤害/受击转身前，沿Sweep中心与缓存真实入射方向按Mesh Bounds长度检查骨骼PhysicsAsset表面；检查有效骨骼、非初始穿透、入射侧法线。射线掠空时只接受附近入射侧最近PhysicsAsset表面；失败隐藏可见弹体，保留原伤害/倒计时。骨骼局部点/法线在Super调用前保存，调用后转换回当前骨骼世界位置，避免即时转身导致落点沿用旧世界坐标。
- Development Editor Win64编译通过；首次FVector_NetQuantize三元类型错误已显式转FVector修复。六方向实际飞行/骨骼附着/受击骨骼随动与准星/Sticky回归进行中。碰撞表面仍为PhysicsAsset近似，不宣称逐三角形蒙皮精度。
- 最终Development Editor Win64成功、无新增编译警告。StickySurfaces.log中ProjectileCrosshairAim与StickyExplosionAndBlood Success，六方向附着位置/骨骼/可见性全部通过；随动初次因离屏测试未刷新骨骼失败。仅测试实例启用AlwaysTickPoseAndRefreshBones后，StickySurfacesFinal.log的StickyBodySurfaces Success：真实飞行命中胶囊后，前/后/四斜向均附入射侧Mesh有效骨骼，位置距独立表面射线预期<0.5cm，受击动画中弹体确实移动且保持骨骼局部位置。没有修改角色资产、碰撞配置或用户VFX/震屏参数。结果未最终提交/push。

## 2026-09-05 实体弹近距离准星汇聚（实现与自验完成）

- 用户确认按排查方案动手。原GA_Shoot从枪口以CameraForward平行发射，近距准星/枪口视差明显。检查点e287ecf选择性保存上轮Phantom固定靶；用户地图/ExternalActor/Explosion Cue参数不纳入。
- GA_Shoot改为读取PlayerController实际视点，Projectile先Visibility射线确定中心目标，再从枪口汇聚；无目标使用HitscanRange远点。沿镜头→枪口按弹体球半径/碰撞通道/Response做Sweep，阻挡时在近侧生成并走原ProcessHit，避免贴墙生成到墙后；退化/身后目标不倒射。正常飞行仍由ProjectileMovement碰撞，不远程直接结算目标。忽略玩家及挂载装备，未改Enemy胶囊/骨骼命中规则。
- Development Editor Win64构建成功，无新增C++警告。ProjectileCrosshairAim通过真实PrimaryFire路径准备1.5/3/10米汇聚与20cm墙面挡枪验证；三枪切换和StickyExplosionAndBlood回归进行中。
- 最终ProjectileAim.log三项3/3 Success：ProjectileCrosshairAim验证三个距离目标面轨迹误差<0.1cm、实际命中附着、消耗一发弹药、20cm贴墙枪口越界仍在墙前附着且不倒射；StickyExplosionAndBlood、ThreeWeaponPIESwitch回归通过。未修改资产/地图或用户震屏和Enemy VFX开关，结果未最终Git提交/push。

## 2026-09-05 Phantom固定靶临时开关

- 用户要求敌人始终不移动、不自动朝向玩家，以测试不同方向的受击动作。根因为EnemyBase.ReactToProjectileHit直接旋转，加上Humanoid设置Aim/AI Focus，零速度不足以阻止转向。
- 选择性检查点7b77124保存上轮EnemyScale修复；用户地图/ExternalActor与本轮发现的Explosion Cue调参不纳入、不修改。Phantom新增默认开启的Phantom|Testing → Stationary Hit Test，BeginPlay移除该实例AI控制器、清巡逻计时器、关闭角色Tick和Movement；ProjectileHit跳过转向/Aim。Mesh及ExplosionHitReaction组件继续工作，未改伤害/死亡/受击表现。取消开关并重新PIE恢复原行为，旧静止树/零速度配置需另恢复才回正式巡逻。
- 首次编译因用户编辑器锁定DLL失败；MCP确认DIRTY_CONTENT/DIRTY_MAPS均为空后正常关闭，当前地图TestMap，完成构建验证后重新打开。
- Development Editor Win64构建成功，无新增C++警告（既有StructUtils插件弃用提示保留）。StationaryPhantom.log中EnemyExplosionControlRig Success，新增断言确认四方向ProjectileHit不改Actor朝向、无AI Controller、Movement=None，实际骨骼仍沿四方向弯曲并恢复，胶囊/支撑脚不动。StickyExplosionAndBlood首次因用户已关闭CameraShake而触发旧的强制震屏断言；改为按当前Cue开关检查，StationaryPhantomRegression.log最终Success，未改用户震屏配置。测试完成重新打开TestMap，结果未最终提交/push。

## 2026-09-05 EnemyEffectScale修复（实现与自验完成）

- 用户确认动手并要求继续。写前检查点4838cfa选择性保存条件子弹时间与Air007迁入；地图/ExternalActor不纳入。
- 冷审计Air007共22发射器，局部/世界空间混合，现有组件Scale没有统一影响SpriteSize、RibbonWidth和世界空间速度。对本枪System内22条ParticleUpdate栈接引擎ApplyOwnerScaleToAttributes，在Solve前执行，Owner Scale绑定Engine.Owner.Scale；Sprite/Ribbon/CameraOffset统一，世界空间另缩放初速度/力/mesh，本地运动保持既有组件变换以避免双重缩放，Drag不缩放。
- 新增Editor-only NiagaraEditor依赖以及显式-InstallEnemyScale安装入口；无标志时审计只读。首次模块默认全false，已开启目标静态开关；第二次安装重连已有override触发引擎断言（未保存），已改为保留已有链接，InstallEnemyScale3成功编译保存。Development Editor Win64成功；0.25/1/2三倍率真实PIE渲染和粒子尺寸验证进行中。
- 最终Development Editor Win64构建成功，无新增编译警告。EnemyScaleFinal.log四项4/4 Success：ExplosionScaleAudit、EnemyExplosionScaleVisual、ExplosionRadialDamage、StickyExplosionAndBlood。审计检查22个发射器各一个缩放模块、Owner Scale链接及空间对应静态开关；默认不写资产。
- 实际Enemy Cue在无GroundHit条件下生成Air007；固定18步×1/60秒CPU取样，倍率1/.25/2的火球尺寸分别913.798/228.566/1828.529，碎片平均半径263.855/63.168/763.051cm，亮区2939/252/8401像素。火球尺寸近似线性，碎片扩散随倍率变化，原随机运动/力/碰撞不保证轨迹严格等比。截图TMT_EnemyExplosionScale_0/1/2.png保存在Saved/Screenshots/WindowsEditor，已目视确认缩放；测试内临时确定性设置恢复且不保存。
- ValidateEnemyScaleCold.log输出AIR_COLD_OK 115；依赖闭包、无Redirector和供应商目录、Enemy/环境两个槽位与原子弹时间均通过。冷读确认用户当前EnemyEffectScale实际为0.1（保留），震屏6、子弹时间.05/.01/1/.01。本轮只改Enemy System与编辑器安装/验证代码，不改运行时伤害或Cue分支。结果未最终提交/push，地图与External Actor保持用户状态。

## 2026-09-05 条件子弹时间与Enemy Air 007（实现与自验完成）

- 用户确认：仅当本次爆炸实际造成新的Chaos破坏或击杀Enemy时触发子弹时间；普通爆炸/仅受伤不触发。同时指定Enemy身上爆炸使用TMIIR地图的N_ExplosionAir_007，实际爆点播放，环境仍Ground006。
- 选择性安全检查点4f15ed0保存上轮Enemy Control Rig和音效；不纳入地图/External Actor。新增ExplosionGun专属ExplosionOutcomeSubsystem，短期监听实际Chaos Break事件，所有受影响组件共享单次结果；死亡由范围伤害前后Health判定。现有BulletTime参数/恢复曲线不改。
- Development Editor Win64首次成功；新增九种真实PIE场景验证正在进行。TMIIR Overview_Map中同名Actor确认使用Air007、旋转identity、scale1；迁移115包，正在整理到本枪Effects/EnemyExplosion并冷验引用。尚未宣称完成。
- 最终：ExplosionOutcomeFirst.log的ExplosionOutcomeBulletTime Success，九场景覆盖空地、存活受伤、击杀、Chaos抗破坏、新破坏、散落碎块再击、击杀与破碎同时、墙后、禁用。每场验证实际血量/破碎状态、启动次数、恢复原速度及临时通知还原。原生代码构建成功；首次double时间戳收窄编译错误已改为double字段。
- ExplosionOutcomeRegression.log六项6/6 Success（BulletTimeAndPain、ExplosionChaosGround、ExplosionDirectionalShake、ExplosionRadialDamage、StickyExplosionAndBlood、ThreeWeaponBaseline），ExplosionOutcomeRigRegression.log补充EnemyExplosionControlRig Success。受伤动作、原Damage/Fuse/Chaos/声音/震屏/血迹均回归通过。
- Air007和115包依赖已按本枪独立所有权整理至Effects/EnemyExplosion，正式System为NS_ExplosionGun_EnemyDetonation。EnemyEffectOnGround=false，原EnemyScale1、时间.05/.01/1/.01、震屏6、环境音量3均保留。ValidateEnemyAirCold.log输出AIR_COLD_OK；115包全部可加载且在本枪新目录，无Redirector/供应商目录实体。源图N_ExplosionAir_007已确认同名System，未迁入地图或角色。
- Blueprint在UE打开/编译/保存成功；安装脚本仅在保存后的关闭编辑器步骤遇未暴露的Python方法，已修为close_all_editors_for_asset，并通过独立冷回读与实际PIE验证持久化，不盲目重复迁移。ResavePackages发现旧目录已无资产，磁盘仅空文件夹，检查绝对路径和文件数0后清理。
- 已查看实际PIE截图TMT_StickyExplosion.png，Enemy身体周围可见空中火花/烟雾；地面仍使用原Ground006。旧Audio配置脚本同步新的Air槽和开关，避免以后重跑还原Ground。结果未最终提交/push，地图/External Actor不纳入；用户本轮未要求关机。

## 2026-09-05 夜间自验：Enemy爆炸与Control Rig（实现/自动验收完成）

- 用户确认：Enemy爆炸使用N_ExplosionGround_006，制作方向性上半身Control Rig，合成独立音效；助手自行验收并记录后正常关机，用户明日手动验收。死亡动画/击退不在本轮范围。写前当前任务范围干净，14d9138已推送远端作为恢复点；地图和外部Actor保持原样，不创建空检查点。
- 新EnemyHitReactionComponent归人形敌人，保存爆点方向/部位与游戏时间包络；爆炸已通过原伤害/LOS筛选后请求，零距离使用缓存入射方向。默认22度/.055秒攻击/.55秒恢复，部分回弹，根/腿不动。Phantom专属Control Rig通过PostProcess AnimBP叠在原最终姿势上，避免改共享模板或替换locomotion；无重定向操作。
- 新原生FRigUnit_EnemyHitReaction分配spine_01/02/03权重.2/.35/.45，手臂或头部命中额外反应；输入由专用EnemyHitReactionAnimInstance在游戏线程采样，不在RigVM工作线程读Actor。
- 合成1.3秒48kHz单声道Enemy爆破声，程序化低频/噪声瞬态/能量尾声，峰值约-2.05dBFS，源脚本Scripts/Audio/synthesize_enemy_detonation.py。Sound Cue按arch14随机、3D衰减与并发。
- 初次Rig变量创建遇UE5.7 AddMemberVariable对FVector类型要求对象路径而非CPP名导致编辑器断言；只保存了空Rig，无动画/地图更改，修正为/Script/CoreUObject.Vector后继续。C++已成功编译，资产接入与PIE自验进行中。
- 完成：Phantom专属CR_Phantom_ExplosionReaction/ABP_Phantom_ExplosionReaction位于Phantom/Animations/ControlRig；SK_Mannequin.PostProcessAnimBlueprint已接通。EnemyEffect使用已迁入的N_ExplosionGround_006，EnemyEffectOnGround=true；EnemyExplosionSound为独立程序合成Sound Cue。全部Blueprint/Rig打开编译保存。ConfigureEnemyExplosion2在成功保存后Slate退出崩溃，未以此宣称验收；后续ValidateEnemyExplosionCold冷回读以及两轮独立PIE正常完成，证明持久化和运行路径有效。
- 最终Development Editor Win64成功，无新增源码警告。EnemyExplosionRigFinal.log七项7/7 Success/exit0：EnemyExplosionControlRig、ExplosionRadialDamage、BulletTimeAndPain、StickyExplosionAndBlood、ExplosionChaosGround、ExplosionDirectionalShake、ThreeWeaponBaseline。四方向头部实际位移同向>2cm，脚/胶囊不动，回到冻结原Pose；左臂命中有独立左臂响应；真实范围伤害触发Rig、墙后无Rig；Enemy音频实际播放一次、地面Niagara落点正确。原伤害/倒计时/Chaos/血花/痛呼/子弹时间回归通过。
- 已检查Saved/Screenshots/WindowsEditor/TMT_EnemyRig_Directions.png，四个人形呈不同方向偏转；Before图提供对照。冷验证检查新音频时长/单声道/Modulator/衰减/并发、Mesh→PostProcess→Rig引用、源Wave依赖和范围内无Redirector；工厂临时SK_Mannequin_CtrlRig.uasset磁盘不存在。
- 用户最新保存值回读：TimeScale=.05、SlowIn=.01、Hold=1、Recovery=.01、Inner/Outer=200/1500；CameraShakeScale=6、环境音量3，全保留。通用验证脚本不再钉死旧用户值。未修改死亡即时销毁流程、地图、外部Actor或共享locomotion/AimIK。
- 明日人工验收入口见progress.md。用户授权本轮自验/记录后正常关机，不强制关闭未保存程序；本轮结果未自动提交/push，14d9138仍为远端恢复点。

## 2026-09-05 平滑子弹时间与分支爆炸表现

- 用户确认保留同一Explosion GC，Enemy/环境独立声音与VFX；移除HitStop，改为先减速再平滑恢复；加强余震并排查穿模。安全检查点4f6f676保存前置工作，未纳入地图/外部Actor。
- 写前冷审计用户实际BP覆盖：CameraShakeScale=4、VolumeMultiplier=3、旧HitStop.Duration=.5（仍受旧.12上限约束）；原Damage=5、ExplosionDelay=2、范围20/400cm。保留震屏4/音量3与全部伤害、倒计时和Chaos数据。
- 删除旧HitStopSubsystem，新增Core/_Shared/Feedback/BulletTimeSubsystem和FBulletTimeSettings。BP_ExplosionGunBullet的Bullet|Explosion|Bullet Time默认TimeScale=.2、SlowInDuration=.05、HoldDuration=.08、RecoveryDuration=.25真实秒，距离200~1500cm。Smoothstep渐入/渐出，恢复进入前速度，不超速；活动期请求不重启/延长曲线，结束100ms恢复窗口；外部改速让出所有权，WorldEndPlay/Deinitialize恢复。
- 同一个GC新增EnemyExplosionSound、EnemyVolumeMultiplier=3、EnemyEffectScale=1；Enemy声音与VFX分别为空时跳过各层，不回退环境。暂未提供Enemy专用资源，两个槽保持空。环境原Ground投影、SCue_ExplosionGun_Detonation保持不变。
- 旧相机位移5cm乘用户4倍会产生约20cm未碰撞检测的视点偏移，而ArmsViewMesh仍挂HeadCamera，属于明确穿模风险。新爆炸Shake零平移，0.75真实秒/12Hz多次衰减旋转，原生幅度1.5度；Cue暴露ShakeDuration/ShakeFrequency/ShakeRotationDegrees，CameraShakeScale最多按8使用。每本地相机只保留本类一个爆炸Shake，不无限叠加；方位与200~1800cm平方衰减保留。不承诺解决独立的武器贴墙穿透。
- Development Editor Win64构建成功；ConfigureExplosionFeedback.log完成两个Blueprint打开/编译/保存，继承新默认，原伤害/音量/用户震屏覆盖回读通过。BulletTimeFeedbackRegression.log六项6/6 Success、exit0（BulletTimeAndPain、ExplosionDirectionalShake、ExplosionRadialDamage、StickyExplosionAndBlood、ExplosionChaosGround、ThreeWeaponBaseline）：曲线渐入/保持/恢复、重入不延长、原速恢复/外部所有权、弹体销毁/无GC、痛呼独立、方向/衰减/多次过零/全程零平移、伤害/Chaos全通过。ValidateExplosionFeedbackCold.log独立冷回读EXPLOSION_FEEDBACK_OK cold-read，新默认/用户覆盖/两个Enemy空槽/无Redirector均通过。未最终提交/push；实际震感与独立贴墙穿透仍需用户体验反馈。

## 2026-09-04 爆炸Hit Stop、Enemy分支与痛呼（本轮完成）

- 最新用户确认覆盖中途方案：Enemy和环境命中都附着倒计时，结束均触发Chaos/声音/震屏/HitStop/二次范围伤害，唯有爆炸Niagara不同。EnemyExplosionEffect独立可配、暂为空；不回退环境大地面decal。用户最终接受半径400cm、20伤害、只伤Enemy、同一Enemy一次、墙体阻挡，伤害/半径可调；原首次伤害不变。
- 检查点258bf70保留统一Sound Cue前置状态；地图与用户外部Actor保持原样。中途“Enemy立即销毁/只倒计时销毁”均已被最新要求撤销，不得按旧描述继续。
- 新Core/_Shared/Feedback/HitStopSubsystem负责单机世界时间：真实时钟、相机距离衰减、保存/恢复原速度、强者优先/连续上限、结束50ms恢复窗口、外部速度改变时让出控制及WorldEndPlay/Deinitialize清理。爆炸子弹参数默认0.06秒/0.05倍率/200~1500cm/0.12秒上限；GC不写时间或伤害。
- 新EnemyHitAudioComponent按Actor保存痛呼状态，不在Static GC CDO保存冷却；0.6秒真实时间间隔且同一敌人不重叠，附着/销毁停止。指定下载424116-Wizard-Pain-Vocal-Hurt-Uhh.wav导入S_Enemy_Pain，0.87579秒立体声；SCue_Enemy_Pain轻微随机0.97~1.03、音量0.95~1，共用命中3D衰减、SC_EnemyPain按Owner限制1条PreventNew。原肉体声5倍率不变。
- ConfigureEnemyPain.log首次导入/配置通过。新增ExplosionDamage/Radius/EffectClass，沿用GE_BulletDamage和Data.Damage；范围候选去重，先快照Visibility可见目标再施加GE/Chaos。源ASC消失时使用目标ASC构造保留Context的GE。两次中途C++编译通过；正在补充与运行HitStop/Pain/范围伤害及既有回归。
- 最终Development Editor Win64构建成功。HitStopPainDamageRegression.log六项全部Success（进程exit0）：HitStopAndPain、ExplosionRadialDamage、StickyExplosionAndBlood、ExplosionChaosGround、ExplosionDirectionalShake、ThreeWeaponBaseline。验证真实时间恢复原0.5速度、最强/连续上限、外部速度覆盖、弹体销毁/GC缺失不影响HitStop恢复，痛呼每敌人独立/冷却/再次播放/销毁停止。
- 范围伤害实测环境无即时伤害，爆炸后可见敌人100→80，墙后/范围外敌人100不变、范围内玩家不变；附着Enemy实测100→95→75且附近Chaos RootBroken。EnemyExplosionEffect空时即使带GroundHit也不生成环境地面Niagara；原环境Cue/Shake/清理与零伤害Hit保留。
- ConfigureEnemyPainFinal完成Enemy Hit/Explosion Cue/Bullet三Blueprint打开编译保存。ValidateEnemyPainHitStop冷启动回读ENEMY_PAIN_HITSTOP_OK，验证音频长度/声道、Modulator参数、衰减/按Owner并发、消费者/源音频依赖/无Redirector、HitStop默认和范围伤害默认。当前痛呼默认1，肉体5/爆炸3/震屏8保留。
- 参数位置与职责已同步arch07/09/10/14，工作面板已更新。未最终Git提交/未push，地图与用户External Actor保留原样。

## 2026-09-04 声音资产统一随机化（本轮完成）

- 用户确认把随机音高/音量移到Sound Cue，按用途共用衰减；打人只由角色Hit发声，环境由武器Impact发声，其他表现不变。以后新增音效必须沿用并验证规则。检查点d8badd4保留上轮随机血迹/3D命中结果，地图用户改动不纳入。
- AuditAllAudio.log全项目SoundWave/SoundCue/MetaSoundSource审计：13个有引用的源音频；三枪开火/空仓/命中、肉体声、爆炸声与TestGun共12个适合随机化；扫描识别音保持稳定作为例外。另2个RepairGun旧切枪/模板音频无引用，不添加原本不存在的播放行为。
- 新TheManAudioAssetLibrary编辑器助手创建标准Modulator/WavePlayer图，多素材支持Random无放回，拒绝覆写已有图。Scripts/Audio/configure_audio_cues.py显式消费者清单与冷验证入口；不移动或改写源Wave，不改用户5/3倍音量与8倍震屏。
- 已移除PitchVariation/CharacterSoundMultiplier代码随机；ShouldPlayImpactSound由普通武器拒绝Character、EnemyHit显式拥有自身声音。首次C++编译成功。资产脚本初次停在Factory Python名称差异，未改消费者；已核对引擎SoundCueFactoryNew，继续执行验证。
- 创建图时还发现UE5.7要求ChildNodes与图引脚数量一致，首个未保存Cue触发断言；现添加ReconstructNode后连接/编译，无消费者在失败时改写。ConfigureAudioCues3成功保存12个Cue和9个消费者Blueprint；ValidateAudioCues冷回读12/12、源Wave依赖、实际消费者、无Redirector及用户倍率5/3/8均通过。
- AudioPolicyRegression.log六项Success：AssetVariationPolicy、AmmoLifecycle、ExplosionDirectionalShake、SpatialImpactFeedback、StickyExplosionAndBlood、ThreeWeaponBaseline。覆盖实际Sound Cue源Wave图连接、随机区间/并发/3D，未来多素材无放回助手及拒绝覆盖，真实血花/肉体声/伤害/倒计时/震屏/弹药。
- 实际输出六份WAV均非零：肉体近左RMS=0.14701/0.02558、近右0.02723/0.15658、远左0.06671/0.01155；电击近左0.16916/0.02928、近右0.02967/0.17139、远左0.04102/0.00710。左右优势>2倍、远处总RMS<近处70%断言通过。
- 最后补充三枪实际OnExecute携带角色HitResult的声音组件计数断言：AudioPolicyHitRouting.log的SpatialImpactFeedback为Success，三枪均不创建武器声音；原Niagara仍播放。补丁后Development Editor Win64再次成功。测试背景音量覆盖增加析构恢复，失败退出也不遗留设置。
- 接入规范已写AGENTS.md与arch/14-audio-policy.md；以后新音效需加Sound Cue随机/用途衰减/并发及消费者验收，例外说明原因。结果未最终Git提交/未push，用户地图与前置Chaos资产不变。

## 目标

- 为 RepairGun 接通现有第一人称开火蒙太奇。
- 以 RepairGun 为完整配置基线创建电击枪和爆炸枪。
- 两把新枪只替换模型、枪口 VFX、命中 VFX 与贴花；玩法、动画、音频、弹药、GAS、后坐力和子弹行为保持与 RepairGun 一致。
- 外部来源仅迁移最终模型和 VFX 依赖，不迁移源项目角色、武器蓝图或动画。

## 2026-09-04 随机命中与3D音效（本轮完成）

- 用户确认血迹/弹痕随机尺寸、旋转、图案，修正身上附着，并把命中声改为3D远近/左右定位，提升肉体声可辨性；伤害和爆炸Fuse不变。检查点8f2f1e6保存前置不规则Chaos结果。只读AuditHitFeedback.log发现用户EnemyHit音量已改5，e1ed363单独保存该覆盖；不能继续按旧文档1处理。
- 四Cue/声音均无Attenuation。新共用Core/_Shared/Audio/SA_ProjectileImpact启用Spatialize/Attenuate，Sphere180cm+2200cm线性衰减、无立体声spread。各枪命中Enemy时武器撞击层降为0.3，Enemy肉体声保持用户5；分别配置12/8路并发上限，轻微随机音高。
- ImpactFeedbackBase统一SpawnImpactSound并传入衰减/并发；弹痕随机roll、尺度/长宽及独立MID噪声偏移。血迹原材质与两枪新增专属DecalVaried材质仅扩展Opacity，不动颜色/原图/寿命链。EnemyBody重新Trace到Mesh表面，使用其法线和骨骼，失败向最近骨骼补Trace，不在胶囊点悬空生成。
- InstallHitFeedbackAssets.log与ConfigureHitFeedback.log完成资产/四Cue编译保存。Development Editor Win64构建通过；新增真实输出录音与骨骼附着/声音播放验证进行中。
- 最终Development Editor Win64编译成功。HitFeedbackFinal.log六项回归全部Success：SpatialImpactFeedback、StickyExplosionAndBlood、ExplosionChaosGround、ExplosionDirectionalShake、IrregularCubeGeometry、ThreeWeaponBaseline。涵盖随机MID/尺寸/旋转、实际肉体AudioComponent播放、身上贴花附着骨骼并随目标移动，原伤害/Fuse/Chaos不变。
- 新Opacity图初版存在Noise输入连接错误，已用显式连接成功断言修复；三项材质实际Shader错误数为0。已查看TMT_EnemyBloodHit_Isolated.png，绿色错误材质方块消失，身体/地面出现暗红斑迹；细节最终观感仍以用户游戏内体验为准。
- 初次录音发现用户5倍肉体音量削波，增加独立SMX_EnemyFleshHit/SFX_EnemyFleshLimiter，不改素材或用户倍率。后台编辑器全静音曾使合并回归录音为0；测试临时设UnfocusedVolume=1，结束恢复，未修改项目全局音频配置。
- HitSpatialFocused.log独立冷启动SpatialImpactFeedback成功，六份WAV均非零；肉体近左RMS L/R=0.14946/0.02601、近右0.02527/0.14509、远左0.07442/0.01288；电击近左0.17801/0.03082、近右0.02983/0.17231、远左0.04271/0.00739。两类左右优势均超过2倍，远处总RMS低于近处70%，实际3D空间与距离断言通过。限幅后肉体近处削波样本从约1224降至33，未宣称完全无削波。
- 本轮结果未最终提交/未push；用户地图/External Actor改动保持原样。检查点8f2f1e6与e1ed363可追溯前置状态。

## 2026-09-04 不规则真实破碎

- 用户不接受规则3x3x3分块，确认改为不规则Voronoi、大小错落、凹凸断面与随机翻滚；伤害、Fuse、Cue、Ground规则不动。选择性检查点db9a156保存前置源码/文档/Cube资产，地图及用户新增外部Actor未纳入。
- CreateTestCubeAsset(bRebuild)改为在临时UGeometryCollection中切割完整100cm Cube：24个分散随机点+18个局部密集点，固定seed92417；原生PlanarCut/Voronoi，断面noise幅度0.8cm、间距5cm，零grout。成功后替换同一GC资产的几何与材质槽，保留使用方路径；常规调用不重建，只有显式true重建。
- Bullet新增ChaosAngularSpeed默认5rad/s；保留径向冲量，延迟释放碎块时对半径内逐粒子随机角速度，仅非Enemy分支。PlanarCut插件仅Editor启用，Build.cs的PlanarCut/Voronoi也仅Editor依赖，运行时不切网格。
- RebuildIrregularCube.log已保存正式GC并打开/编译/保存Cube蓝图；未保存或重新布置地图。新增形状统计和回归验证进行中。
- 最终Development Editor Win64构建成功；IrregularCubeFinal.log中IrregularCubeGeometry、ExplosionChaosGround、ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline 5/5 Success。冷读取统计42个刚体块，体积284.6~72946.6cm³，总体积1000000.1cm³，12085个非轴对齐顶点法线；总量保持100cm实心Cube，没有规则网格碎块。
- PIE真实Sweep附着、延迟RootBroken/飞散、范围外不碎、地面VFX、Enemy分支排除、声震/血花/原伤害回归全部通过。测试区新增临时照明（不写地图），已查看TMT_ChaosCube_Detonated.png，大小错落且斜面碎片清晰可见。
- 仅覆盖既有GC和编译Cube BP，未删除资产；原规则版可从db9a156恢复。地图/外部Actor包保持用户前置工作区状态，结果未最终提交/push，FEAT-080保持in_progress。

## 2026-09-04 Chaos Cube 与爆炸地面投射

- 用户确认：可摆放Chaos Cube类，爆炸弹非Enemy命中时倒计时触发Chaos；Enemy分支不扩展，不增加伤害。实际爆点负责物理/声音/震屏，Ground Niagara只在Actor Tag `ExplosionGround`且坡度合格的表面播放。检查点f199564保存前轮音量覆盖。
- 新增Actors/DestructibleCube/ChaosDestructibleCube，27块闭合Cube网格组成真实聚类GeometryCollection；FractureAsset/Toughness与Actor Scale可调。触发逻辑留在ExplosionGunBullet，半径400cm、Strain500000、速度变化1200；初碰不会自动碎。
- GroundSearchDistance=2000cm，GroundMaxSlope=45度；Object Multi Trace穿过Cube，拒绝Enemy/弹体/GeometryCollection。Params.Location保持实际爆点，EffectContext.HitResult仅携带合格地面；无地面跳过Niagara，保留声/震。
- Development Editor Win64初次构建成功。GASPTest Plane、VFXTestMap VFXTest_Floor已标记；VFXTestMap新增3个不同尺寸Cube。TestMap分区地形标记及完整PIE验证进行中。
- 第一次NullRHI脚本在摆放Actor时触发引擎除零异常，已改用D3D编辑器完成资产/关卡保存；未将崩溃当作保存成功，已重新加载现有资产后续作业。
- 初版GC仅组装网格，缺少凸包/体积等数据，实弹穿透；补充UpdateGeometryDependentProperties后创建SimulationData，实弹附着、RootBroken与碎块位移全部通过。测试临时地板需先Movable再SetStaticMesh；坡度案例用薄平板，避免旋转立方体另一面实际仍合格。
- TestMap的Landscape及64个StreamingProxy通过WorldPartitionBlueprintLibrary加载并写Actor Tag，共65项外部Actor包；GASPTest/VFXTestMap各仅一个真实地板被标记。没有标记天花板、墙或Cube。ValidateChaosAssets.log独立冷回读全部65地形、两地板、3Cube及无Redirector，三项Blueprint编译保存；音量3/震屏8保持。
- ExplosionChaosPhysics.log：ExplosionChaosGround与StickyExplosionAndBlood 2/2 Success，验证真实ProjectileSweep附着、倒计时后RootBroken、半径外不碎、碎块扩散、地面落点与Enemy不引爆附近Cube；原5点伤害/零伤害血花/清理保持。最终增加缺失地面不生成Niagara断言与两张PIE截图，四项回归见ExplosionChaosFinal.log。
- 最终ExplosionChaosFinal.log：ExplosionChaosGround、StickyExplosionAndBlood、ExplosionDirectionalShake、ThreeWeaponBaseline全部4/4 Success。已查看Detonated截图，可见碎块与地面效果；Intact截图因测试区域远离灯光较暗，不作为外观验收。当前未最终提交，整体FEAT-080仍in_progress。

## 2026-09-04 指定爆炸音量再增强

- 用户确认将当前Alien Cannon爆炸声从VolumeMultiplier=1改为3；只改正式Explosion Cue的该值，不修改音频样本、敌人音量或震屏。检查点b0fb3ec保存前轮音频替换。
- 冷回读发现当前CameraShakeScale实际为8，保持不变（与旧文档3不同）；EnemyHit.VolumeMultiplier保持1。BoostAlienExplosionFinal.log的saved=3/实际shake=8为准，末尾旧固定打印shake=3不作为证据。首次验证错误仅为脚本硬编码shake=3的错误假设，未修改震屏。
- Blueprint在UE编译保存成功，独立回读确认爆炸3/敌人1。回归测试不再把可调震屏值钉死为3，改为检查配置值与距离衰减的关系；Development Editor Win64构建成功。测试见AlienVolume3Final.log。
- 单声源理论峰值会超过0dBFS，可能触发混音限制或失真；这次按用户明确确认保留3倍增益，没有宣称无削波或主观响度正好三倍。
- AlienVolume3Final.log独立D3D启动ExplosionDirectionalShake测试Success；当前只改正式Cue音量及对应测试/文档，无最终Git提交。

## 2026-09-04 指定爆炸/肉体命中音频与震屏增强

- 用户指定Downloads/512565-Alien-Game-Explosion-Robot-Cannon-4-Big-Hard-Impact-Glitchy.wav用于爆炸、166553-Bullet-Hit-Body-Flesh_05.wav用于敌人Hit，并确认增强震屏及删除无用资产。检查点 `4a40cd8` 保存前轮零伤害Cue修复。
- 源音频审计：爆炸6.960秒/96kHz/24bit/stereo，峰值-0.50dBFS、RMS-16.79dBFS；肉体0.565秒/48kHz/24bit/mono，峰值-1.34dBFS、RMS-17.53dBFS。保留原WAV样本，不套旧合成音频3倍增益；两Cue音量倍率1，避免单声源超满幅。
- 爆炸音频新路径 `Weapons/ExplosionGun/Audio/S_ExplosionGun_AlienDetonation`，敌人受击 `Enemy/_Shared/Audio/S_Enemy_FleshHit`；源码WAV跟随各自资产放入项目，Downloads原文件不动。分别配置Explosion Cue.ExplosionSound和默认EnemyHit.ImpactSound。所有武器原Impact/Fire音频保留。
- CameraShakeScale从1提高到3（正式Cue及原生默认），200/1800cm距离范围与0.45秒时长不变，仍保持方位偏向和距离平方衰减。
- 通过完整D3D编辑器Python导入/编译保存，避免无音频commandlet解码问题。旧S_ExplosionGun_Detonation经硬引用确认无使用方后用EditorAssetLibrary删除，旧合成WAV同步删除，两项可从4a40cd8恢复；未删除其他在用VFX/音频。
- Development Editor Win64编译成功；ValidateUserAudio.log冷回读音频路径、时长、两Cue倍率、震屏强度、旧音频不存在及无Redirector，0错误0警告。新素材引用/3倍震屏断言与零伤害/爆炸回归见UserAudioShakeFinal.log。
- 最终UserAudioShakeFinal.log：ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline全部3/3 Success/exit0，D3D运行无音频解码错误。核验两段指定声音资产与3倍震屏配置；原零伤害血花、正伤害、附着倒计时和自动清理仍通过。

## 2026-09-04 零伤害有效命中触发敌人 Cue

- 用户明确确认零伤害命中也出血花，不改变Damage。写入前检查点 `dca1224` 保存前轮3倍爆炸音量/方位震屏。
- ABulletBase在原穿透/重复Hit门禁与GE逻辑之后，仅当Damage==0且有效命中Enemy时，携带真实HitResult显式调用目标ExecuteHitReactionCue(Context,0,true)。SourceASC缺失时使用目标ASC创建Context；不伪造伤害，不新增GE。
- AEnemyBase::ExecuteHitReactionCue新增默认false的bAllowZeroDamageHit。普通Health回调对0/治疗仍不触发；显式零伤害Hit即时InvokeGameplayCueEvent，正伤害仍沿原ExecuteGameplayCue流程，避免一次命中两次血花。Phantom穿透和重复命中仍提前返回。
- BP_ElectricGunBullet.Damage保持0，未改蓝图/伤害资产。Development Editor Win64编译成功。StickyExplosionAndBlood增加正式电击弹的零伤害血花、生命不变、重复Hit与穿透断言，并对原正伤害血花数量严格断言1；验证日志ZeroDamageEnemyHit.log。
- 最终D3D PIE：ZeroDamageEnemyHit.log中StickyExplosionAndBlood、ThreeWeaponBaseline、ExplosionDirectionalShake全部3/3 Success/exit0。确认电击弹伤害0、血花+1、重复Hit不增加、Phantom穿透不增加、Health不变；正伤害仍仅一次血花。无资产改动，无最终Git提交。

## 2026-09-04 爆炸三倍音量与方位震屏

- 用户确认爆炸音量至少原来的3倍，并按相对玩家方位加入震屏。写入前本地检查点 `84aa9cd` 保存上一轮附着弹/爆炸Cue/默认血迹；本轮不自动最终提交。
- `UGCN_ExplosionGunExplosion.VolumeMultiplier` 默认从1改为3，正式Cue继承并通过UE编译保存确认。保留原合成SoundWave，不重复合成/叠播，不改变原命中或开火声。
- 新 `UExplosionCameraShake / UExplosionCameraShakePattern` 位于本枪 `Effects/ExplosionCameraShake.h/.cpp`。0.45秒有限衰减冲击，位移主轴沿爆炸点→相机的UserDefined播放空间，含旋转抖动；不改变ControllerRotation，不替代开火震屏。
- 爆炸Cue配置 `CameraShakeClass / CameraShakeScale / ShakeInnerRadius / ShakeOuterRadius`，默认强度1、200cm以内全强、1800cm以外0、中间平方衰减。遍历本地玩家相机，可见方向由爆炸位置而不是表面法线决定。
- 电击枪不出血花根因已确认：`BP_ElectricGunBullet.Damage=0`，HitEffectClass仍为GE_BulletDamage；默认敌人Hit Cue只响应DamageTaken>0。不是材质缺失。已异步询问用户保留零伤害但命中也出血，还是伤害改5；未得到选择前保持原伤害/触发规则，不擅改。
- `ExplosionAudioElectricAudit.log` UE配置/编译保存0错误0警告；Development Editor Win64编译通过。方向/衰减/自动结束单测与D3D爆炸Cue链回归见 `ExplosionDirectionalShake.log`。
- 最终 `ExplosionDirectionalShake.log`：ExplosionDirectionalShake、StickyExplosionAndBlood、ThreeWeaponBaseline 3/3 Success/exit0。验证正式Cue音量3、左右爆炸相反偏移、距离倍率、0.45秒自动结束、真实弹体爆炸Cue向本地PlayerCameraManager添加震屏；原5点伤害与特效清理回归仍通过。测试编辑器正常退出。

## 2026-09-04 附着倒计时爆炸与默认敌人血迹

- 用户确认增加附着弹与可调倒计时；明确保留首次伤害和全部已有武器命中表现，本轮爆炸仅表现，不追加范围伤害。追加要求默认 Enemy Hit Cue 提供血迹、短促飙血。写入前检查点 `5f029d4`，结果未最终提交。
- `AExplosionGunBullet : ABulletBase` 位于 `Weapons/ExplosionGun/Bullets`；先执行基类有效命中路径，再停止移动/碰撞并附着命中组件（角色优先细化到骨骼 Mesh）。`BP_ExplosionGunBullet` 改父类并设置 `bDestroyOnHit=false`，`ExplosionDelay=2s`。重复 Hit 不重置倒计时；零秒走下一 Tick；EndPlay 清理 Timer。隐身 Phantom 仍穿透。
- 新 `UGCN_ExplosionGunExplosion` 位于本枪 `GAS/GameplayCues`，由弹体倒计时结束调用 `GameplayCue.Weapon.ExplosionGun.Explosion`；Niagara 按表面法线的 +Z 方向生成，声音/EffectScale/VolumeMultiplier 都在 `GC_Weapon_ExplosionGun_Explosion` 配置。Source ASC 失效时通过 CueManager 直接播放。
- TMIIR `/Game/NiagaraExplosion01/Niagaras/Ground/N_ExplosionGround_006` 和递归依赖共116包经 AssetTools 迁入，整理到 `/Game/Weapons/ExplosionGun/Effects/Explosion`；系统名 `NS_ExplosionGun_Detonation`。迁移前保留并恢复56个材质纹理参数。独立合成低频爆破 WAV `Weapons/ExplosionGun/Audio/S_ExplosionGun_Detonation.wav`，1.4秒/48kHz/mono，与原命中/开火声独立。
- 原默认 `GC_Character_Enemy_Hit` 的 ImpactEffect/ImpactSound/ImpactDecalMaterial 均空。现在 `UGCN_EnemyHit` 自身生成敌人附着血迹及近处环境血迹，并生成0.55秒自动销毁的 `AEnemyBloodSpray`（9张无碰撞、无阴影、面向相机的短促飞溅卡片）。默认血迹12秒，最后2秒淡出；不修改通用武器 ImpactFeedbackBase，不制作金属弹孔。
- 血迹源纹理由 imagegen skill 的内置 image_gen 生成，保留真实 Alpha（采样范围0–255）；源 PNG 与 Unreal Texture 位于 `Enemy/_Shared/Effects/Hit/Textures/T_Enemy_BloodSplatter`，独立 `M_Enemy_BloodSpray`/`M_Enemy_BloodStain`。生成提示词见同目录 `BloodSplatter-generation.md`。
- Development Editor Win64 构建成功；`StickyBloodD3D.log`：`StickyExplosionAndBlood` 和 `ThreeWeaponBaseline` 2/2 Success。PIE 验证初次5点伤害、重复Hit不叠伤、附着跟随、停止碰撞/速度、可调倒计时、0秒/缺少SourceASC路径、延时后子弹销毁及Niagara保留、不追加伤害、真实伤害触发血迹/喷溅与喷溅销毁。
- `ValidateStickyBlood.log` 冷加载验证两项 GameplayCueName 精确匹配正式Tag，递归116包均owner-local、材质纹理参数非空、无Redirector；供应商目录在Registry与磁盘均清空。首次无界面导入发生音频解码ensure/Interchange Slate退出，但资产已保存；独立二次安装0错误0警告，D3D真实音频设备测试无相关错误。定向Fixup提示无包，因为AssetTools已移除旧包，随后仅清理确认空的目录。
- 延长PIE至14秒的清理测试发现源Niagara尾焰仍active，因此Explosion Cue新增可调 `EffectLifeSpan=8s`，通过绑定Niagara组件的weak timer兜底销毁，独立于弹体与发射者。血迹12秒到期销毁断言通过。视觉测试保存 `TMT_StickyExplosion.png`、`TMT_EnemyBloodHit_Isolated.png`，最终清理回归见 `StickyBloodFinal.log`。
- 最终 `StickyBloodFinal.log`：StickyExplosionAndBlood + ThreeWeaponBaseline 2/2 Success，包含14秒时爆炸组件active为0、血迹组件已过期为0的新断言。已查看隔离截图确认腿部深红受击飞溅/血迹及地面红色斑点可见、无方形透明底；爆炸截图保留源橙黄大范围能量闪光。`ValidateStickyBloodFinal2.log` 再次编译保存两项Cue、冷回读Tag与116包依赖成功。没有关卡写入、没有最终Git提交，测试编辑器均正常退出。

## 2026-09-04 爆炸枪高能核心材质

- 用户认可电击枪并要求只换爆炸枪材质，确认“深色金属、橙红能量舱、亮黄脉冲核心、槽线流动”方向。检查点 f3c38f0 保存上一轮两枪材质及灰壳/描边/缩放修复。
- 仅修改 ExplosionGun/Materials/M_ExplosionGun_Surface 与 MI_ExplosionGun_Rifle：侧面双能量舱以局部坐标定位，橙红外层、亮黄中心与环状热能纹理；核心0.65Hz平缓脉冲，槽线亮点以独立0.45速度流动。深色金属替换原黄铜配色，提高粗糙度，限制发光在能量舱/槽线，保留共享显现函数。
- 参数：CorePulseRate、EnergyFlowSpeed、PlasmaShellColor、PlasmaCoreColor、PlasmaIntensity。不修改模型、开火Niagara、灯光、默认尺寸或玩法代码；ElectricGun资产无变动。
- ExplosionEnergySurface.log 材质编辑/参数验证成功0错误0警告；ExplosionEnergyD3D.log 的 SharedEquipReveal、ExplosionVisualCapture、ThreeWeaponBaseline 全部3/3 Success/exit0。已查看 TMT_WeaponSurface_2.png 实机截图并另存 TMT_ExplosionEnergyCore.png，橙红舱体和亮黄核心可辨，原表面显现保持正常。无C++改动，无新增资产/删除。

## 2026-09-04 枪体材质、灰壳与爆炸枪尺寸修复

- 用户确认实施：修复切换时灰模、参考 RepairGun 做两枪科幻材质、删除三枪描边及无引用资产、按 ElectricGun 接通 ExplosionGun 的真实枪口尺寸倍率。写入前检查点 `3be752e` 保存前轮共享装备 VFX 工作。
- 排查发现 Outline 材质无 Amount (S)，旧共享组件会把描边壳替换为通用不透明显现表面；移除该备用替换，MID 只继承原材质，未接溶解契约的材质保持原样。三把当前武器表面均已接入共享溶解函数；新装备仍自动继承组件，但自定义材质需接同一函数才能参与溶解。
- 删除 AFirearm.StaticMeshOverlay 原生组件；三 BP 清空旧引用并重新编译保存，按外部引用检查删除三把枪 Outline mesh 与 M_EquipmentOutline（4项），可从检查点恢复。
- 新建各枪专属 M_ElectricGun_Surface / M_ExplosionGun_Surface：参考 RepairGun 的 PBR 分层但不套用不匹配 UV 的维修枪贴图。使用枪体局部坐标生成面板接缝、分区金属、能量嵌条、微法线和粗糙度变化，配合原有 owner-local SurfaceDetail 纹理。电击青蓝，爆炸黑化金属/黄铜/橙；保留共享显现函数，不使用整枪 Fresnel 发光。
- 原共享 M_EquipmentEquipSurface 不再有两个使用方，迁回 RepairGun/Materials/M_RepairGun_Rifle，共享目录只保留显现函数和噪声。两枪独立材质实例改为各自表面主材质。
- 通过一次性 Editor migration 按 ElectricGun 已验证的 Spawn ScaleSpriteSize 模块模式，为 PhysicalMuzzle 全部9发射器追加 Uniform Scale Factor = User.MuzzleScale；默认参数1，枪体BP倍率2。迁移代码及临时 NiagaraEditor 构建依赖已移除，并重新冷编译成功。
- ValidateWeaponSurfacesFinal.log：三枪可见表面、无描边组件、材质图输出、纹理参数、蓝图编译、Redirector 校验成功0警告。检查过程中对 RepairGun 11 个材质产生的非语义重存已精确恢复检查点版本，不改变维修枪自身材质。
- Development Editor/Win64 最终构建成功；WeaponSurfacesD3DFinal.log 五项 SharedEquipReveal、EquipDissolveEvidence、ExplosionVisualCapture、ThreeWeaponBaseline、ThreeWeaponPIESwitch 全部 Success/exit0。新增 MID.Parent 与原表面精确一致断言、兼容普通装备/不兼容槽保留原材质断言。
- 首轮 ThreeWeaponPIESwitch 因 TestMap 加载卡顿导致固定 wall-clock 等待早于游戏时间显现完成，切枪输入被正确锁定拒绝；测试已改为最多15秒等待实际可见且显现结束后发输入，不改玩法锁。最终通过。
- ExplosionScale1D3D.log 的1倍实际射击与显现证据均 Success；1/2倍对照截图 TMT_ExplosionScale_1.png / _2.png 已查看，2倍闪光、Sprite与折射范围明显增大。参数覆盖仅运行时测试对象，不改 BP 的2倍默认。
- 已查看 TMT_WeaponSurface_1.png / _2.png（电击/爆炸）和 TMT_EquipReveal_2.png：金属分区、青蓝/橙能量嵌条可见，显现时保留同一表面，没有灰壳。既有 M_UE4Man_Body 缺图材质警告仍与本轮无关。结果未最终提交，待用户审核观感。

## 2026-09-04 通用装备显现与默认枪口倍率

- 用户要求所有装备共用爆炸枪同款切换 VFX，但可有各自动画，并将默认 MuzzleEffectScale XYZ 改为 2。已将 AFirearm 原生默认改为 `(2,2,2)`，BP_ExplosionGun 同步覆盖；保留 ElectricGun 当前 2 倍与 RepairGun 专属 0.85 倍。
- 定位到 RepairGun 实际显示的 SkeletalMesh 使用 M_SCFR_BaseMat，不含 Amount (S)，旧代码对它写参数无效；ElectricGun 的 Noise 被枪体 SurfaceDetail 贴图覆盖，导致溶解图案与源效果不同。
- 新增 `Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h/.cpp`，由所有 AEquipmentBase 原生创建。组件持有 0.5 秒 Hermite 1→0 时序、临时 MID、原材质恢复及取消逻辑；只在播放期间 Tick。没有溶解契约的未来自定义材质会在显现期间使用共享备用表面，结束/卸下时恢复原材质；接入共享函数的材质全程保留自身外观。
- AEquipmentBase 只保留 PlayEquipEffect 委托、Equip/Unequip 和动画配置；新增独立 `bPlayEquipAnimation`（默认 false）允许同时播放每件装备自己的 EquipMontage，EquipmentAnimLayerClass 仍独立。切换和首次装备统一由 EquipmentManager.QueueEquipPresentation 延迟一帧等待姿势，然后播放 VFX/可选动画并显示；移除 FPSCharacterBase 的 PlayInitialEquipEffect 与重复入口。切换锁定直接读取组件是否仍播放，不再复制固定时长定时器。
- 将爆炸枪工作正常的主材质、描边材质和函数通过 AssetTools 提升到 `/Game/Weapons/_Shared/Equipment/Effects/Equip/Materials/{M_EquipmentEquipSurface,M_EquipmentOutline,Functions/MF_EquipmentEquipDissolve}`。专用噪声 `Textures/T_EquipmentEquipNoise` 为共享副本，保留仍被枪口/命中使用的原 VFX 噪声。
- ElectricGun / ExplosionGun 的独立材质实例及 RepairGun 静态材质引用共享父材质；RepairGun 实际骨骼材质在原颜色、法线、金属度、纹理和发光图后接入同一个函数。ElectricGun 与 RepairGun 静态 Outline 槽使用共享描边，所有当前表面统一使用共享噪声。
- 删除 4 项已无引用的旧 RepairGun/ElectricGun 父材质与各自溶解函数；清理三把枪空的 Materials/Functions 目录。旧内容可从写入前 WIP checkpoint `2dff0c6` 恢复，结果未提交。
- Development Editor / Win64 构建成功；冷启动验证共享父材质依赖闭包 3 项全部位于共享 Equip 目录、三把枪可见材质均接通共享函数/噪声与继承组件，相关 Blueprint 编译保存通过。D3D 首轮 SharedEquipReveal、ThreeWeaponBaseline、ThreeWeaponPIESwitch、ExplosionVisualCapture 均 Success。三张显现截帧为 `Saved/Screenshots/WindowsEditor/TMT_EquipReveal_0.png`、`_1.png`、`_2.png`。旧 EquipDissolveEvidence 初次因 PIE 启动工作错过首帧采样失败，已改为测试开始时明确启动测量播放，最终 D3D12 SharedEquipReveal、EquipDissolveEvidence、ThreeWeaponBaseline、ThreeWeaponPIESwitch 4/4 Success。

## 2026-09-04 爆炸枪按截图替换为 Physical 1

- 用户明确授权替换模型、特效、光源和清理旧资产。截图 `屏幕截图 2026-09-04 192212.png` 中 `BP_Weapon_Rifle_Physical_Child` 在源 VFXPack 内重定向到 `BP_Weapon_Rifle_Physical_01_Child`。
- 新枪体来自 `SM_Weapon_Ballistics_Rifle_01` 及 Outline，正式命名 `SM_ExplosionGun_Rifle` / `SM_ExplosionGun_Rifle_Outline`。主材质保留源配置，正式命名 `MI_ExplosionGun_Rifle` / `M_ExplosionGun_Rifle`；源蓝图组件额外指定的 `MA_Example_Item_HackyOutline` 单独迁入为 `M_ExplosionGun_Outline`，写入 Outline Mesh 材质槽，消除空材质槽警告。
- 枪口来自 `NE_VFX_Muzzle_Physical_Burst_1`，命中来自 `NE_VFX_Projectile_Impact_Physical_1`，正式路径分别为 `/Game/Weapons/ExplosionGun/Effects/Muzzle/Systems/NS_ExplosionGun_PhysicalMuzzle` 与 `/Game/Weapons/ExplosionGun/Effects/Impact/Systems/NS_ExplosionGun_PhysicalImpact`；黄色环境贴花继续 2.0，弹体和玩法配置保持原值。
- 主模型相对位置 `(0,-16.757669,3.554176)`；枪口按源 MuzzleFlashLoc 相对握把变换为 `(0.000174,41.751608,8.507415)` / Yaw 90。点光源使用源射击强度 300（源组件静态默认 500，开火变量覆盖为 300）、线性颜色 `(1,0.551385,0.147041)`、半径 87.370407、SourceRadius 60、0.1 秒淡出；局部偏移 `(8.851011,-17.618745,2.96144)` 与源灯相对枪体位置一致。枪口倍率 1。
- AssetTools 迁移/重命名 41 项依赖；18 项有效纹理参数在迁移时保留并冷启动验证。递归依赖闭包均位于 ExplosionGun；旧模型与两套 Physical 3 系统的依赖图按外部引用保护子树后删除 43 项，3 项仍被现有资产引用的依赖保留。供应商目录、迁移空目录和范围内 Redirector 均清空。
- 用户 ElectricGun 当前尺寸为 `(2,2,2)`，已保留；写入前本地 WIP checkpoint 为 `f5b8fb5`。既有基线测试同步新资产路径、ExplosionGun 开灯与 ElectricGun 用户倍率；新增真实开火截帧验证消耗弹药、点光开启/淡出与渲染画面。
- Development Editor / Win64 构建成功；冷启动 `EXP_VALIDATE|DONE|closure=41`；D3D12 首轮和补齐 Outline 材质后的最终 ThreeWeaponBaseline、ThreeWeaponPIESwitch、ExplosionVisualCapture 均 3/3 Success；爆炸枪空材质槽警告消失。现存 RepairGun Outline 空槽、M_UE4Man_Body 纹理和 AimIK 警告不属本轮范围。截图：`Saved/Screenshots/WindowsEditor/TMT_ExplosionGun_Physical1.png`。当前 FEAT-080 继续 in_progress，后续爆炸弹玩法另行推进；结果未提交。

## 源资产调研

- 外部项目：`D:\Unreal Projects\UE389_MuzzleSource\VFX Pack - Stylized FPS Muzzle and Impacts Effects 5.1\VFXPack`
- 电击枪来源：`BP_Weapon_SMG_02_child`，模型 `SM_Weapon_SubmachineGun_02`，枪口 `NE_VFX_Muzzle_Energy_Burst_3`，环境命中粒子为空，紫色贴花，尺寸倍率 1.1。
- 爆炸枪来源：`BP_Weapon_Rifle_Physical_02_Child`，模型 `SM_Weapon_Ballistics_Rifle_02`，枪口 `NE_VFX_Muzzle_Physical_Burst_3`，命中 `NE_VFX_Projectile_Impact_Physical_3`，黄色贴花，尺寸倍率 2.0。
- 源项目 15 个武器 Blueprint 均不直接引用 FPS 动画；角色统一使用 `FirstPerson_AnimBP`，投射物武器父类统一使用 `FirstPerson_Recoil_Large_Montage`。
- TheManTest 已有 `AS_MaintenanceWorker_FP_Fire` 与引用它的 `AM_MaintenanceWorker_FP_RecoilLarge`；RepairGun 当前未配置 `FireMontage`。

## 资产所有权

- `/Game/Weapons/ElectricGun/...`
- `/Game/Weapons/ExplosionGun/...`
- 即使依赖内容相同，也为每把武器复制并语义化重命名专属版本，避免后续调参互相影响。
- 不保留供应商目录；移动和重命名通过 Unreal AssetTools 完成。

## 实施记录

- 2026-09-03：用户确认开始实施。FEAT-079 的实际应用验收暂缓并转 `needs_improvement`；已通过自动化的实现以 WIP checkpoint `5a39440` 封存。
- 2026-09-03：通过 Unreal AssetTools 迁移两把枪所需模型、轮廓模型、Niagara、材质、纹理及依赖，并按所有权重命名到 `/Game/Weapons/ElectricGun`（45 个资产）和 `/Game/Weapons/ExplosionGun`（60 个资产）；未迁移源项目角色、武器蓝图或动画。
- 2026-09-03：RepairGun 原第一人称开火序列/蒙太奇和装备蒙太奇归档并重命名为 `AS_RepairGun_FP_Fire`、`AM_RepairGun_FP_Fire`、`AM_RepairGun_FP_Equip`；`BP_RepairGun` 已接通开火蒙太奇。
- 2026-09-03：创建 `BP_ElectricGun`、`BP_ExplosionGun` 及各自的 Bullet、GameplayCue、AnimBP、开火/装备动画、音频、CameraShake 和子弹表现副本。两把枪的开火蒙太奇内部已改为引用各自的开火序列，不再依赖 RepairGun 开火序列。
- 2026-09-03：通用命中 Cue 增加可配置贴花材质、尺寸倍率和生命周期。电击枪使用 Energy Burst 3 枪口、无粒子命中、紫色贴花 1.1；爆炸枪使用 Physical Burst 3 枪口、Physical Impact 3 命中和黄色贴花 2.0。
- 2026-09-03：新增两个原生 GameplayCue Tag 和扫描路径；`BP_MaintenanceWorker.InitialEquipmentClasses` 按 RepairGun、电击枪、爆炸枪顺序配置三把枪。
- 2026-09-03：定向依赖检查发现复制后的两个弹体 StaticMesh 仍引用 RepairGun 弹体材质；已通过 StaticMesh 正式材质接口改为各自 `M_<WeaponName>_Bullet` 并冷回读确认，对 RepairGun 的依赖降为 0。迁移脚本曾强制重存 RepairGun 整个目录，收尾时已精确还原无语义变化的材质、纹理和 Niagara 文件，只保留动画重命名所需引用及 `BP_RepairGun` 配置改动。

## 验证结果

- Development Editor / Win64 构建成功，无新增编译警告。
- RepairGun 与两把新枪的 Weapon/Bullet/Cue/AnimBP/CameraShake Blueprint 均在 Unreal 冷启动命令会话内编译保存并重新加载通过。
- `TheManTest.Player.Weapons.ThreeWeaponBaseline`：Success；核对玩法基线、模型、枪口/命中 VFX、Cue Tag、贴花与独立资产引用。
- `TheManTest.Player.Weapons.ThreeWeaponPIESwitch`：Success；PIE 中按 RepairGun → ElectricGun → ExplosionGun 切换，每把枪均可见，且独立开火蒙太奇可在实时 Arms AnimInstance 启动。
- `TheManTest.Player.CombatHUD.AmmoLifecycle`：Success；既有 RepairGun、GAS 与 HUD 行为未回归。
- 冷启动 Asset Registry 的两个 `GameplayCueName` 与正式 Tag 精确匹配；两把新枪对 RepairGun/供应商目录的定向依赖为 0；范围内 Redirector 为 0；供应商路径和旧角色 Actions 路径在 Asset Registry 及磁盘均不存在。

## 剩余验收

- 自动化使用 NullRHI，尚未验证最终渲染观感。由用户在带渲染窗口的 PIE 中确认三把枪模型握持位置、枪口 VFX、爆炸枪命中粒子以及两种贴花尺寸；确认后可将 FEAT-080 归档为 done。

## 2026-09-03 子弹、材质与命中特效完善

- 用户最新要求覆盖了早先的“子弹无 Mesh”方案：电击枪和爆炸枪均改为拥有独立可见弹体。
- 通过 BlenderMCP 在外部专用工程 `D:\Blender Projects\TheManTestWeaponProjectiles\TheManTestWeaponProjectiles.blend` 制作两个低模弹体，并导出 `SM_ElectricGun_Projectile.fbx`（约 842 面，26.65×8.55×8.55 cm）与 `SM_ExplosionGun_Projectile.fbx`（约 1156 面，21.8×10.33×10.33 cm）。TheManTest 只接收最终 FBX，不包含 Blender 工作文件。
- 为两类弹体生成独立无缝表面纹理，并在各武器目录创建参数化主材质与三组材质实例。电击弹使用深蓝金属、青色导体和紫青发光核心；爆破弹使用黑化金属、黄铜结构和橙色发光核心。枪体材质同步使用相同色彩语言调校，仍沿用原枪体主材质和溶解能力。
- `BP_ElectricGunBullet`、`BP_ExplosionGunBullet` 从 `ARepairGunBullet` 改为直接继承 `ABulletBase`，保留通用伤害、命中 Cue 与销毁流程，但不再误继承 RepairGun 专属泡泡膨胀、减速和危险区抑制行为。旧复制弹体 Mesh/Material 已经通过 Unreal Editor 删除。
- 重新核对 VFXPack 实现后接入电击枪 Energy Impact 3 与爆炸枪 Physical Impact 3。2026-09-04 用户调整反馈分层：武器 Cue 无论命中环境或角色均播放各自同一个 `ImpactEffect`；敌人额外受击表现只由目标自己的 Character Hit Cue 负责。
- 保持现有架构：`UGA_Shoot → AFirearm.MuzzleEffect` 负责枪口 Niagara；`ABulletBase → GameplayCue → UGCN_ImpactFeedbackBase` 负责武器命中表现。`CharacterImpactEffect` 与角色分支已删除，武器贴花仍只生成在环境表面。
- 验证结果：Development Editor / Win64 构建成功；冷启动资产验证输出 `CODEX_FEAT080_VALIDATE|DONE`；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 为 Success；定向依赖扫描输出 `CODEX_FEAT080_DEP|DONE`，未发现 RepairGun 或供应商目录依赖。
- 当前仅剩带渲染窗口的 PIE 观感验收：弹体尺寸/朝向、飞行可读性、枪体材质、枪口 Niagara、统一武器命中 Niagara 与环境贴花尺寸。

## 2026-09-04 Phantom 静止测试 AI

- 为爆炸弹命中与范围逻辑调试创建 Phantom 专属测试行为树 `/Game/Enemy/Humanoid/Phantom/AI/BT_Phantom_TestIdle`，结构为 `Root -> Sequence -> Wait`，Wait 固定为 86400 秒；行为树不包含 MoveTo、攻击或搜索节点。
- 从公共人形 AI Controller 派生资产 `/Game/Enemy/Humanoid/Phantom/AI/BP_Phantom_TestIdleAIController`，其 `BehaviorTree` 指向上述静止树；`BP_Phantom.AIControllerClass` 已切换到该专用测试 Controller，不影响其他人形敌人。
- 为保证 Phantom 即使感知玩家、受击进入 Aim 或收到巡逻配置也不发生位移，`BP_Phantom` 的 `PatrolWalkSpeed`、`CombatWalkSpeed`、`TurnWalkSpeed`、`SearchRushSpeed` 与 CharacterMovement `MaxWalkSpeed` 均临时设为 0。
- Blueprint 在 UE 编辑器内编译保存成功；行为树结构回读为 1 个 Sequence + 1 个 Wait，运行时 `BehaviorTreeComponent` 为 active/running。编辑器重启后的命令行冷加载再次确认测试树、Controller 引用及全部 0 速度配置已持久化。
- NullRHI PIE 中生成 `Codex_PhantomIdleProbe`，确认使用 `BP_Phantom_TestIdleAIController_C`；连续 5 秒位置保持 `(0, 0, 90.15)`、速度保持 `(0, 0, 0)`。测试 Actor 仅存在于 PIE，停止 PIE 后未保存进 TestMap。
- 这是 FEAT-080 的临时命中测试支架；恢复正式 Phantom AI 时需把 `BP_Phantom.AIControllerClass` 改回 `/Game/Enemy/Humanoid/_Shared/AI/BP_HumanoidAIController_C`，并恢复速度 `150/300/50/600` 与 CharacterMovement `MaxWalkSpeed=600`。

## 2026-09-04 EnemyBase 简易血条与临时视角后坐开关

- `AEnemyBase` 新增屏幕空间 `UWidgetComponent`，默认位于角色根节点上方 120cm、尺寸 180×18；原生 `UEnemyHealthBarWidgetBase` 绘制黑色边框、暗红底和亮红填充。所有 EnemyBase 子类（包括 Phantom）自动继承。
- BeginPlay 在初始 GE 后绑定敌人自身 ASC 的 Health/MaxHealth 委托；首次显示与后续受伤均即时刷新，死亡销毁前隐藏。
- `AFirearm` 新增 `bEnableViewRecoil`，公共默认暂时为 false；`UGA_Shoot` 保留 `FireCameraShake` 播放，只跳过改变 Controller Rotation 的 `AddRecoil`。Pitch/Yaw/Damping 原配置完整保留，恢复时把开关改回 true。
- Development Editor / Win64 构建成功。`TheManTest.Enemy.Shared.EnemyBaseHealthBar` 在 NullRHI PIE 中直接生成 `BP_Phantom`，确认继承的屏幕空间组件和原生 Widget 已初始化，并验证 Health 从 100 改为 75 后界面立即同步；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 确认三把枪视角后坐关闭且 CameraShake 资产仍配置，两项测试均为 Success。
- `ABulletBase` 构造函数通过共享资产路径默认加载 `GE_BulletDamage` 作为 HitEffectClass；之后新建普通伤害子弹只需填写 Damage，仍允许特殊子弹覆盖或清空。Development Editor 构建及冷启动 `ThreeWeaponBaseline` 默认类断言为 Success。
- 按用户确认移除 `UGCN_ImpactFeedbackBase.CharacterImpactEffect` 与 Character Niagara 选择分支；电击枪/爆炸枪现在对所有目标分别统一使用 Energy Impact 3/Physical Impact 3，敌人额外反馈交给自身 Character Hit Cue。两个 `NS_*_EnemyImpact` 和 12 个确认无引用的专属材质/纹理已通过 Unreal 资产接口删除；被枪口或环境命中引用的依赖保留。两项 Cue 编译保存、冷启动 `ThreeWeaponBaseline` 与删除资产断言均为 Success。

## 2026-09-04 暗场 VFX 测试地图

- 新建独立地图 `/Game/Maps/VFXTestMap`，不修改既有 `TestMap`。地图使用 22m×14m 的封闭测试房，包含地面、四面墙、天花板和两块不同尺寸的环境命中靶面。
- 新建地图专属哑光材质 `/Game/Maps/VFXTest/Materials/M_VFXTest_Dark`，基础色为深蓝灰、粗糙度 0.92；场景使用 850 强度冷蓝主光与 260 强度暖橙侧光。
- Unbound PostProcess 固定使用 Manual Exposure，Exposure Bias=-1.6、Bloom=1.15、Motion Blur=0，避免自动曝光把暗场提亮，同时让枪口和命中发光更清晰。
- `VFXTest_PlayerStart` 朝向正前方的 `VFXTest_Phantom`；Phantom 沿用专属静止 AI、0 移速和 EnemyBase 通用血条。地图 GameMode 为 `BP_TheManGamemodeBase_C`。
- 冷启动校验确认 13 个预期 Actor、Phantom 位置 `(250,0,96)`、MaxWalkSpeed=0、1 个继承血条组件、正确 GameMode 与 Manual Exposure；MapCheck 为 0 Error / 0 Warning。
- 实际 `-game` 启动确认地图进入 Play、默认玩家 Pawn 与 RepairGun 成功生成；渲染截帧确认深灰房间、冷暖分区、Phantom 和环境靶面均清晰可见。截图保存在 `Saved/Screenshots/WindowsEditor/ScreenShot00002.png`。

## 2026-09-04 电击枪 Laser VFX 替换与地图归档

- 按用户要求从外部资源项目迁入 `/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/VFX/Presets/Muzzle/NE_VFX_Muzzle_Laser_Burst_2` 与 `/Game/VFX_SciFi_Muzzle_And_Impact_Pack_1/VFX/Presets/Impacts/NE_VFX_Projectile_Impact_Laser_2`，连同依赖共 51 个包；未迁移源武器蓝图、角色或动画。
- 两个 Niagara System 在目标项目内分别语义化命名为 `/Game/Weapons/ElectricGun/Effects/Muzzle/Systems/NS_ElectricGun_LaserMuzzle` 与 `/Game/Weapons/ElectricGun/Effects/Impact/Systems/NS_ElectricGun_LaserImpact`。`BP_ElectricGun.MuzzleEffect` 和 `GC_Weapon_ElectricGun_Impact.ImpactEffect` 已改为对应新系统。
- 依赖按 ElectricGun 所有权合并/重命名到 `/Game/Weapons/ElectricGun/Effects`；冷启动递归检查覆盖 51 个依赖包，所有 `/Game` 依赖均位于 ElectricGun 目录。迁移期 40 个 Redirector 通过 UE 5.7 `ResavePackages -FixupRedirects` 删除，供应商路径为空。
- 删除被替换的 `NS_ElectricGun_Muzzle`、`NS_ElectricGun_Impact` 及 5 个确认无外部引用的旧专属效果依赖；保留紫色环境贴花、命中音效、CameraShake 与所有仍被新系统或其他电击枪资产引用的资源。
- 将测试地图从 `/Game/Maps/VFXTestMap` 整理到 `/Game/Maps/VFXTest/VFXTestMap`，与 `/Game/Maps/VFXTest/Materials/M_VFXTest_Dark` 同目录归档。冷启动打开新地图确认 13 个预置 Actor、`VFXTest_Phantom` 与 `VFXTest_PlayerStart` 均保留，旧根目录地图不存在。
- `CombatHUDTests.cpp` 的 ThreeWeaponBaseline 硬编码路径同步更新为新 Laser 系统；Blueprint/Cue 已在 Unreal 内编译保存。Development Editor / Win64 构建成功；冷启动校验输出 `CODEX_ELECTRIC_LASER_VALIDATE|DONE|dependencies=51|actors=13`；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 为 Success。

## 2026-09-04 两枪模型互换与 Laser 方形面片修复

- 按用户要求互换电击枪与爆炸枪的主模型和 Outline。最终仍保留 owner-local 语义路径：`SM_ElectricGun` 现承载原 Ballistics Rifle 02 几何体（LOD0 10272 顶点），`SM_ExplosionGun` 现承载原 SMG 02 几何体（LOD0 8706 顶点），未建立跨武器目录引用。
- 与几何体绑定的 StaticMesh 相对位置及 `MuzzleLocalTransform` 同步交换；电击枪使用原步枪握持/枪口位置，爆炸枪使用原 SMG 握持/枪口位置。枪体材质没有跟随来源交叉引用，仍分别使用 `MI_ElectricGun` 青蓝主题与 `MI_ExplosionGun` 橙色主题。
- 排查原工程和目标工程的 Laser 材质后确认：主材质均为 Translucent/Surface/TwoSided，问题不是 Niagara 加载或 BlendMode；目标工程有 11 个材质实例的 `Main_Texture` 参数为空，而源工程对应参数均有效，导致白色默认纹理把 Niagara Sprite 卡片显示成明显方块。
- 已恢复 LensFlare 1 项、Lightning 5 项、MuzzleFlash 1 项、Smoke 2 项、Fire 1 项与 Rocks 1 项，共 11 个 owner-local 贴图引用。冷启动精确回读 11/11 材质参数、两把枪主模型/Outline/材质/握持/枪口位置及两个 Laser System 均通过，临时交换目录在 Asset Registry 与磁盘均不存在。
- Blueprint 在 UE 内重新编译保存；`TheManTest.Player.Weapons.ThreeWeaponBaseline` 与 `TheManTest.Player.Weapons.ThreeWeaponPIESwitch` 在 NullRHI 和真实 D3D 渲染设备下均为 Success，未出现 Material、Niagara、Shader 或 D3D 错误；定向依赖扫描为 DONE。写入前的地图归档和 Laser 初次迁移结果已封存于 WIP checkpoint `6549004`；本轮结果等待用户明确要求后再更新 Git。

## 2026-09-04 电击枪开火点光增强

- 以 `D:\ROG\Videos\EV录屏\20260904_133033.mp4` 为视觉验收基准逐帧检查：原版青绿色枪体反光从约 6.083s 开始，最强位于 6.100–6.117s，约 3–5 帧后衰减；表现核心是枪身瞬时变为高亮青绿色，而不是整间环境持续变绿。源武器使用同一 Laser Burst 2；`Weapon_Idle_Particle` 不是本次开火反光来源。
- `AFirearm` 的默认关闭 PointLight 增加 `SourceRadius` 与枪口局部 `LocalOffset`，组件对齐源设置：Unitless、Inverse Square、FalloffExponent=8、Cast Shadows、Affect Translucent Lighting。颜色修正为源资产真实线性值 `(0.075319,1,0.652928)`；定时器改为 0.1 秒线性淡出，快速连射重置计时，`Unequip` 立即清灯。
- 两枪模型互换后，电击枪当前 `MuzzleLocalTransform` 坐标轴与源展示蓝图不同，不能直接复用源偏移。最终把枪口位置前移到源枪管位置 `(0.000751,67.696307,6.434297)`，并用当前坐标系反算灯位偏移 `(1.480382,-6.734961,-15.577805)`；Niagara 保持 CameraForward 的 0°附加旋转，90°会把方向性光束错误横扫屏幕。
- `BP_ElectricGun` 最终设置 `MuzzleEffectScale=(1,1,1)`、点光 Intensity=1800、AttenuationRadius=200、SourceRadius=60、Duration=0.1s。源 UE 5.1 的 600 在当前 UE 5.7/枪体材质/曝光链下实际画面只有参考视频约三分之一亮度，因此实例强度按视觉标准补偿；RepairGun 与 ExplosionGun 继续继承默认关闭。
- `/Game/Maps/VFXTest/VFXTestMap` 的 Bloom 临时从 1.15 调为 4.0、Threshold=0.5，以对齐源 MainScene 的审核环境；不影响其他地图。新增 `ElectricMuzzleVisualCapture`，预热 Niagara 后走真实 `PrimaryFire` 并直接读取 PIE 游戏视口，证据截图为 `Saved/Screenshots/WindowsEditor/TMT_ElectricMuzzle_VideoStandard.png`，截帧时点光强度 1500。
- Development Editor / Win64 构建成功。NullRHI `ThreeWeaponBaseline` 与 `ThreeWeaponPIESwitch` 2/2 Success；D3D12/SM6 `ElectricMuzzleVisualCapture` Success。D3D 日志仍仅报告项目既有 `M_UE4Man_Body` 缺失输入纹理与 AimIK 警告，与本轮武器改动无关。

## 2026-09-04 枪口 VFX 与点光统一尺寸倍率

- `MuzzleEffectScale` 现在是每把枪统一的枪口尺寸入口。`UGA_Shoot` 创建 Niagara Component 后先写入 float `User.MuzzleScale`，再激活系统，避免首帧沿用默认倍率。
- `/Game/Weapons/ElectricGun/Effects/Muzzle/Systems/NS_ElectricGun_LaserMuzzle` 已暴露 `User.MuzzleScale`，16 个 emitter 均在 Particle Spawn 阶段通过 Uniform `ScaleSpriteSize` 模块缩放 SpriteSize；倍率 1 保持源外观，倍率 3 的实际 D3D 截图确认闪光、光晕与 Sprite 同步增大。
- 点光尺寸使用同一个倍率：`AttenuationRadius` 与 `SourceRadius` 乘以 `MuzzleEffectScale` 最大绝对轴，Intensity、Color、Duration 与 LocalOffset 不变。蓝图建议 XYZ 填相同值；`BP_ElectricGun` 默认已恢复 `(1,1,1)`。
- Development Editor / Win64 构建成功。NullRHI `ThreeWeaponBaseline` 与 `ThreeWeaponPIESwitch` 2/2 Success；D3D12/SM6 `ElectricMuzzleVisualCapture` 在 1x 与 3x 均为 Success。1x 实测点光半径为 200/60，3x 为 600/180；证据图为 `Saved/Codex/MuzzleSpawnModuleScale_1.png`、`Saved/Codex/MuzzleSpawnModuleScale_3.png` 与最终 `Saved/Codex/MuzzleScaleFinal_1.png`。
