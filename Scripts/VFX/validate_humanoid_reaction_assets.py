import unreal,math
ROOT='/Game/Enemy/Humanoid/Phantom/Animations/Reactions'
SK='/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes/UE4_Mannequin_Skeleton'
sk=unreal.load_asset(SK)
idle=unreal.load_asset('/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Idle_IP')
names=list(map(str,sk.get_reference_pose().get_bone_names()))
base=unreal.AnimationLibrary.get_bone_poses_for_time(idle,names,0,False)
reg=unreal.AssetRegistryHelpers.get_asset_registry()
assets=reg.get_assets_by_path(ROOT,recursive=True)
assert len(assets)==5,len(assets)
for data in assets:
    a=data.get_asset()
    assert isinstance(a,unreal.AnimSequence),a
    assert a.get_editor_property('skeleton')==sk
    assert set(map(str,unreal.AnimationLibrary.get_animation_track_names(a)))==set(names)
    assert not a.get_editor_property('enable_root_motion')
    for time in [0,a.get_play_length()]:
        poses=unreal.AnimationLibrary.get_bone_poses_for_time(a,names,time,False)
        for n,p,b in zip(names,poses,base):
            assert (p.translation-b.translation).length()<.01,(a.get_name(),n)
            dot=sum(x*y for x,y in zip([p.rotation.x,p.rotation.y,p.rotation.z,p.rotation.w],[b.rotation.x,b.rotation.y,b.rotation.z,b.rotation.w]))
            assert abs(dot)>.9999,(a.get_name(),n)
    deps=[str(d) for d in reg.get_dependencies(data.package_name,unreal.AssetRegistryDependencyOptions(include_soft_package_references=True,include_hard_package_references=True))]
    assert all(not p.startswith('/Game/') or p.startswith('/Game/Enemy/Humanoid/Phantom/OriginalRifle/') for p in deps),deps
    print('REACTION_ASSET_VALID',a.get_name(),'duration',a.get_play_length(),'tracks',len(names),'dependencies',deps)
print('REACTION_ASSETS_OK',len(assets),'endpoints_and_dependencies')
