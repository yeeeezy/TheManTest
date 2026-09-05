import unreal
E=unreal.EditorAssetLibrary
root='/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/'
bp=E.load_asset(root+'ABP_Humanoid_HitReaction')
assert bp.get_editor_property('target_skeleton') is None
# Compiling again must preserve both graph branches and the concrete animation settings.
enemy=E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
unreal.BlueprintEditorLibrary.compile_blueprint(enemy)
cdo=unreal.get_default_object(enemy.generated_class())
r=cdo.get_editor_property('explosion_hit_reaction')
assert r and r.get_editor_property('reaction_mode')==unreal.EnemyHitReactionMode.ANIMATION
for field,suffix in [('front_animation','Front'),('back_animation','Back'),('left_animation','Left'),('right_animation','Right'),('heavy_front_animation','HeavyTwist')]:
    assert r.get_editor_property(field).get_name()=='AS_Humanoid_RifleHit_'+suffix
reg=unreal.AssetRegistryHelpers.get_asset_registry()
for name in ['ABP_Humanoid_HitReaction','CR_Humanoid_HitReaction']:
    deps=[str(x) for x in reg.get_dependencies(root+name,unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)) if str(x).startswith('/Game/')]
    assert all(d.startswith(root) for d in deps),deps
    print('ANIM_REACTION_SHARED_DEPS',name,deps)
assert not [a for a in reg.get_assets_by_path(root,True) if str(a.asset_class_path.asset_name)=='ObjectRedirector']
print('ANIMATION_REACTION_DEFAULTS_OK compile_and_cold_settings')
