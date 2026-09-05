"""Cold, read-only shared humanoid reaction dependency and default checks."""
import unreal,os
E=unreal.EditorAssetLibrary
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(True)
root='/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/'
bp=E.load_asset(root+'ABP_Humanoid_HitReaction');rig=E.load_asset(root+'CR_Humanoid_HitReaction')
assert bp and rig
assert bp.get_editor_property('target_skeleton') is None
assert rig.get_preview_mesh() is None
assert rig.get_editor_property('source_hierarchy_import') is None
assert rig.get_editor_property('source_curve_import') is None
o=unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)
for p in [root+'ABP_Humanoid_HitReaction',root+'CR_Humanoid_HitReaction']:
    deps=[str(x) for x in (r.get_dependencies(p,o) or []) if str(x).startswith('/Game/')]
    assert all(x.startswith(root) for x in deps),deps
    print('SHARED_REACTION_COLD_DEPS',p,deps)
mesh=E.load_asset('/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes/SK_Mannequin')
assert mesh.get_editor_property('post_process_anim_blueprint') is None
phantom=unreal.get_default_object(E.load_blueprint_class('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom'))
reaction=phantom.get_component_by_class(unreal.EnemyHitReactionComponent)
assert reaction
print('SHARED_REACTION_TUNING',reaction.get_editor_property('max_angle_degrees'),reaction.get_editor_property('recovery_duration'),reaction.get_editor_property('follow_delay'),reaction.get_editor_property('leg_compression'))
assert not r.get_assets_by_path('/Game/Enemy/Humanoid/Phantom/Animations/ControlRig',True)
assert not os.path.exists(os.path.join(unreal.Paths.project_content_dir(),'Enemy/Humanoid/Phantom/Animations/ControlRig'))
assert not [a for a in r.get_assets_by_path(root.rstrip('/'),True) if str(a.asset_class_path.asset_name)=='ObjectRedirector']
print('SHARED_REACTION_COLD_OK')
