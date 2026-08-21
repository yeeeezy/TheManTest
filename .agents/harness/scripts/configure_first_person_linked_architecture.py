import unreal

host = unreal.load_asset("/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/ABP_MaintenanceWorker_FirstPerson")
interface = unreal.load_asset("/Game/Weapons/_Shared/Animations/Interfaces/ALI_WeaponAnim")
concrete = unreal.load_asset("/Game/Weapons/RepairGun/Animations/FirstPerson/Logic/ABP_RepairGun_FirstPerson")
template = unreal.load_asset("/Game/Weapons/_Shared/Animations/Templates/TABP_FirstPersonFirearmBase")

if not all((host, template, concrete, interface)):
    raise RuntimeError("Required first-person linked animation assets are missing")

ok = unreal.TheManAnimationAssetLibrary.configure_first_person_firearm_linked_layer(
    host, template, concrete, interface, "WeaponUpperBody")
if not ok:
    raise RuntimeError("ConfigureFirstPersonFirearmLinkedLayer failed")

for asset in (host, template, concrete):
    if not unreal.EditorAssetLibrary.save_loaded_asset(asset, only_if_is_dirty=False):
        raise RuntimeError(f"Failed to save {asset.get_path_name()}")

unreal.log_warning("FIRST_PERSON_LINKED_ARCHITECTURE_CONFIGURED")
