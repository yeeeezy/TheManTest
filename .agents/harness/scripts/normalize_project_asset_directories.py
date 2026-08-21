import unreal


MOVES = [
    # Character base infrastructure.
    ("/Game/Characters/CharacterBase/DataAsset/DA_BaseCharacterAttributes", "/Game/Characters/CharacterBase/Data/DA_BaseCharacterAttributes"),
    ("/Game/Characters/CharacterBase/GAS/GameplayAbility/BGA_Shoot", "/Game/Characters/CharacterBase/GAS/Abilities/BGA_Shoot"),
    ("/Game/Characters/CharacterBase/GAS/GameplayEffect/GE_CharacterBaseBase_Init", "/Game/Characters/CharacterBase/GAS/Effects/GE_CharacterBase_Init"),

    # MaintenanceWorker animation ownership.
    ("/Game/Characters/CharacterBase/Animations/BlendSpaces/Body/BS_RunWalk_MaintenanceWorker", "/Game/Characters/MaintenanceWorker/Animations/Body/Locomotion/BS_RunWalk_MaintenanceWorker"),
    ("/Game/Characters/CharacterBase/Animations/BlendSpaces/FirstPerson/BS_MW_FP_WalkRun", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/BS_MW_FP_WalkRun"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Actions/AM_VFXPack_FP_RecoilLarge", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Actions/AM_MaintenanceWorker_FP_RecoilLarge"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Actions/AS_VFXPack_FP_Fire", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Actions/AS_MaintenanceWorker_FP_Fire"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_Idle", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_Idle"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_JumpEnd", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_JumpEnd"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_JumpLoop", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_JumpLoop"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_JumpStart", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_JumpStart"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_Run", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_Run"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/AS_VFXPack_FP_Still", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/AS_MaintenanceWorker_FP_Still"),
    ("/Game/Characters/CharacterBase/Animations/FirstPerson/Locomotion/BS_VFXPack_FP_WalkRun", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion/BS_MaintenanceWorker_FP_WalkRun"),
]


DIRECTORY_MOVES = [
    ("/Game/Characters/CharacterBase/Animations/Sequences/Body", "/Game/Characters/MaintenanceWorker/Animations/Body/Locomotion"),
    ("/Game/Characters/CharacterBase/Animations/Sequences/FirstPerson", "/Game/Characters/MaintenanceWorker/Animations/FirstPerson/Locomotion"),
    ("/Game/Characters/CharacterBase/Body/Mesh", "/Game/Characters/MaintenanceWorker/Body/Meshes"),
    ("/Game/Characters/CharacterBase/Body/Materials", "/Game/Characters/MaintenanceWorker/Body/Materials"),
    ("/Game/Characters/CharacterBase/FirstPerson/Mesh", "/Game/Characters/MaintenanceWorker/FirstPerson/Meshes"),
    ("/Game/Characters/CharacterBase/FirstPerson/Materials", "/Game/Characters/MaintenanceWorker/FirstPerson/Materials"),
    ("/Game/Characters/CharacterBase/FirstPerson/Textures", "/Game/Characters/MaintenanceWorker/FirstPerson/Textures"),
    ("/Game/Characters/CharacterBase/FirstPerson/Legacy/Mesh", "/Game/Characters/MaintenanceWorker/FirstPerson/Meshes"),
    ("/Game/Characters/CharacterBase/FirstPerson/Legacy/Materials", "/Game/Characters/MaintenanceWorker/FirstPerson/Materials"),
    ("/Game/Characters/MaintenanceWorker/Meshes", "/Game/Characters/MaintenanceWorker/Body/Meshes"),
    ("/Game/Characters/MaintenanceWorker/Materials", "/Game/Characters/MaintenanceWorker/Body/Materials"),
    ("/Game/Characters/MaintenanceWorker/Textures", "/Game/Characters/MaintenanceWorker/Body/Textures"),
    ("/Game/Characters/MaintenanceWorker/DataAsset", "/Game/Characters/MaintenanceWorker/Data"),
    ("/Game/Characters/Infiltrator/DataAsset", "/Game/Characters/Infiltrator/Data"),
    ("/Game/Characters/Infiltrator/GAS/GameplayAbility", "/Game/Characters/Infiltrator/GAS/Abilities"),
    ("/Game/Characters/Infiltrator/Material", "/Game/Characters/Infiltrator/Materials"),
    ("/Game/Characters/TheExecutive/DataAsset", "/Game/Characters/TheExecutive/Data"),

    ("/Game/Actors/Interable/InteractableBase", "/Game/Actors/InteractableBase"),
    ("/Game/Actors/InteractableBase/Material", "/Game/Actors/InteractableBase/Materials"),
    ("/Game/Actors/InteractableBase/Mesh", "/Game/Actors/InteractableBase/Meshes"),

    ("/Game/Weapons/RepairGun/Animation/BlendSpace", "/Game/Weapons/RepairGun/Animations/FirstPerson/Locomotion"),
    ("/Game/Weapons/RepairGun/Animation/Montage", "/Game/Weapons/RepairGun/Animations/FirstPerson/Actions"),
    ("/Game/Weapons/RepairGun/Material", "/Game/Weapons/RepairGun/Materials"),
    ("/Game/Weapons/RepairGun/Mesh", "/Game/Weapons/RepairGun/Meshes"),
    ("/Game/Weapons/TestGun/Animation", "/Game/Weapons/TestGun/Animations"),
    ("/Game/Weapons/TestGun/Material", "/Game/Weapons/TestGun/Materials"),
    ("/Game/Weapons/TestGun/Mesh", "/Game/Weapons/TestGun/Meshes"),
    ("/Game/Weapons/_Shared/Animations/Interface", "/Game/Weapons/_Shared/Animations/Interfaces"),
    ("/Game/Weapons/_Shared/Animations/Logic", "/Game/Weapons/_Shared/Animations/Templates"),
    ("/Game/Weapons/_Shared/GAS/GameplayEffect", "/Game/Weapons/_Shared/GAS/Effects"),
    ("/Game/Weapons/_Shared/Material", "/Game/Weapons/_Shared/Materials"),
    ("/Game/Weapons/_Shared/Mesh", "/Game/Weapons/_Shared/Meshes"),

    ("/Game/Enemy/Humanoid/Phantom/Animations/BlendSpace", "/Game/Enemy/Humanoid/Phantom/Animations/BlendSpaces"),
    ("/Game/Enemy/Humanoid/Phantom/Animations/Sequence", "/Game/Enemy/Humanoid/Phantom/Animations/Sequences"),
    ("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility", "/Game/Enemy/Humanoid/Phantom/GAS/Abilities"),
    ("/Game/Enemy/Humanoid/Phantom/Mesh", "/Game/Enemy/Humanoid/Phantom/Meshes"),
    ("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpace", "/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpaces"),
    ("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Mesh", "/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes"),
    ("/Game/Enemy/Humanoid/_Shared/Animation", "/Game/Enemy/Humanoid/_Shared/Animations"),
    ("/Game/Enemy/Nightmare/FlyingBug2/Mesh", "/Game/Enemy/Nightmare/FlyingBug2/Meshes"),
    ("/Game/Enemy/_Shared/GAS/GameplayAbility", "/Game/Enemy/_Shared/GAS/Abilities"),
    ("/Game/Enemy/_Shared/GAS/GameplayEffect", "/Game/Enemy/_Shared/GAS/Effects"),
]

COVER_MOVES = [
    ("/Game/Enemy/Humanoid/_Shared/Cover/Mesh/SM_PhantomCover", "/Game/Enemy/Humanoid/_Shared/Cover/Meshes/SM_PhantomCover"),
    ("/Game/Enemy/Humanoid/_Shared/Cover/Mesh/M_Cover_Accent", "/Game/Enemy/Humanoid/_Shared/Cover/Materials/M_Cover_Accent"),
    ("/Game/Enemy/Humanoid/_Shared/Cover/Mesh/M_Cover_Base", "/Game/Enemy/Humanoid/_Shared/Cover/Materials/M_Cover_Base"),
    ("/Game/Enemy/Humanoid/_Shared/Cover/Mesh/M_Cover_Rubber", "/Game/Enemy/Humanoid/_Shared/Cover/Materials/M_Cover_Rubber"),
]


move_map = {}

for source, destination in MOVES + COVER_MOVES:
    if unreal.EditorAssetLibrary.does_asset_exist(source):
        move_map[source] = destination

for source_directory, destination_directory in DIRECTORY_MOVES:
    if not unreal.EditorAssetLibrary.does_directory_exist(source_directory):
        continue
    for object_path in unreal.EditorAssetLibrary.list_assets(
            source_directory, recursive=True, include_folder=False):
        package_path = object_path.split(".", 1)[0]
        relative = package_path[len(source_directory):].lstrip("/")
        move_map[package_path] = f"{destination_directory}/{relative}"

rename_data = []
for source, destination in sorted(move_map.items()):
    if unreal.EditorAssetLibrary.does_asset_exist(destination):
        continue
    asset = unreal.EditorAssetLibrary.load_asset(source)
    if not asset:
        raise RuntimeError(f"Unable to load source asset: {source}")
    destination_path, destination_name = destination.rsplit("/", 1)
    rename_data.append(unreal.AssetRenameData(asset, destination_path, destination_name))

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
if not asset_tools.rename_assets(rename_data):
    raise RuntimeError("AssetTools batch rename failed")
unreal.log_warning(f"NORMALIZE_BATCH_RENAMED count={len(rename_data)}")

for root in ("/Game/Characters", "/Game/Actors", "/Game/Weapons", "/Game/Enemy"):
    unreal.EditorAssetLibrary.save_directory(root, only_if_is_dirty=True, recursive=True)

unreal.log_warning("PROJECT_ASSET_DIRECTORIES_NORMALIZED")
