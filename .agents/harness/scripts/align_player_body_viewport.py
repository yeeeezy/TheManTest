import unreal

BP_PATH = "/Game/Characters/MaintenanceWorker/Blueprint/BP_MaintenanceWorker"
bp = unreal.load_asset(BP_PATH)
cls = unreal.load_object(None, BP_PATH + ".BP_MaintenanceWorker_C")
if not cls:
    raise RuntimeError("Could not load MaintenanceWorker class")
changed = []
for name in ["CharacterMesh0", "ShadowBodyMesh", "LegsMesh"]:
    ok = unreal.TheManAnimationAssetLibrary.set_inherited_scene_component_rotation(
        # Unreal Python positional order is roll, pitch, yaw. The required
        # correction is the blue Z/Yaw field, not the green Y/Pitch field.
        bp, name, unreal.Rotator(0.0, 0.0, -90.0))
    changed.append("%s:%s" % (name, ok))

print("TMT_BODY_VIEWPORT_ALIGNED " + " | ".join(changed))
print("TMT_BODY_VIEWPORT_SAVED", unreal.EditorAssetLibrary.save_loaded_asset(bp, False))
