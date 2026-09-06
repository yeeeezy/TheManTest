# 当前进度

## Active Feature

- FEAT-080，in_progress。2026-09-06用户澄清只在延时爆炸造成伤害时播放击退，首次中弹不播放。已修正，Development Editor Win64编译成功，三项PIE验证全部Success，测试编辑器已退出。用户已认可并授权提交、推送到origin/main。

## 当前受击系统

- ExplosionGunBullet仅在延时范围爆炸中比较Health前后值；所有实际扣血且存活的Enemy请求动画，不再要求是AttachedHitActor。首次中弹仍扣血并附着，但不播放击退。爆炸按各目标结算前的当前朝向和爆心关系判断方向，水平距离近零回退弹道。
- 零伤害、GE未修改Health、墙后、范围外和死亡不触发存活动画。重复伤害正常扣血但不重启播放。伤害数值、范围、遮挡、死亡布娃娃、声音、VFX、附着和子弹时间沿用现有逻辑。
- EnemyHitReactionComponent仍仅配置前后左右4条AS_Humanoid_BlastRifle成品动画，来自用户认可的Continuity版本，位于Phantom/Animations/Reactions，绑定现役70骨。无部位分类或受击Rig。
- 共享ABP_Humanoid_HitReaction位于Humanoid/_Shared/Animations/Logic。ApplyAnimationRootMotion默认开启，非下落时扫掠胶囊消费水平根位移并暂停/恢复Movement，死亡不恢复；下落仅上半身。主AnimBP继续运行。
- 本轮仅修改C++、测试和harness；无动画资产、蓝图、地图、音效或VFX写入。

## 验证

- DelayedOnlyReactionBuild：Development Editor Win64编译成功。
- DelayedOnlyReactionRegression：ExplosionDamageReactions、AttachedLimbReaction、ExplosionRadialDamage三项全部Success（3/3，exit0）。上一轮8项回归证据保留在archive。
- 当前测试覆盖旋转37度目标的4方向范围爆炸、首次和重复直接扣血均无动画、真实100→95无动画→75延时爆炸播放、邻居被波及、零伤害/GE未扣血、遮挡/范围外、致死布娃娃及实际后处理动画实例。

## 会话交接

- 写前检查点5ae54b2保存上一轮11个C++/测试/harness文件。用户已授权将本轮修正、测试和记录提交到origin/main，连同此前23条本地提交一并推送；本轮新提交仅包含9个代码/测试/harness文件，地图/ExternalActors/声音/VFX/电击弹等其他未提交资产留在本地。发布提交和远端同步状态以Git记录为准。
- 用户已明确只需延时爆炸击退；不要再次接入首次中弹触发。
- 本次未要求关机；上一轮关机记录属于已结束会话。
- 历史动画验证与预览详见FEAT-080 archive；Continuity预览位于D:/Blender Projects/HumanoidHitReactions/Humanoid_Mixamo_Rifle_Continuity_Normal.gif。
