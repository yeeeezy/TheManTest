import unreal

moves = [
    ("/Game/Weapons/_Shared/Animations/Logic/TABP_Firearm_UpperBodyBase",
     "/Game/Weapons/_Shared/Animations/Logic/TABP_FirstPersonFirearmBase"),
    ("/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_AnimLayer",
     "/Game/Weapons/RepairGun/Animations/FirstPerson/Logic/ABP_RepairGun_FirstPerson"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Logic/ABP_CharacterBase_FirstPerson",
     "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/ABP_MaintenanceWorker_FirstPerson"),
    ("/Game/Characters/CharacterBase/Animations/Body/Logic/ABP_CharacterBase_Body",
     "/Game/Characters/MaintenanceWorker/Animations/Body/Logic/ABP_MaintenanceWorker_Body"),
    ("/Game/Characters/CharacterBase/Animations/Logic/TABP_BodyLocomotion",
     "/Game/Characters/CharacterBase/Animations/Body/Logic/TABP_CharacterBase_BodyLocomotion"),
]

for source, destination in moves:
    if unreal.EditorAssetLibrary.does_asset_exist(source):
        if not unreal.EditorAssetLibrary.rename_asset(source, destination):
            raise RuntimeError(f"Failed to move {source} -> {destination}")

unused = "/Game/Characters/CharacterBase/Animations/Template/TABP_CharacterBase"
if unreal.EditorAssetLibrary.does_asset_exist(unused):
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(unused, load_assets_to_confirm=True)
    if refs:
        raise RuntimeError(f"Unused template unexpectedly referenced: {refs}")
    if not unreal.EditorAssetLibrary.delete_asset(unused):
        raise RuntimeError(f"Failed to delete {unused}")

unreal.EditorAssetLibrary.save_directory("/Game/Characters", only_if_is_dirty=False, recursive=True)
unreal.EditorAssetLibrary.save_directory("/Game/Weapons", only_if_is_dirty=False, recursive=True)
unreal.log_warning("FIRST_PERSON_ANIMATION_ASSETS_ORGANIZED")
