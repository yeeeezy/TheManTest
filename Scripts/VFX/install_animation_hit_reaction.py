"""Enable authored reactions while retaining the original shared Control Rig branch."""
import unreal
E=unreal.EditorAssetLibrary
root='/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/'
bp=E.load_asset(root+'ABP_Humanoid_HitReaction')
assert unreal.TheManAnimationAssetLibrary.install_enemy_reaction_animation_branch(bp)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert E.save_loaded_asset(bp)
enemy=E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')
unreal.BlueprintEditorLibrary.compile_blueprint(enemy)
cdo=unreal.get_default_object(enemy.generated_class())
reaction=cdo.get_component_by_class(unreal.EnemyHitReactionComponent)
assert reaction
reaction.set_editor_property('reaction_mode',unreal.EnemyHitReactionMode.ANIMATION)
for field,suffix in [('front_animation','Front'),('back_animation','Back'),('left_animation','Left'),('right_animation','Right'),('heavy_front_animation','HeavyTwist')]:
    a=E.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_RifleHit_'+suffix)
    assert a
    reaction.set_editor_property(field,a)
assert E.save_loaded_asset(enemy,False)
print('ANIMATION_REACTION_INSTALLED')
print('POSTPROCESS_API',[x for x in dir(unreal.SkeletalMeshComponent) if 'post_process' in x])
