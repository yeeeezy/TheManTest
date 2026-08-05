import unreal

ASSET = "/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor"
rig = unreal.load_asset(ASSET)
if not rig:
    raise RuntimeError("Control Rig asset failed to load")

def foot(name):
    return (
        '(AnkleBone=(Type=Bone,Name="%s"),CollisionRadius=8.0,'
        'MaxHeelPeel=(X=0.000000,Y=0.000000,Z=50.000000),'
        'StaticLocalOffset=(X=0.000,Y=0.000,Z=0.000))' % name
    )

# This mesh is a six-legged creature, not the tutorial's eight-legged spider.
# Use the six terminal chains whose reference-pose Z is at ground level. The old
# tent_low* choices are head tentacles around Z=150-178 and must never be feet.
groups = [
    (["tent_large_forward3_left5", "tent_large_forward3_right5"], 0.000),
    (["tent_large_back2_left5", "tent_large_back2_right5"], 0.333),
    (["tent_large_back_left5", "tent_large_back_right5"], 0.667),
]
foot_sets = "(%s)" % ",".join(
    "(Feet=(%s),PhaseOffset=%.2f)" % (
        ",".join(foot(name) for name in names), phase
    )
    for names, phase in groups
)

controller = rig.get_controller()
rig.modify()

# The previous experimental setup created eight Get Transform nodes. Remove the
# two surplus nodes before shrinking the FBIK effector array; resizing first can
# leave their links pointing at deleted array pins and corrupt the RigVM graph.
model = rig.get_model()
for stale_node_name in ("GetFoot7", "GetFoot6"):
    stale_node = model.find_node_by_name(stale_node_name)
    if stale_node:
        removed = controller.remove_node(stale_node, True, False)
        print("TMT_STALE_NODE_REMOVED", stale_node_name, removed)

changed = controller.set_pin_default_value(
    "Locomotor.FootSets", foot_sets, True, False, False
)
print("TMT_GAIT_SET", changed)
print("TMT_GAIT_VALUE", rig.get_model().find_pin("Locomotor.FootSets").get_default_value())

# This creature's feet are long pointed tips. Keep the procedural clearance low
# so four or more tips remain visually load-bearing instead of forming a tall,
# alternating biped silhouette.
for pin_path, value in (
    # Locomotor phase speed is cycles/second. Match the authored walk's
    # cadence without asking the five-segment chains to span the source clip's
    # impractical ~190 cm inferred stride. At the runtime roam speed of 120
    # cm/s this evaluates to 1.852 cycles/s (about a 65 cm stride), instead of
    # the previous frantic 3.43 cycles/s / 35 cm stride.
    ("Locomotor.Movement.PhaseSpeedMin", "0.800"),
    ("Locomotor.Movement.PhaseSpeedMax", "2.100"),
    ("Locomotor.Movement.MinimumStepLength", "12.0"),
    ("Locomotor.Stepping.PercentOfStrideInAir", "0.35"),
    ("Locomotor.Stepping.StepHeight", "6.0"),
    ("Locomotor.Stepping.MaxCollisionHeight", "1.0"),
	("Locomotor.Pelvis.BobOffset", "-35.0"),
):
    print("TMT_STEP_SET", pin_path,
          controller.set_pin_default_value(pin_path, value, True, False, False))

effectors = "(%s)" % ",".join(
    '(Item=(Type=Bone,Name="%s"),Position=(X=0.000000,Y=0.000000,Z=0.000000),'
    'PositionAlpha=%.1f,PositionDepth=1000,Rotation=(X=0.000000,Y=0.000000,Z=0.000000,W=1.000000),'
    'RotationAlpha=0.0,RotationDepth=1000,Pull=0.000000)' %
    # The front pair must drive the whole five-joint chain just like the other
    # four legs.  A 0.2 alpha only moved the terminal numerically and was
    # effectively invisible in the final silhouette.
    (name, 1.0)
    for names, _phase in groups for name in names
)
effectors_changed = controller.set_pin_default_value(
    "FullBodyIK.Effectors", effectors, True, False, False
)
print("TMT_EFFECTORS_SET", effectors_changed)
print("TMT_EFFECTORS_VALUE", rig.get_model().find_pin("FullBodyIK.Effectors").get_default_value())

# Preserve each authored Walk chain's bend plane. The legacy Fullbody IK node
# otherwise has completely free angular motion and can flip a multi-segment leg
# through the opposite side of its natural pose when an effector crosses a
# singular configuration. Moderate angular stiffness keeps the source-pose bend
# while leaving enough freedom for Locomotor ground placement.
leg_prefixes = (
    "tent_large_forward3_left", "tent_large_forward3_right",
    "tent_large_back2_left", "tent_large_back2_right",
    "tent_large_back_left", "tent_large_back_right",
)
constraints = []
for prefix in leg_prefixes:
    # Joint 2 is the dominant authored bend in all six five-segment chains.
    # Constraining only this pivot prevents flips without over-constraining the
    # remaining joints or making the deprecated solver excessively expensive.
    constraints.append(
        '(Item=(Type=Bone,Name="%s2"),bEnabled=True,bUseStiffness=True,'
        'LinearStiffness=(X=1.000000,Y=1.000000,Z=1.000000),'
        'AngularStiffness=(X=0.78,Y=0.78,Z=0.78),bUseAngularLimit=False,'
        'bUsePoleVector=False)' % prefix
    )
constraints_value = "(%s)" % ",".join(constraints)
constraints_changed = controller.set_pin_default_value(
    "FullBodyIK.Constraints", constraints_value, True, False, False
)
print("TMT_CONSTRAINTS_SET", constraints_changed)
print("TMT_CONSTRAINTS_COUNT", len(constraints))

rig.recompile_vm()
saved = unreal.EditorAssetLibrary.save_asset(ASSET, False)
print("TMT_GAIT_SAVED", saved)
if not saved:
    saved_packages = unreal.EditorLoadingAndSavingUtils.save_packages([rig.get_outermost()], True)
    print("TMT_GAIT_PACKAGE_SAVED", saved_packages)
