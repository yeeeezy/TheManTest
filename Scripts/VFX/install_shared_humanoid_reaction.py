"""Explicit shared humanoid reaction migration. Does not save maps or weapon tuning."""
import unreal
E=unreal.EditorAssetLibrary
old='/Game/Enemy/Humanoid/Phantom/Animations/ControlRig/'
new='/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/'
for before,after in [('CR_Phantom_ExplosionReaction','CR_Humanoid_HitReaction'),('ABP_Phantom_ExplosionReaction','ABP_Humanoid_HitReaction')]:
    if not E.does_asset_exist(new+after):
        assert E.rename_asset(old+before,new+after)
rig=E.load_asset(new+'CR_Humanoid_HitReaction')
bp=E.load_asset(new+'ABP_Humanoid_HitReaction')
rig.set_preview_mesh(None)
rig.set_editor_property('source_hierarchy_import',None)
rig.set_editor_property('source_curve_import',None)
c=rig.get_controller()
members={str(v.name) for v in rig.get_member_variables()}
if 'ReactionFrame' not in members:
    rig.add_member_variable('ReactionFrame','/Script/TheManTest.HumanoidReactionFrame',True,False,'')
nodes={n.get_node_path() for n in rig.get_default_model().get_nodes()}
if 'GetReactionFrame' not in nodes:
    assert c.add_variable_node('ReactionFrame','FHumanoidReactionFrame',unreal.load_object(None,'/Script/TheManTest.HumanoidReactionFrame'),True,'',unreal.Vector2D(100,400),'GetReactionFrame')
assert c.add_link('GetReactionFrame.Value','EnemyReaction.ReactionFrame')
rig.recompile_vm()
assert unreal.TheManAnimationAssetLibrary.install_enemy_hit_reaction_rig(bp,rig)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
assert E.save_loaded_asset(rig,False)
assert E.save_loaded_asset(bp,False)
mesh=E.load_asset('/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes/SK_Mannequin')
mesh.set_editor_property('post_process_anim_blueprint',None)
assert E.save_loaded_asset(mesh,False)
unreal.BlueprintEditorLibrary.compile_blueprint(E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom'))
r=unreal.AssetRegistryHelpers.get_asset_registry();r.search_all_assets(True)
o=unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)
for p in [new+'CR_Humanoid_HitReaction',new+'ABP_Humanoid_HitReaction']:
    deps=[str(x) for x in (r.get_dependencies(p,o) or []) if str(x).startswith('/Game/')]
    print('SHARED_REACTION_DEPS',p,deps)
print('SHARED_REACTION_INSTALLED')
