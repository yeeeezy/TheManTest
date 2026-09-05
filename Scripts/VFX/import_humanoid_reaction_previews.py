"""Import finished animation-only FBX. No retargeting is performed here."""
import unreal
from pathlib import Path

SOURCE = Path('D:/Blender Projects/HumanoidHitReactions')
DEST = '/Game/Enemy/Humanoid/Phantom/Animations/Reactions'
SKELETON = '/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes/UE4_Mannequin_Skeleton'
NAMES = ['Front', 'Left', 'HeavyTwist', 'Right', 'Back']
sk = unreal.load_asset(SKELETON)
assert sk
before = list(map(str, sk.get_reference_pose().get_bone_names()))
for suffix in NAMES:
    name = 'AS_Humanoid_RifleHit_' + suffix
    assert not unreal.EditorAssetLibrary.does_asset_exist(DEST + '/' + name)
    task = unreal.AssetImportTask()
    task.filename = str(SOURCE / (name + '.fbx'))
    task.destination_path = DEST
    task.destination_name = name
    task.automated = True
    task.save = True
    task.replace_existing = False
    task.factory = unreal.FbxFactory()
    options = unreal.FbxImportUI()
    options.automated_import_should_detect_type = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    options.import_mesh = False
    options.import_animations = True
    options.import_materials = False
    options.import_textures = False
    options.skeleton = sk
    options.anim_sequence_import_data.set_editor_property('use_default_sample_rate', False)
    options.anim_sequence_import_data.set_editor_property('custom_sample_rate', 30)
    task.options = options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    a = unreal.load_asset(DEST + '/' + name)
    assert isinstance(a, unreal.AnimSequence), list(task.imported_object_paths)
    assert a.get_editor_property('skeleton') == sk
    a.set_editor_property('enable_root_motion', False)
    a.set_editor_property('force_root_lock', True)
    assert unreal.EditorAssetLibrary.save_loaded_asset(a)
    print('REACTION_IMPORTED', a.get_path_name(), a.get_play_length())
assert list(map(str, sk.get_reference_pose().get_bone_names())) == before
print('REACTION_IMPORT_OK', len(NAMES), 'existing_skeleton_unchanged')
