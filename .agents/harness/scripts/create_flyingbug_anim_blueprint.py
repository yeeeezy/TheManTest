import unreal

walk = unreal.load_asset("/Game/Enemy/Nightmare/FlyingBug2/Animations/Anim_Nightmare_bug2_walk1")
rig = unreal.load_asset("/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor")
target = "/Game/Enemy/Nightmare/FlyingBug2/Animations/Logic/ABP_NightmareFlyingBug2_WalkLocomotor"
if unreal.EditorAssetLibrary.does_asset_exist(target):
    if not unreal.EditorAssetLibrary.delete_asset(target):
        raise RuntimeError("Failed to replace existing FlyingBug2 animation blueprint")
anim_bp = unreal.TheManAnimationAssetLibrary.create_control_rig_anim_blueprint(
    "/Game/Enemy/Nightmare/FlyingBug2/Animations/Logic",
    "ABP_NightmareFlyingBug2_WalkLocomotor",
    walk,
    rig,
)
print("TMT_ANIM_BP_CREATED", anim_bp.get_path_name() if anim_bp else "NONE")
if not anim_bp:
    raise RuntimeError("Failed to create FlyingBug2 animation blueprint")
print("TMT_ANIM_BP_SAVED", unreal.EditorAssetLibrary.save_loaded_asset(anim_bp, False))
