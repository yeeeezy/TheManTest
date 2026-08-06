import unreal

asset = unreal.load_asset(
    "/Game/Characters/MaintenanceWorker/Blueprint/BP_MaintenanceWorker")
if not asset:
    raise RuntimeError("Could not load BP_MaintenanceWorker")
unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([asset])
print("TMT_PLAYER_BLUEPRINT_OPENED")
