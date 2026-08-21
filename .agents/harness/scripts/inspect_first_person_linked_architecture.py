import unreal

paths = [
    "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/ABP_MaintenanceWorker_FirstPerson",
    "/Game/Weapons/_Shared/Animations/Interfaces/ALI_WeaponAnim",
    "/Game/Weapons/_Shared/Animations/Templates/TABP_FirstPersonFirearmBase",
    "/Game/Weapons/RepairGun/Animations/FirstPerson/Logic/ABP_RepairGun_FirstPerson",
]

for path in paths:
    asset = unreal.load_asset(path)
    unreal.log_warning(f"ARCH_ASSET {path} class={asset.get_class().get_name() if asset else 'NONE'}")
    if not asset:
        continue
    for prop in ("parent_class", "target_skeleton", "is_template"):
        try:
            unreal.log_warning(f"  {prop}={asset.get_editor_property(prop)}")
        except Exception as exc:
            unreal.log_warning(f"  {prop}=UNAVAILABLE {exc}")
    try:
        unreal.log_warning(f"  graphs={[g.get_name() for g in asset.get_editor_property('function_graphs')]}")
    except Exception as exc:
        unreal.log_warning(f"  graphs=UNAVAILABLE {exc}")
