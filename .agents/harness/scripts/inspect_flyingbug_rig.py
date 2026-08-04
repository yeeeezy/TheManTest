import unreal

ASSET = "/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor"
rig = unreal.load_asset(ASSET)
print("TMT_RIG_CLASS", rig.get_class().get_name() if rig else "NONE")
if not rig:
    raise RuntimeError("Control Rig asset failed to load")

model = rig.get_model()
print("TMT_MODEL", model.get_node_path() if model else "NONE")
print("TMT_RIG_APIS", [name for name in dir(rig) if "hierarch" in name.lower() or "control" in name.lower()])
hierarchy = rig.hierarchy
for key in hierarchy.get_all_keys():
    if key.type == unreal.RigElementType.CONTROL:
        print("TMT_CONTROL", key.name)
def dump_pin(node_name, pin, depth=0):
    default = pin.get_default_value()
    print("TMT_PIN", node_name, depth, pin.get_pin_path(), repr(default), pin.get_cpp_type())
    for sub_pin in pin.get_sub_pins():
        dump_pin(node_name, sub_pin, depth + 1)


for node in model.get_nodes():
    name = node.get_name()
    if name in ("Locomotor", "FullBodyIK") or name.startswith("GetFoot"):
        print("TMT_NODE", name, node.get_node_path())
        for pin in node.get_pins():
            dump_pin(name, pin)

for path in ("Locomotor.Stepping", "Locomotor.Pelvis", "Locomotor.FootSets", "FullBodyIK.Effectors"):
    pin = model.find_pin(path)
    print("TMT_FOCUS", path, repr(pin.get_default_value() if pin else "MISSING"))
