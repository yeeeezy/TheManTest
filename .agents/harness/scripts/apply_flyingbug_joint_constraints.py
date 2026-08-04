import unreal

asset = "/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor"
rig = unreal.load_asset(asset)
controller = rig.get_controller()
prefixes = (
    "tent_large_forward3_left", "tent_large_forward3_right",
    "tent_large_back2_left", "tent_large_back2_right",
    "tent_large_back_left", "tent_large_back_right",
)
entries = [
    '(Item=(Type=Bone,Name="%s2"),bEnabled=True,bUseStiffness=True,'
    'LinearStiffness=(X=1.0,Y=1.0,Z=1.0),AngularStiffness=(X=0.78,Y=0.78,Z=0.78),'
    'bUseAngularLimit=False,bUsePoleVector=False)' % prefix
    for prefix in prefixes
]
rig.modify()
print("TMT_JOINT_CONSTRAINTS_SET", controller.set_pin_default_value(
    "FullBodyIK.Constraints", "(%s)" % ",".join(entries), True, False, False))
rig.recompile_vm()
print("TMT_JOINT_CONSTRAINTS_SAVED", unreal.EditorAssetLibrary.save_asset(asset, False))
