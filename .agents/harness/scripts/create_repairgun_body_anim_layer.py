import unreal

SOURCE = "/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_AnimLayer"
TARGET = "/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_BodyAnimLayer"
if unreal.EditorAssetLibrary.does_asset_exist(TARGET):
    unreal.EditorAssetLibrary.delete_asset(TARGET)

body_layer = unreal.EditorAssetLibrary.duplicate_asset(SOURCE, TARGET)
if not body_layer:
    raise RuntimeError("Could not create body animation layer")

fp_idle = unreal.load_asset(
    "/Game/Characters/CharacterBase/Animations/Sequences/FirstPerson/Rifle/AS_Rifle_A_Idle")
fp_run = unreal.load_asset(
    "/Game/Characters/CharacterBase/Animations/Sequences/FirstPerson/Rifle/AS_Rifle_A_Run")
body_idle = unreal.load_asset(
    "/Game/Characters/CharacterBase/Animations/Sequences/Body/RTG_W2_Walk_Aim_F_Loop_IP")
body_run = unreal.load_asset(
    "/Game/Characters/CharacterBase/Animations/Sequences/Body/RTG_W2_Jog_Aim_F_Loop_IP")

unreal.TheManAnimationAssetLibrary.add_animation_asset_override(body_layer, fp_idle, body_idle)
unreal.TheManAnimationAssetLibrary.add_animation_asset_override(body_layer, fp_run, body_run)
unreal.EditorAssetLibrary.save_loaded_asset(body_layer, False)

repair_class = unreal.load_object(
    None, "/Game/Weapons/RepairGun/Blueprint/BP_RepairGun.BP_RepairGun_C")
repair_bp = unreal.load_asset("/Game/Weapons/RepairGun/Blueprint/BP_RepairGun")
body_layer_class = unreal.load_object(
    None, TARGET + ".ABP_RepairGun_BodyAnimLayer_C")
if not repair_class or not body_layer_class:
    raise RuntimeError("Could not load RepairGun/body layer classes")
repair_cdo = unreal.get_default_object(repair_class)
repair_bp.modify()
repair_cdo.modify()
repair_cdo.set_editor_property("body_equipment_anim_layer_class", body_layer_class)
unreal.EditorAssetLibrary.save_loaded_asset(repair_bp, False)
print("TMT_BODY_LAYER_SAVED layer=%s class=%s" % (body_layer.get_path_name(), body_layer_class.get_path_name()))
