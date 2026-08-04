import unreal

ROOT = "/Game/Enemy/Nightmare/FlyingBug2"
registry = unreal.AssetRegistryHelpers.get_asset_registry()
for data in registry.get_assets_by_path(ROOT, recursive=True):
    class_name = str(data.asset_class_path.asset_name)
    if class_name in ("AnimSequence", "AnimBlueprint", "Blueprint", "ControlRigBlueprint"):
        print("TMT_ANIM_ASSET", class_name, data.package_name)

bp_class = unreal.load_object(None, ROOT + "/Blueprint/BP_NightmareFlyingBug2.BP_NightmareFlyingBug2_C")
if bp_class:
    cdo = unreal.get_default_object(bp_class)
    roam = cdo.get_editor_property("roam_animation")
    mesh = cdo.get_editor_property("mesh")
    print("TMT_ROAM_ANIMATION", roam.get_path_name() if roam else "NONE")
    print("TMT_ANIM_CLASS", mesh.get_editor_property("anim_class") if mesh else "NONE")
    print("TMT_ANIM_MODE", mesh.get_editor_property("animation_mode") if mesh else "NONE")
