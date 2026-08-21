import unreal

redirectors = [
    "/Game/Characters/Infiltrator/Material/M_Highlight",
    "/Game/Characters/Infiltrator/Material/Mat_Outline_library",
    "/Game/Actors/Interable/InteractableBase/Blueprint/BP_InteractableBase",
]

world = unreal.EditorLoadingAndSavingUtils.load_map("/Game/Maps/TestMap")
if not world:
    raise RuntimeError("Unable to load TestMap for external actor reference fixup")
actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = actor_subsystem.get_all_level_actors()
for actor in actors:
    actor.modify()
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.log_warning(f"NORMALIZE_SAVED_TESTMAP_EXTERNAL_ACTORS count={len(actors)}")

for redirector in redirectors:
    refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(
        redirector, load_assets_to_confirm=True
    )
    if refs:
        raise RuntimeError(f"Redirector still referenced: {redirector} <- {list(refs)}")
    if unreal.EditorAssetLibrary.does_asset_exist(redirector):
        if not unreal.EditorAssetLibrary.delete_asset(redirector):
            raise RuntimeError(f"Unable to delete redirector: {redirector}")
        unreal.log_warning(f"NORMALIZE_DELETED_REDIRECTOR {redirector}")

unreal.log_warning("NORMALIZE_REDIRECTOR_FIX_COMPLETE")
