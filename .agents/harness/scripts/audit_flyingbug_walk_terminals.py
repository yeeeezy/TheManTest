import unreal

anim = unreal.load_asset("/Game/Enemy/Nightmare/FlyingBug2/Animations/Anim_Nightmare_bug2_walk1")
opts = unreal.AnimPoseEvaluationOptions()
print("TMT_POSE_SPACES", [x for x in dir(unreal.AnimPoseSpaces) if not x.startswith("_")])
print("TMT_ANIM_LENGTH", anim.get_play_length())

pose0 = anim.get_anim_pose_at_time(0.0, opts)
names = [str(x) for x in pose0.get_bone_names()]
terminals = []
for name in names:
    if not name.startswith("tent_large"):
        continue
    # Terminal bones in this skeleton end in 3, 4, or 5 depending on chain length.
    if name[-1:].isdigit() and not any(other.startswith(name) and len(other) > len(name) for other in names):
        terminals.append(name)

stats = {name: {"min_z": 1e9, "max_z": -1e9, "min_t": 0.0, "max_abs_x": 0.0, "y_at_min": 0.0} for name in terminals}
length = anim.get_play_length()
for frame in range(41):
    t = length * frame / 40.0
    pose = anim.get_anim_pose_at_time(t, opts)
    for name in terminals:
        tr = pose.get_bone_pose(name, unreal.AnimPoseSpaces.WORLD)
        loc = tr.translation
        s = stats[name]
        if loc.z < s["min_z"]:
            s["min_z"] = loc.z
            s["min_t"] = t
            s["y_at_min"] = loc.y
        s["max_z"] = max(s["max_z"], loc.z)
        s["max_abs_x"] = max(s["max_abs_x"], abs(loc.x))

for name in sorted(terminals):
    s = stats[name]
    print("TMT_WALK_TERMINAL", name, "minZ", round(s["min_z"], 2), "maxZ", round(s["max_z"], 2),
          "minT", round(s["min_t"], 3), "yAtMin", round(s["y_at_min"], 2), "maxAbsX", round(s["max_abs_x"], 2))
