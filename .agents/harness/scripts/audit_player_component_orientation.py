import unreal

CLASS_PATH = "/Game/Characters/MaintenanceWorker/Blueprint/BP_MaintenanceWorker.BP_MaintenanceWorker_C"
cls = unreal.load_object(None, CLASS_PATH)
if not cls:
    raise RuntimeError("Could not load " + CLASS_PATH)

cdo = unreal.get_default_object(cls)
print("TMT_PLAYER_CLASS class=%s" % cls.get_path_name())
components = cdo.get_components_by_class(unreal.SceneComponent)
for component in components:
    name = component.get_name()
    if name not in {
        "CharacterMesh0",
        "HeadCamera",
        "ArmsViewMesh",
        "ShadowBodyMesh",
        "ShadowUpperBodyMesh",
        "LegsMesh",
        "BodyRoot",
        "ViewmodelRoot",
        "Arrow",
    }:
        continue
    transform = component.get_relative_transform()
    parent = component.get_attach_parent()
    mesh = None
    if isinstance(component, unreal.SkeletalMeshComponent):
        mesh = component.get_editor_property("skeletal_mesh_asset")
        anim_class = component.get_editor_property("anim_class")
    else:
        anim_class = None
    print(
        "TMT_ORIENTATION name=%s parent=%s location=%s rotation=%s scale=%s mesh=%s"
        % (
            name,
            parent.get_name() if parent else "None",
            transform.translation,
            transform.rotation.rotator(),
            transform.scale3d,
            mesh.get_path_name() if mesh else "None",
        )
    )
    if anim_class:
        print("TMT_ANIM_CLASS component=%s class=%s" % (name, anim_class.get_path_name()))

by_name = {component.get_name(): component for component in components}
arms = by_name.get("ArmsViewMesh")
body = by_name.get("CharacterMesh0")
camera = by_name.get("HeadCamera")
viewmodel_root = by_name.get("ViewmodelRoot")
legs = by_name.get("LegsMesh")
if arms and body and camera and viewmodel_root and legs:
    # At the BP default orientation the actor right axis is +Y.  Component origins
    # are sufficient here because the requested invariant is absolute lateral
    # placement, while depth and height are intentionally allowed to differ.
    camera_location = camera.get_relative_transform().translation
    viewmodel_location = viewmodel_root.get_relative_transform().translation
    arms_location = arms.get_relative_transform().translation
    body_location = body.get_relative_transform().translation
    arm_lateral = camera_location.y + viewmodel_location.y + arms_location.y
    body_lateral = body_location.y
    print("TMT_AXIS arm_lateral=%.6f body_lateral=%.6f error=%.6f" % (
        arm_lateral, body_lateral, abs(arm_lateral - body_lateral)))
    print("TMT_LEGS_ORIGIN body=%s legs_world_basis=%s body_root=%s" % (
        body_location, legs.get_relative_transform().translation,
        by_name["BodyRoot"].get_relative_transform().translation))

registry = unreal.AssetRegistryHelpers.get_asset_registry()
for bp_path in [
    "/Game/Characters/CharacterBase/Animations/Skeleton/ABP_CharacterBase_Body",
    "/Game/Characters/CharacterBase/Animations/Logic/TABP_BodyLocomotion",
    "/Game/Characters/CharacterBase/Animations/Legacy/VFXPackFirstPerson/ABP_VFXPack_FirstPerson",
]:
    bp = unreal.load_asset(bp_path)
    if bp:
        try:
            parent = bp.get_editor_property("parent_class")
        except Exception as exc:
            parent = "ERROR:%s" % exc
        print("TMT_BP_PARENT asset=%s parent=%s" % (bp_path, parent))
deps = unreal.EditorAssetLibrary.find_package_referencers_for_asset(
    "/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_AnimLayer", False)
print("TMT_LAYER_REFERENCERS %s" % deps)
for dep in registry.get_dependencies(
    "/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_AnimLayer",
    unreal.AssetRegistryDependencyOptions(True, True, True, True)):
    print("TMT_LAYER_DEP %s" % dep)
for dep in registry.get_dependencies(
    "/Game/Weapons/_Shared/Animations/Logic/TABP_Firearm_UpperBodyBase",
    unreal.AssetRegistryDependencyOptions(True, True, True, True)):
    print("TMT_LAYER_TEMPLATE_DEP %s" % dep)
for asset in registry.get_assets_by_path("/Game/Characters/CharacterBase/Animations", True):
    if "AnimBlueprint" in str(asset.asset_class_path) or "AnimSequence" in str(asset.asset_class_path):
        print("TMT_CHARACTER_ANIM asset=%s class=%s" % (asset.package_name, asset.asset_class_path))
