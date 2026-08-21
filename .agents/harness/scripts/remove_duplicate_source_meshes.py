import unreal

duplicates = [
    "/Game/Characters/CharacterBase/Body/Mesh/SKM_UE4Mannequin",
    "/Game/Characters/CharacterBase/FirstPerson/Mesh/SKM_MaintenanceWorker_FirstPersonArms",
    "/Game/Characters/CharacterBase/FirstPerson/Legacy/Mesh/SKM_Mannequin_Arms",
    "/Game/Enemy/Humanoid/Phantom/Mesh/SK_Cyber01",
    "/Game/Enemy/Humanoid/Phantom/OriginalRifle/Mesh/SK_Mannequin",
    "/Game/Enemy/Nightmare/FlyingBug2/Mesh/SK_Nightmare_bug2",
    "/Game/Enemy/Nightmare/FlyingBug2/Mesh/SK_Nightmare_bug2_Skeleton",
    "/Game/Weapons/RepairGun/Mesh/SK_SCFRIFLE",
    "/Game/Weapons/RepairGun/Mesh/SK_SCI_FI_Rifle_NoMag",
    "/Game/Weapons/TestGun/Mesh/SK_SCFP",
]

for source in duplicates:
    if not unreal.EditorAssetLibrary.does_asset_exist(source):
        continue
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(
        source, load_assets_to_confirm=True
    )
    if refs:
        raise RuntimeError(f"Duplicate source still referenced: {source} <- {list(refs)}")
    if not unreal.EditorAssetLibrary.delete_asset(source):
        raise RuntimeError(f"Unable to delete duplicate source: {source}")
    unreal.log_warning(f"NORMALIZE_DELETED_DUPLICATE {source}")

unreal.log_warning("NORMALIZE_DUPLICATE_CLEANUP_COMPLETE")
