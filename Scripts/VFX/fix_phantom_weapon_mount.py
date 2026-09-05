"""Restore Rifle_01's authored animated weapon attachment; -MountValidateOnly is read-only."""
import unreal

path = '/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom'
bp = unreal.load_asset(path)
validate_only = '-MountValidateOnly' in unreal.SystemLibrary.get_command_line()
if not validate_only:
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
cdo = unreal.get_default_object(bp.generated_class())
weapon = cdo.get_editor_property('weapon_mesh')
if not validate_only:
    cdo.set_editor_property('weapon_attach_socket', 'hand_r_wepSocket')
    weapon.set_editor_property('relative_scale3d', unreal.Vector(1, 1, 1))
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert unreal.EditorAssetLibrary.save_loaded_asset(bp, False)
    cdo = unreal.get_default_object(bp.generated_class())
    weapon = cdo.get_editor_property('weapon_mesh')
assert str(cdo.get_editor_property('weapon_attach_socket')) == 'hand_r_wepSocket'
scale = weapon.get_editor_property('relative_scale3d')
assert abs(scale.x-1) < .0001 and abs(scale.y-1) < .0001 and abs(scale.z-1) < .0001
mesh = cdo.get_editor_property('mesh')
assert str(mesh.get_socket_bone_name('hand_r_wepSocket')) == 'hand_r_wep'
print('PHANTOM_WEAPON_MOUNT_OK', 'cold-read' if validate_only else 'saved', scale)
