import unreal

rig = unreal.load_asset("/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor")
model = rig.get_model()
for link in model.get_links():
    source = link.get_source_pin().get_pin_path()
    target = link.get_target_pin().get_pin_path()
    if any(token in source or token in target for token in ("Locomotor", "GetFoot", "FullBodyIK")):
        print("TMT_RIG_LINK", source, "->", target)

for name in ("Locomotor", "FullBodyIK", "GetFoot0", "GetFoot1", "GetFoot2", "GetFoot3", "GetFoot4", "GetFoot5"):
    node = model.find_node_by_name(name)
    if not node:
        continue
    print("TMT_AUDIT_NODE", name)
    for pin in node.get_pins():
        print("TMT_AUDIT_PIN", pin.get_pin_path(), pin.get_direction(), repr(pin.get_default_value()))
