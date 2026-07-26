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

## Known Cleanup Item

- Active `BP_Infiltrator` still has a hard dependency on `/Game/Characters/Infiltrator/Blueprint/BP_Infiltrator_Old`. The exact referring property/node has not yet been identified; do not delete or rewrite it automatically.

## Pending Verification

- Compile each rebuilt skeleton-bound AnimBP.
- Confirm no missing animation references after the user's manual deletions.
- PIE-test player locomotion/weapon layers and each Enemy's patrol, turn, aim, and combat paths.
