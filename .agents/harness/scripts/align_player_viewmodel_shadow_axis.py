import unreal

BP_PATH = "/Game/Characters/MaintenanceWorker/Blueprint/BP_MaintenanceWorker"
bp = unreal.load_asset(BP_PATH)
if not bp:
    raise RuntimeError("Could not load " + BP_PATH)

lib = unreal.TheManAnimationAssetLibrary
unit = unreal.Vector(1.0, 1.0, 1.0)
zero_rot = unreal.Rotator(0.0, 0.0, 0.0)
yaw_minus_90 = unreal.Rotator(0.0, 0.0, -90.0)

results = []


def set_transform(name, location, rotation):
    ok = lib.set_inherited_scene_component_transform(
        bp, name, location, rotation, unit)
    results.append("%s=%s" % (name, ok))
    if not ok:
        raise RuntimeError("Failed to set inherited component transform: " + name)


# The VFXPack arms retain their camera-relative depth/height/rotation.  Move the
# camera by the exact inverse lateral amount so the evaluated arm component origin
# remains on CharacterMesh0's actor-space Y=0 centre line.
set_transform("HeadCamera", unreal.Vector(0.0, -18.852108, 77.0), zero_rot)
set_transform("ViewmodelRoot", unreal.Vector(0.0, 0.0, 0.0), zero_rot)
set_transform(
    "ArmsViewMesh",
    unreal.Vector(-18.107912, 18.852108, -150.007950),
    unreal.Rotator(-1.0, -3.0, -15.0),
)

# CharacterMesh0 is the only complete animated shadow caster.  Legs use the same
# capsule-space origin and authored +Y -> actor +X yaw correction.
set_transform("BodyRoot", unreal.Vector(0.0, 0.0, 0.0), zero_rot)
set_transform("CharacterMesh0", unreal.Vector(0.0, 0.0, -90.0), yaw_minus_90)
set_transform("LegsMesh", unreal.Vector(0.0, 0.0, -90.0), yaw_minus_90)

# Retired duplicate shadow components must also be empty in the Blueprint editor,
# not merely cleared by BeginPlay after entering PIE.
for name in ["ShadowBodyMesh", "ShadowUpperBodyMesh"]:
    ok = lib.set_inherited_skeletal_mesh(bp, name, None)
    results.append("%s.Mesh=None:%s" % (name, ok))
    if not ok:
        raise RuntimeError("Failed to clear inherited shadow mesh: " + name)
    set_transform(name, unreal.Vector(0.0, 0.0, -90.0), yaw_minus_90)

saved_before_compile = unreal.EditorAssetLibrary.save_loaded_asset(bp, False)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
saved = unreal.EditorAssetLibrary.save_asset(BP_PATH, False)
print("TMT_PLAYER_AXIS_WRITE " + " | ".join(results))
print("TMT_PLAYER_AXIS_SAVED before_compile=%s after_compile=%s" % (
    saved_before_compile, saved))
