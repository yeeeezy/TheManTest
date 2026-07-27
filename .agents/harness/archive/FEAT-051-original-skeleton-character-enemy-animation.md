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

## Known Cleanup Item

- Active `BP_Infiltrator` still has a hard dependency on `/Game/Characters/Infiltrator/Blueprint/BP_Infiltrator_Old`. The exact referring property/node has not yet been identified; do not delete or rewrite it automatically.

## Pending Verification

- Compile each remaining Enemy skeleton-bound AnimBP.
- Confirm no missing animation references after the user's manual deletions.
- PIE-test player locomotion/weapon layers and each Enemy's patrol, turn, aim, and combat paths.
