"""Install finalized rifle reactions and remove the retired enemy Rig asset."""
import unreal
E=unreal.EditorAssetLibrary
OLD='/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/'
NEW='/Game/Enemy/Humanoid/_Shared/Animations/Logic/'
name='ABP_Humanoid_HitReaction'
post=E.load_asset(NEW+name if E.does_asset_exist(NEW+name) else OLD+name)
assert post
assert unreal.TheManAnimationAssetLibrary.install_enemy_reaction_animation_branch(post)
assert E.save_loaded_asset(post,False)
if not E.does_asset_exist(NEW+name):
    assert E.rename_asset(OLD+name,NEW+name)
post=E.load_asset(NEW+name)
unreal.BlueprintEditorLibrary.compile_blueprint(post)
assert E.save_loaded_asset(post,False)
bp=E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
cdo=unreal.get_default_object(bp.generated_class())
cdo.set_editor_property('hit_reaction_post_process',post.generated_class())
c=cdo.get_component_by_class(unreal.EnemyHitReactionComponent)
animations={d:E.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_BlastRifle_'+d) for d in ['Front','Back','Left','Right']}
assert all(animations.values())
sets=[]
for region in ['TORSO','HEAD','LEFT_ARM','RIGHT_ARM','LEFT_LEG','RIGHT_LEG']:
    row=unreal.EnemyBodyReactionAnimations();row.set_editor_property('region',getattr(unreal.EnemyHitRegion,region))
    for d,a in animations.items():row.set_editor_property(d.lower(),a)
    sets.append(row)
c.set_editor_property('body_animations',sets)
for d,a in animations.items():c.set_editor_property(d.lower()+'_animation',a)
c.set_editor_property('heavy_front_animation',animations['Front'])
c.set_editor_property('animation_blend_in',.035)
c.set_editor_property('animation_blend_out',.18)
c.set_editor_property('animation_play_rate',1.)
c.set_editor_property('apply_animation_root_motion',True)
assert E.save_loaded_asset(bp,False)
reg=unreal.AssetRegistryHelpers.get_asset_registry()
rig=OLD+'CR_Humanoid_HitReaction'
if E.does_asset_exist(rig):
    refs=E.find_package_referencers_for_asset(rig,False)
    assert not refs,refs
    assert E.delete_asset(rig)
# The native default and the only concrete override now point to Logic.
if E.does_asset_exist(OLD+name):
    refs=E.find_package_referencers_for_asset(OLD+name,False)
    assert not refs,refs
    assert E.delete_asset(OLD+name)
assert not reg.get_assets_by_path(OLD.rstrip('/'),True)
if E.does_directory_exist(OLD):assert E.delete_directory(OLD)
print('MIXAMO_REACTIONS_INSTALLED 4 directions / 6 region slots; old Rig removed')
