import unreal

mesh = unreal.load_asset('/Game/Enemy/Nightmare/FlyingBug2/Mesh/SK_Nightmare_bug2')
skeleton = unreal.load_asset('/Game/Enemy/Nightmare/FlyingBug2/Mesh/SK_Nightmare_bug2_Skeleton')
print('TMT_MESH_APIS', [n for n in dir(mesh) if 'bone' in n.lower() or 'skeleton' in n.lower() or 'ref' in n.lower()])
print('TMT_SKEL_APIS', [n for n in dir(skeleton) if 'bone' in n.lower() or 'skeleton' in n.lower() or 'ref' in n.lower()])
subsystem = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
print('TMT_SUBSYSTEM_APIS', [n for n in dir(subsystem) if 'bone' in n.lower() or 'skeleton' in n.lower() or 'ref' in n.lower()])
pose = skeleton.get_reference_pose()
print('TMT_POSE_TYPE', type(pose), [n for n in dir(pose) if 'bone' in n.lower() or 'transform' in n.lower() or 'name' in n.lower()])
for name in pose.get_bone_names():
    print('TMT_BONE_NAME', name)

terminal_candidates = [
    'tent_low1_left3', 'tent_low1_right3', 'tent_low2_left4', 'tent_low2_right4',
    'tent_low3_left3', 'tent_low3_right3', 'tent_low4_left4', 'tent_low4_right4',
    'tent_large_back_left5', 'tent_large_back_right5',
    'tent_large_forward_left3', 'tent_large_forward_right3',
    'tent_large_back2_left5', 'tent_large_back2_right5',
    'tent_large_forward2_left5', 'tent_large_forward2_right5',
    'tent_large_forward3_left5', 'tent_large_forward3_right5',
]
for name in terminal_candidates:
    try:
        print('TMT_CANDIDATE', name, pose.get_ref_bone_pose(name))
    except Exception as exc:
        print('TMT_CANDIDATE_ERROR', name, exc)

rig = unreal.load_asset('/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor')
hierarchy = rig.hierarchy
for name in terminal_candidates:
    key = unreal.RigElementKey(type=unreal.RigElementType.BONE, name=name)
    try:
        print('TMT_GLOBAL', name, hierarchy.get_global_transform(key, True))
    except Exception as exc:
        print('TMT_GLOBAL_ERROR', name, exc)
