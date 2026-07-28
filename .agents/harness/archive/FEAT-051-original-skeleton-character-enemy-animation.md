# [FEAT-051] Original-Skeleton Character and Enemy Animation Blueprints

**Created:** 2026-07-26  
**Status:** in_progress

## Scope

Keep the existing animation architecture and rebuild only the concrete skeleton-bound AnimBP layer after unsatisfactory retargeted assets were removed.

### Player

- Keep `GetMesh()`, `ArmsViewMesh`, and weapon Linked Anim Layers on the same player Skeleton.
- Retargeted animation remains acceptable for lower-body locomotion when its visual result is good enough.
- Keep `TABP_BodyLocomotion`, `ALI_WeaponAnim`, `TABP_Firearm_UpperBodyBase`, and the C++ AnimInstance driver hierarchy; concrete players use skeleton-bound child AnimBPs such as `ABP_MaintenanceWorker`.

### Enemy

- Prefer each animation set's original Skeleton.
- Reuse the existing skeleton-independent Template AnimBP/state-machine logic.
- Create a skeleton-bound child AnimBP per Enemy and provide compatible assets through Asset Override.
- Validate hard-coded bone/socket assumptions per skeleton: `hand_r`, `hand_l`, spine chain, `AimSocket`, weapon grip, and IK targets.

## Ownership Boundary

- The user manually deletes, selects, imports, and assigns concrete animation assets in the Unreal Editor.
- Codex maintains architecture documentation, performs read-only MCP audits, and can guide editor verification step by step after confirmation.

## Initial MCP Audit

- Player core assets initially present: `ABP_BodyLocomotion`, `ALI_WeaponAnim`, `TABP_Firearm_UpperBodyBase`.
- Enemy template/child assets still present: `ABP_HumanoidEnemy`, `ABP_Phantom`.
- Phantom retains its original `SK_Cyber01_Skeleton` and associated animation set.
- FEAT-046's prior rifle BlendSpace is not complete: the actual state machine uses `Idle <-> WalkRun`, while `BS_Rifle_UpperBody_IdleWalkRun` is a 2D BlendSpace with zero samples.

## MaintenanceWorker Rebuild

- The player locomotion template is now `TABP_BodyLocomotion`; `ABP_MaintenanceWorker` is its skeleton-bound child.
- MaintenanceWorker body, lower-body, arms, and temporary animation assets use the arms Skeleton as the single player Skeleton. Minor reference-pose differences are accepted temporarily because the current milestone validates code and AnimBP architecture; final animation assets will be rebuilt later.
- RepairGun uses `/Game/Weapons/RepairGun/Animation/Logic/ABP_RepairGun_AnimLayer` as a weapon-specific Linked Anim Layer.
- `EquipmentAnimClass` was removed from `AEquipmentBase`; weapons can no longer replace the character's complete AnimInstance and must use `EquipmentAnimLayerClass`.
- Player turn-in-place is intentionally disabled for the current architecture pass. `AFPSCharacterBase` now drives `BodyRoot` directly from Actor yaw, and `UFPSCharacterAnimInstance` no longer exposes turn state or curve variables. The user will revisit turning as a separate animation feature later.

## Game Animation Sample Turn Audit

Read-only audit of `D:\Unreal Projects\GameAnimationSample` found that its convincing footwork is a combined Motion Matching system, not a single turn state. `SandboxCharacter_CMC_ABP` uses Pose Search history/trajectory, Motion Matching, `Offset Root Bone`, `Orientation Warping`, and `Foot Placement`. Dedicated databases include `PSD_Relaxed_Stand_TurnInPlace` (45/90/135/180 left/right) and walk/run/sprint Pivot/Spin databases. Moving turns commonly provide separate `Lfoot` and `Rfoot` assets, allowing pose search to select a compatible supporting foot before Foot Placement locks and aligns it. This is research for a future independent turn feature only; FEAT-051 keeps direct Pawn yaw.

## RepairGun Projectile Mobility Bug

After the bullet component sizes were adjusted, `BP_RepairGunBullet.CollisionSphere` was accidentally set to Static while its child `BulletMesh` remained Movable. MCP confirmed the root mobility mismatch, and PIE logs repeatedly reported that `CollisionSphere` must be Movable for `ProjectileMovementComponent`. This made bullets remain at the muzzle and only begin expansion when the moving gun touched them later. The user restored `CollisionSphere` to Movable and confirmed the behavior was fixed; size settings and `MaxExpansionScale=5` were retained.

## First-Person Strafe Orientation Fix

The player body, shadow, and `ArmsViewMesh` remain synchronized through the same locomotion timing and player Skeleton. The strafe issue did not require a second first-person locomotion AnimBP: `TABP_Firearm_UpperBodyBase.WeaponUpperBody` already layered its forward-facing firearm state machine over the incoming locomotion pose from `spine_01` with blend depth 2, but its `Layered Blend per Bone` used local-space rotation blending. Lateral pelvis/root rotation therefore propagated into the weapon upper body and visibly changed the first-person gun direction.

Session90 enabled `Mesh Space Rotation Blend` on that existing layered blend. No graph topology, branch filter, state machine, BlendSpace samples, Skeleton assignment, or locomotion timing was changed. Unreal MCP compiled and saved `TABP_Firearm_UpperBodyBase`, `ABP_RepairGun_AnimLayer`, `TABP_BodyLocomotion`, and `ABP_MaintenanceWorker` successfully.

PIE runtime sampling supplied stronger verification than screenshots: while moving at 250 cm/s, A and D produced identical five-sample `ArmsViewMesh.hand_r` rotations relative to the camera at matching Walk animation phases, even though the full-body `spine_01` rotations differed by strafe direction. Runtime topology also confirmed both `CharacterMesh0` and `ArmsViewMesh` use `ABP_MaintenanceWorker_C`, while `ShadowBodyMesh` and `LegsMesh` retain `CharacterMesh0` as their Leader Pose. Thus the first-person weapon direction is no longer direction-biased and the body/shadow synchronization chain remains intact.

## Centralized Upper-Body Blend

Session91 moved the ownership of upper-body composition out of `TABP_Firearm_UpperBodyBase` and into the main `TABP_BodyLocomotion`. `WeaponUpperBody` now outputs only `SM_FirearmUpperBody`; the firearm template's duplicate `Layered Blend per Bone` was deleted. The main graph owns the single blend between `DefaultSlot` locomotion and the linked weapon pose, followed by `WeaponAimOffset`, `UpperBodySlot`, the highest-priority `FullBodySlot`, and the output pose.

The central blend uses `spine_01`, blend depth 4, weight 1, and Mesh Space Rotation Blend. Mesh-space rotation retains the completed A/D first-person weapon-direction fix for every weapon layer using the interface.

Unreal compiled and saved `TABP_BodyLocomotion`, `TABP_Firearm_UpperBodyBase`, `ABP_RepairGun_AnimLayer`, and `ABP_MaintenanceWorker`. Runtime audit again confirmed that only `ShadowBodyMesh` casts the player shadow, both `ShadowBodyMesh` and `LegsMesh` follow `CharacterMesh0`, and both animated meshes use `ABP_MaintenanceWorker_C`. Subsequent user inspection established that the apparent waist break in the shadow comes from the current full-body model's segmented waist geometry, not from animation blending. A true silhouette fix therefore requires editing or replacing that mesh.

Session92 corrected a regression in the first central graph wiring. AnimGraph pose outputs are single-link: connecting `DefaultSlot.Pose` to `WeaponUpperBody.UpperBodyInPose` after connecting it to the central blend silently removed the `BasePose` link. The character therefore translated at runtime while the original locomotion pose was absent. The firearm layer now remains a pure-pose provider with its unused interface input disconnected, while `DefaultSlot.Pose` feeds the central blend's `BasePose` directly. Four AnimBPs compiled and saved again. During a 250 cm/s W test, five successive `thigh_l` and `calf_l` component-space samples all changed, proving the walk cycle was evaluating; A/D screenshots `CentralBlend_Fixed_A.png` and `CentralBlend_Fixed_D.png` provided visual regression coverage.

## Immediate Initial Character Visibility

Session93 audited the apparent delay when entering the current `TestMap`. The map's `BP_TheManGamemodeBase` synchronously falls back to `BP_MaintenanceWorker` when `SelectedCharacterID` is empty, and PIE already contains that pawn when the play request returns; there is no asynchronous character-class load on this path.

The delay came from `AFPSCharacterBase::BeginPlay`: after synchronous equipment initialization it explicitly hid `GetMesh()`, `ArmsViewMesh`, `ShadowBodyMesh`, `LegsMesh`, and the current equipment actor, then restored them in a next-tick callback. That callback also played the initial Equip Montage. Session93 removed the hide/show lifecycle entirely. All character render components and the equipped weapon now retain their configured visibility from spawn, while `PlayInitialEquipMontage()` remains scheduled for the next tick so animation-instance initialization order stays safe.

Development Editor/Win64 compiled successfully and the final patch loaded through Live Coding. Runtime inspection confirmed `BP_MaintenanceWorker_C_0` as the immediate view target; its first-person arms, shadow body, legs, and RepairGun remain visible (the inherited `CharacterMesh0` is still intentionally OwnerNoSee). PIE visual regression confirmed the arms, RepairGun, and complete shadow after animation evaluation. MCP-driven tests can report a 3 FPS PIE max tick rate while the editor is unfocused, so a forced screenshot before the first world tick is not equivalent to a player-visible frame and was excluded from completion evidence.

## Shadow Leader Bone Refresh Fix

Session94 diagnosed the abnormal shadow animation seen after the initial-visibility cleanup. The Leader Pose topology and AnimClass were still correct (`ShadowBodyMesh -> CharacterMesh0`, both using `ABP_MaintenanceWorker_C`), but runtime inspection showed `CharacterMesh0` using `AlwaysTickPose` despite the C++ constructor assigning `AlwaysTickPoseAndRefreshBones`. `BP_MaintenanceWorker` had retained an older serialized component value that overrode the constructor default. Because `CharacterMesh0` is OwnerNoSee and does not render for the local player, pose evaluation without bone refresh can leave its shadow follower with stale transforms.

`AFPSCharacterBase::BeginPlay` now reapplies `AlwaysTickPoseAndRefreshBones` to `GetMesh()` after Blueprint defaults have been instantiated, guaranteeing that the invisible animation host refreshes the bones consumed by `ShadowBodyMesh` and `LegsMesh`. Development Editor/Win64 and Live Coding both succeeded. During PIE W movement at 250 cm/s, five successive `thigh_l` component-space samples changed and every `CharacterMesh0` sample exactly matched `ShadowBodyMesh`; `ShadowRefresh_Fixed_Move.png` visually confirmed the animated armed shadow.

## Synchronized Equip Montage Timing

Session95 diagnosed why the configured RepairGun Equip Montage worked on initial entry but appeared absent when switching back from TestGun. `SwitchEquipment()` linked the weapon Anim Layer and called `PlayEquipMontage()` in the same frame. Runtime inspection showed the Montage become active at position 0, then disappear at the next animation update because Linked Anim Layer initialization reset the just-started Montage. Initial entry did not fail because its playback was already deferred by one tick. TestGun itself currently has no Equip Montage, so switching from RepairGun into TestGun intentionally has no equip animation.

The switch path now defers playback to the next tick and verifies that rapid scrolling has not changed the current equipment before playing. `AEquipmentBase::PlayEquipMontage()` now starts the same Montage on both independent FPS AnimInstances (`ArmsViewMesh` and `CharacterMesh0`); `ShadowBodyMesh` inherits the latter through Leader Pose. Development Editor/Win64 and Live Coding succeeded. Deterministic paused-PIE frame stepping showed both instances active at 0 and then both at 0.3333334 seconds after the next frame. After completion, W movement remained 250 cm/s, neither Montage remained active, and the body/shadow `thigh_l` transforms matched. Evidence: `EquipMontage_Synced_0333.png` and `EquipMontage_PostMove.png`.

Session96 resolved the remaining discrepancy between runtime playback state and the user's visible result. The RepairGun Montage contained only a `DefaultSlot` track. In `TABP_BodyLocomotion`, that slot feeds the base side of the central upper-body blend, after which `WeaponUpperBody` replaces the pose from `spine_01`; consequently the Montage could be active and advancing while its arm motion was completely masked. The same `AS_Rifle_A_Equip` sequence is now also present on the Montage's `UpperBodySlot`, which is evaluated after the central weapon blend. The saved Montage reports 0.8666667 seconds, two slot tracks, and zero notifies. Paused PIE frame screenshots at 0, 0.333, and 0.666 seconds visibly show the weapon and arms lowering, moving right, and raising again, with the ground shadow performing the matching motion (`EquipUpperSlot_T0.png`, `EquipUpperSlot_T0333.png`, `EquipUpperSlot_T0666.png`).

Session97 investigated the resulting “lower from above, then raise” presentation. Direct `AnimPose` sampling of `AS_Rifle_A_Equip` at 0, 0.05, 0.1, 0.2, 0.333, 0.5, 0.666, and 0.85 seconds confirmed that the source sequence begins in its lower equip pose and progresses toward the held pose; it does not contain a preceding holster motion. The reversal was introduced by the Montage's default 0.25-second Hermite Blend In combined with making the newly linked weapon visible in its idle pose before the Montage had evaluated. The graph therefore visibly blended from held idle down to the source start, then followed the source upward.

The RepairGun Montage now uses zero Blend In. During switching, equipment with an Equip Montage stays hidden through linking and attachment, starts the Montage on the next tick, and becomes visible one additional tick later after the start pose has reached both `ArmsViewMesh` and `CharacterMesh0`; equipment without a Montage still appears immediately. Weak current-equipment checks remain on both callbacks for rapid-scroll safety. Deterministic PIE states were `hidden=true, playing=true, position=0` after playback start and `hidden=false, playing=true, position=0.3333334` when first shown in the 3 FPS automated environment. `EquipRaise_FirstVisible.png` and `EquipRaise_Later.png` show only the upward motion. After completion, W movement remained 250 cm/s and `ShadowBodyMesh` still followed `CharacterMesh0` with matching `thigh_l` transforms.

Session98 removed the remaining one-frame empty-hands flash. The flash was a direct visibility gap: the previous weapon was hidden during `Unequip()`, while the incoming Montage weapon intentionally stayed hidden until its start pose had evaluated; `ArmsViewMesh` remained visible between them. Montage switches now retain the outgoing actor as `PendingVisibleEquipment` in the hand. When the incoming start pose is ready, `FinalizeUnequippedEquipment()` hides or holsters the outgoing actor and the same callback reveals the incoming actor, making the visual swap atomic. A new switch first finalizes any older pending actor, preventing rapid-scroll leftovers.

Paused PIE visibility evidence remained mutually exclusive at every stage: initial RepairGun `(Repair false, Test true)`, TestGun `(Repair true, Test false)`, waiting to re-equip RepairGun `(Repair true, Test false)`, and ready RepairGun `(Repair false, Test true)`, where the booleans are hidden-state values. Four rapid switches ended in the same safe waiting state without two hidden weapons. `AtomicEquipSwap_FirstRepairFrame.png` shows the first RepairGun frame with no empty-hands flash. Development Editor/Win64 and Live Coding succeeded; post-switch W movement remained 250 cm/s and body/shadow `thigh_l` transforms still matched.

Session99 corrected that diagnosis after the user clarified the visible artifact: it was not an empty-hand frame. During the frames before the RepairGun raised from below, the first-person hands and gun briefly appeared in their final held position. Linking the incoming weapon layer immediately changed `ArmsViewMesh` to that layer's held Idle pose; hiding only the incoming weapon actor could not hide the already-visible arms, and retaining the outgoing gun made the flash read as a complete held viewmodel.

For Montage switches, `SwitchEquipment()` now finalizes the outgoing equipment immediately and hides both the incoming equipment actor and `ArmsViewMesh` while the linked layer and Montage start pose are prepared. After playback starts and one animation evaluation has completed, the same callback reveals the incoming weapon and arms. `CharacterMesh0` is never hidden, so the full-body animation and `ShadowBodyMesh` Leader Pose remain continuous. Non-Montage equipment restores the arms immediately, and weak current-equipment checks prevent stale rapid-switch callbacks from revealing the wrong state.

Paused PIE evidence was `ArmsViewMesh=false / RepairGun hidden=true` during preparation, then `ArmsViewMesh=true / RepairGun hidden=false / Montage position=0.3333334` on the first visible automated frame. `EquipViewmodel_FirstVisible.png` records that frame. Four rapid switches ended with RepairGun and arms visible; W movement remained 250 cm/s, and `ShadowBodyMesh` still reported `CharacterMesh0` as its Leader. Development Editor/Win64 and Live Coding both succeeded.

Session100 replaced that visibility workaround after the user requested direct Montage playback. Equipment lifecycle and animation-layer lifetime are now independently controllable: `EquipWithoutAnimLayer()` / `UnequipWithoutAnimLayer()` still dispatch through the virtual Equip/Unequip chain, so Firearm ability grants and revokes remain correct, while `LinkEquipmentAnimLayers()` / `UnlinkEquipmentAnimLayers()` can be scheduled separately. Firearm AimSource initialization moved into its Link override so both immediate and delayed links initialize identically.

During a Montage switch, the outgoing weapon's gameplay lifecycle ends immediately but its stable Linked Layer remains temporarily attached. The incoming weapon's gameplay lifecycle, attachment, and rendering begin immediately, but its layer is not linked yet. The Montage therefore starts directly on both existing main AnimInstances without any graph mutation that could cancel it. At the returned Montage duration, the manager unlinks the outgoing layer and links the incoming layer into its held Idle. Repeat switch input is ignored only while this transition is active. No first-person component or incoming weapon is hidden.

Paused PIE showed the direct call state as `ArmsViewMesh=true`, `RepairGun hidden=false`, Montage active at `0`; the next animation step retained playback on both Arms and Body at `0.3338351`, proving it was not reset. After completion, Montage playback was false and `ABP_RepairGun_AnimLayer_C` was linked on the arms; a subsequent TestGun switch succeeded. `DirectEquipMontage_T0.png` records the visible equip motion. Development Editor/Win64 and Live Coding succeeded, W movement remained 250 cm/s, and `ShadowBodyMesh` retained `CharacterMesh0` as Leader.

Session101 removed the deferred-layer experiment after the user observed a T-pose at the end of the switch. That reference-pose gap was caused by ending the Montage first and only then rebuilding the graph to replace the outgoing layer with the incoming layer. The known-smooth initial equip path never performs a layer change after Montage playback begins.

Switching now exactly mirrors initial equip initialization: the outgoing equipment completes `Unequip()` (including ability revocation and layer unlink), the incoming equipment completes `Equip()` (including layer link, AimSource initialization, and ability grant), the actor is attached and visible, and a next-tick callback plays its Montage only if it is still current. The deferred Equip/Unequip APIs, explicit delayed layer methods, transition timer, and Montage-duration lock were deleted. This also guarantees that old weapon abilities are removed before new weapon abilities are granted.

Paused PIE confirmed that the RepairGun Linked Layer already existed on the switch frame. On the next tick both Arms and Body Montage instances were active at position `0`. After playback ended, both meshes retained the same `ABP_RepairGun_AnimLayer_C_1` instances and identical component-space `hand_r` positions; no end-of-Montage Link/Unlink remained that could emit a reference pose. `UnifiedEquip_PostMontage.png` visually confirms the held pose. Development Editor/Win64 and Live Coding succeeded; W movement remained 250 cm/s and Shadow Leader remained `CharacterMesh0`.

Session102 addressed the remaining one-frame held-Idle flash before the next-tick Montage. Harness history showed why the much older implementation appeared simpler: FEAT-006 replaced the entire AnimInstance (`SetAnimInstanceClass`) before linking and playing, a path later removed because weapon equipment destroyed the shared locomotion AnimBP. An attempted request consumed inside `UFPSCharacterAnimInstance::NativeUpdateAnimation` was rejected by deterministic testing because the same Linked Layer initialization still cleared playback; those temporary API and state changes were fully removed.

The retained solution bridges the initialization boundary without hiding the viewmodel or changing layers after playback. After complete outgoing `Unequip()` and incoming `Equip()`, the manager immediately calls `PlayEquipMontage()` so the current render interval has an equip pose instead of held Idle. It also schedules a current-equipment-guarded next-tick replay from position zero after layer initialization has stabilized. With zero Montage Blend In this repeats only the source start pose for one frame. If equipment is switched away while its Montage is active, `Unequip()` uses a 0.01-second nonzero Blend Out; testing proved that a zero-duration stop leaves the montage instance unavailable for immediate replay and must not be used.

Paused PIE produced the required sequence on both `ArmsViewMesh` and `CharacterMesh0`: switch call `playing=true, position=0`, first animation step/replay `true, 0`, next step `true, 0.3333334`. `EquipBridge_FirstEvaluated.png` shows the first evaluated frame already in the low equip start pose. Rapid switch-away made the old RepairGun Montage inactive immediately, and switching back retained the same bridge sequence. After completion both meshes kept `ABP_RepairGun_AnimLayer_C` and identical component-space `hand_r` positions; `EquipBridge_PostMontage.png` shows no T-pose. Full Development Editor and Live Coding builds succeeded, W movement was 250 cm/s, and Shadow Leader remained `CharacterMesh0`.

The user's final hands-on test confirmed the functional flash/T-pose regression was resolved, but the switch still felt less smooth than the initial BeginPlay equip. Work paused for the day with that subjective polish gap explicitly open. A future pass should compare the initial-equip and switch-equip evaluation timelines directly, while preserving the validated no-hiding, full equipment lifecycle, locomotion, shadow, and skill-isolation behavior.

## Native Linked-Layer Inertial Transition

Session104 replaced the repeated-Montage bridge with Unreal's native linked-graph blending path. UE 5.7 source inspection confirmed that `FAnimNode_LinkedAnimGraph::RequestBlend` reads each animation graph's `FAnimGraphBlendOptions` and forwards an inertialization request when the linked instance changes. `TABP_Firearm_UpperBodyBase` now gives both `WeaponAimOffset` and `WeaponUpperBody` a 0.1-second Blend In/Out, while `TABP_BodyLocomotion` places one `Inertialization` node after `FullBodySlot` and before the output pose.

`UEquipmentManagerComponent::SwitchEquipment()` no longer plays the Equip Montage immediately and restarts it on the next tick. It completes the existing full Unequip/Equip lifecycle, lets the linked layer initialize, then plays the Montage exactly once on the next tick. This retains old-ability revocation before new-ability grant and preserves each weapon's independent `ALI_WeaponAnim` implementation without introducing a permanent router AnimBP.

Unreal MCP compiled and saved `TABP_BodyLocomotion`, `TABP_Firearm_UpperBodyBase`, `ABP_RepairGun_AnimLayer`, and `ABP_MaintenanceWorker`; editor Live Coding and the final `TheManTestEditor Win64 Development` build both succeeded. Paused PIE verified the switch-back sequence on both `ArmsViewMesh` and `CharacterMesh0` as `inactive -> active@0 -> active@0.3333334`, removing the old duplicated `active@0 -> active@0 -> active@0.3333334` start. Both meshes had identical component-space `hand_r` transforms throughout and retained `ABP_RepairGun_AnimLayer_C` after Montage completion. W movement remained 250 cm/s; `ShadowBodyMesh` kept `CharacterMesh0` as Leader Pose and matched its component-space `thigh_l` transform exactly. Final subjective smoothness still requires the user's hands-on test at normal foreground frame rate.

## Known Cleanup Item

## Explicit Weapon Pose Transition

Session107 replaced the hidden/pre-rolled Montage workaround after a same-session trace proved why one missed frame still looked large: TestGun's final `hand_r` was `[-56.64,-0.34,111.68]`, while the first revealed RepairGun Equip sample was about `[-28.90,17.73,144.44]`, a visible tens-of-centimeters jump. AimPitch remained zero and both AnimInstances used the same Montage, so neither Aim Offset nor PlayRate was the root cause.

`UFPSCharacterAnimInstance` now saves the final outgoing pose as `WeaponTransitionPose`. `TABP_BodyLocomotion` blends that snapshot into the live FullBodySlot output with `WeaponTransitionAlpha`. The manager snapshots before unlinking, completes the normal ability/layer lifecycle, and atomically swaps actors while Alpha is zero, so the incoming actor initially follows the exact outgoing skeleton pose. After the short 0.08-second pose bridge reaches the incoming weapon's stable base pose, each Arms/Body AnimInstance starts the Equip Montage from zero; the Montage is not hidden, pre-rolled, replayed, or time-restored.

The final deterministic comparison recorded mutually exclusive weapon visibility, snapshot hold at Alpha 0, synchronized Arms/Body Montage positions, and `hand_r` delta 0 throughout the switch. A first bridge target at the stable RepairGun held pose was rejected because it created held → low Montage start → raised motion, perceived as “put down then pick up.” The retained target is the Equip Montage paused at exact time zero. At 60 FPS the bridge hand Z was `111.68 → 114.81 → 118.61 → 122.90 → 127.46 → 131.13`, followed by monotonic Montage time `0 → 0.0167 → 0.0333`; the motion enters the low start and then raises without a preceding drop. The actor swap also calls `SetGameCameraCutThisFrame()` to clear TAA/TSR history; six consecutive 1280x720 audit frames showed no old-gun silhouette. A rejected attempt to delay initial BeginPlay playback by another tick produced a `[0,0,0]` reference-pose sample and was fully reverted. Development Editor and Live Coding builds passed; normal foreground hands-on confirmation remains pending before the final full build and goal closure.

Session106 performed the missing direct same-session comparison between BeginPlay equip and switching back to RepairGun. The trace recorded Montage time, `ArmsViewMesh.hand_r`, `CharacterMesh0.hand_r`, current equipment actor transform, and visibility on every editor tick. It proved the remaining discontinuity was an actor/pose ownership mismatch: on the switch call RepairGun was already visible while the skeleton still held TestGun's pose; the following evaluated frame moved the hand and weapon by tens of centimeters. BeginPlay did not expose that mismatch because RepairGun was already the initialized visible equipment.

The switch is now a two-phase visual transaction. Gameplay ownership still changes immediately: old abilities are revoked, new abilities are granted, old/new Linked Layers are unlinked/linked, and `CurrentEquipmentIndex` changes synchronously. For a weapon with an Equip Montage, however, the outgoing actor stays visible for exactly the initialization frame while the incoming actor remains hidden. The Montage starts immediately, and the next-tick callback restores its elapsed time, atomically finalizes/hides the outgoing actor, and reveals the incoming actor after the new pose has evaluated. `bVisualSwapPending` ignores additional wheel input during this one-frame transaction, preventing stale callbacks and double-weapon residue.

The post-change trace verified mutually exclusive visibility on every sampled frame: RepairGun visible/TestGun hidden before the switch, RepairGun hidden/TestGun visible on the switch-back call frame while the arms retained the TestGun pose, then RepairGun visible/TestGun hidden on the first evaluated Equip frame at Montage position 0.333333 in the throttled environment. There was no double-visible or both-hidden frame, and the incoming weapon was never rendered against the outgoing weapon's pose. Live Coding succeeded; a final full build remains for the end of this iteration so the current editor instance can stay open for user testing.

Session105 followed the user's normal-frame-rate report that the session104 next-tick-only version still felt unchanged. The remaining visible difference from BeginPlay was the switch-call render interval: the new equipment actor was already attached and visible, while its Montage did not start until the next tick. The switch path now starts the Montage immediately at position zero, records world time, and on the next tick restarts at the elapsed position if linked-layer initialization cleared the first instance. `AEquipmentBase::PlayEquipMontage(float StartPosition)` applies the same restored position to both independent AnimInstances.

Paused PIE under the editor's 3 FPS automation throttle produced `active@0 -> active@0.3333335` on both Arms and Body instead of either exposing Idle or repeating zero. At a normal 60 FPS foreground tick this corresponds to approximately `0 -> 0.0167`, preserving a monotonic Montage clock. Component-space `hand_r` remained identical on both meshes. Live Coding and a full `TheManTestEditor Win64 Development` build succeeded. User hands-on confirmation remains pending.

- Active `BP_Infiltrator` still has a hard dependency on `/Game/Characters/Infiltrator/Blueprint/BP_Infiltrator_Old`. The exact referring property/node has not yet been identified; do not delete or rewrite it automatically.

## Pending Verification

- Compile each remaining Enemy skeleton-bound AnimBP.
- Confirm no missing animation references after the user's manual deletions.
- PIE-test player locomotion/weapon layers and each Enemy's patrol, turn, aim, and combat paths.

## Landing Pose Regression Fix

Session108 diagnosed a player disappearance that occurred only after landing. A 60 FPS PIE trace ruled out actor/component visibility and showed the actual failure: at the landing transition, `hand_r` on the body, shadow, legs, and then arms collapsed to the world origin for roughly 0.8 seconds before recovering.

The MaintenanceWorker override `/Game/Characters/MaintenanceWorker/Animations/AnimationSequenceBody/RTG_MM_Land` had regressed to `Local Space Additive` even though `LocomotionSM.Jump_End` consumes it as a complete locomotion pose. The asset was restored to `No Additive` (`AAT_NONE`, `ABPT_NONE`) without changing the shared state-machine architecture. Two automated jumps in one 60 FPS PIE session landed at frames 82 and 222 with no zero bone samples on any of the four player SkeletalMesh components and no visibility changes.
