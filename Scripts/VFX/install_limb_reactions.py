import unreal
E=unreal.EditorAssetLibrary
bp=E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
c=unreal.get_default_object(bp.generated_class()).get_component_by_class(unreal.EnemyHitReactionComponent)
sets=[]
for region,enum in [('Torso','TORSO'),('Head','HEAD'),('LeftArm','LEFT_ARM'),('RightArm','RIGHT_ARM'),('LeftLeg','LEFT_LEG'),('RightLeg','RIGHT_LEG')]:
    row=unreal.EnemyBodyReactionAnimations()
    row.set_editor_property('region',getattr(unreal.EnemyHitRegion,enum))
    for direction in ['Front','Back','Left','Right']:
        a=E.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_Blast_'+region+'_'+direction)
        assert a
        row.set_editor_property(direction.lower(),a)
    sets.append(row)
c.set_editor_property('body_animations',sets)
c.set_editor_property('animation_blend_in',.035)
c.set_editor_property('animation_blend_out',.18)
c.set_editor_property('reaction_mode',unreal.EnemyHitReactionMode.ANIMATION)
assert E.save_loaded_asset(bp,False)
post=E.load_asset('/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction')
unreal.BlueprintEditorLibrary.compile_blueprint(post)
assert E.save_loaded_asset(post,False)
print('LIMB_REACTIONS_INSTALLED',len(sets))
