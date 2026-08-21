import unreal

registry = unreal.AssetRegistryHelpers.get_asset_registry()
targets = [
    "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Logic/ABP_MaintenanceWorker_FirstPerson",
    "/Game/Characters/MaintenanceWorker/Animations/Body/Logic/ABP_MaintenanceWorker_Body",
    "/Game/Weapons/_Shared/Animations/Interfaces/ALI_WeaponAnim",
    "/Game/Weapons/_Shared/Animations/Templates/TABP_FirstPersonFirearmBase",
    "/Game/Weapons/RepairGun/Animations/FirstPerson/Logic/ABP_RepairGun_FirstPerson",
]

for target in targets:
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(target, load_assets_to_confirm=False)
    data = registry.get_asset_by_object_path(unreal.Name(f"{target}.{target.rsplit('/', 1)[-1]}"))
    unreal.log_warning(f"NORMALIZE_AUDIT target={target} valid={data.is_valid()} refs={list(refs)}")

animation_count = 0
missing_skeleton = []
for data in registry.get_assets_by_path(unreal.Name("/Game/Characters/MaintenanceWorker/Animations"), recursive=True):
    if str(data.asset_class_path.asset_name) != "AnimSequence":
        continue
    animation_count += 1
    asset = data.get_asset()
    if not asset or not asset.get_editor_property("skeleton"):
        missing_skeleton.append(str(data.package_name))
unreal.log_warning(f"NORMALIZE_ANIMATIONS count={animation_count} missing_skeleton={missing_skeleton}")

for root in ("/Game/Characters", "/Game/Actors", "/Game/Weapons", "/Game/Enemy"):
    redirectors = []
    for data in registry.get_assets_by_path(unreal.Name(root), recursive=True):
        if str(data.asset_class_path.asset_name) == "ObjectRedirector":
            redirectors.append(str(data.package_name))
    unreal.log_warning(f"NORMALIZE_REDIRECTORS root={root} count={len(redirectors)} assets={redirectors}")
    for redirector in sorted(set(redirectors)):
        refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(redirector, load_assets_to_confirm=False)
        unreal.log_warning(f"NORMALIZE_REDIRECTOR_REFS asset={redirector} refs={list(refs)}")

unreal.log_warning("NORMALIZE_AUDIT_COMPLETE")
