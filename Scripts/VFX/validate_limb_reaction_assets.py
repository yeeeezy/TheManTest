import unreal,json,pathlib,math
OUT=pathlib.Path('D:/Blender Projects/HumanoidHitReactions')
rows=json.loads((OUT/'limb_motion.json').read_text())
for row in rows:
    a=unreal.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/'+row['name'])
    assert a and a.get_editor_property('skeleton').get_path_name().startswith('/Game/Enemy/Humanoid/Phantom/OriginalRifle/')
    assert len(unreal.AnimationLibrary.get_animation_track_names(a))==70
    maxpos=0.;mindot=1.
    for i,frame in enumerate(row['frames']):
        poses=unreal.AnimationLibrary.get_bone_poses_for_time(a,row['bones'],i/row['fps'],False)
        for p,w in zip(poses,frame):
            maxpos=max(maxpos,math.sqrt(sum((v-e)**2 for v,e in zip([p.translation.x,p.translation.y,p.translation.z],w['t']))))
            mindot=min(mindot,abs(sum(v*e for v,e in zip([p.rotation.x,p.rotation.y,p.rotation.z,p.rotation.w],w['q']))))
    assert maxpos<.01 and mindot>.9999,(a.get_name(),maxpos,mindot)
    print('REACTION_COLD_VALID',a.get_name(),'frames',len(row['frames']),'position_error',maxpos,'rotation_dot',mindot)
print('REACTION_EXTERNAL_COLD_OK')

E=unreal.EditorAssetLibrary
bp=E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')
for phase in range(2):
    c=unreal.get_default_object(bp.generated_class()).get_component_by_class(unreal.EnemyHitReactionComponent)
    sets=c.get_editor_property('body_animations');assert len(sets)==6
    names=set()
    for r in sets:
        for field in ['front','back','left','right']:
            a=r.get_editor_property(field);assert a;names.add(a.get_name())
    assert len(names)==24
    assert c.get_editor_property('reaction_mode')==unreal.EnemyHitReactionMode.ANIMATION
    if phase==0:unreal.BlueprintEditorLibrary.compile_blueprint(bp)
reg=unreal.AssetRegistryHelpers.get_asset_registry()
assets=reg.get_assets_by_path('/Game/Enemy/Humanoid/Phantom/Animations/Reactions',True)
for a in assets:
    assert str(a.asset_class_path.asset_name)=='AnimSequence'
    deps=reg.get_dependencies(a.package_name,unreal.AssetRegistryDependencyOptions())
    assert all(str(d).startswith(('/Game/Enemy/Humanoid/Phantom/','/Script/','/Engine/','/ACLPlugin/')) for d in deps),deps
shared=E.load_asset('/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction')
unreal.BlueprintEditorLibrary.compile_blueprint(shared)
for d in reg.get_dependencies('/Game/Enemy/Humanoid/_Shared/Animations/ControlRig/ABP_Humanoid_HitReaction',unreal.AssetRegistryDependencyOptions()):
    assert not str(d).startswith('/Game/Enemy/Humanoid/Phantom/')
print('LIMB_COLD_OK',len(names))
