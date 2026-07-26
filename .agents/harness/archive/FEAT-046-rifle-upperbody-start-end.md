# [FEAT-046] Rifle Upper-Body 1D BlendSpace State Machine

**Created:** 2026-07-05  
**Status:** in_progress  
**Archive file:** `archive/FEAT-046-rifle-upperbody-start-end.md`

---

## Scope

Build the first pass of the rifle upper-body locomotion layer with a simple state machine shell:

- Idle
- Locomotion using a 1D BlendSpace driven by `Speed`

This feature only targets the weapon linked animation layer used by the rifle. It does not change the main player locomotion state machine, lower-body turn-in-place, input mapping, or weapon equip lifecycle.

The implementation intentionally does not branch by movement direction. It only distinguishes idle, walk, and run by speed.

## C++ Driver

`UEquipmentAnimInstance` exposes the shared equipment animation variables:

- `Speed`
- `Direction`
- `Velocity_Z`
- `bIsFalling`

Session75 removed the temporary Start/End-specific driver variables after the user decided to use a normal 1D BlendSpace.

## Planned Editor State Machine

Inside `WeaponUpperBody` on the rifle weapon layer:

```text
Idle <-> Locomotion
```

Recommended transitions:

- `Idle -> Locomotion`: `Speed > 3.0`
- `Locomotion -> Idle`: `Speed <= 3.0`

`Locomotion` should contain a 1D BlendSpace Player driven by `Speed`:

- 0: rifle upper-body idle
- walk speed point: rifle upper-body walk loop
- run speed point: rifle upper-body run loop

Recommended asset name: `BS_Rifle_UpperBody_Locomotion`. This should be rifle-layer generic, not MaintenanceWorker-specific, so any character equipping the rifle can reuse it.

## Implementation Log

### 2026-07-05-session73 - Feature opened

- Created feature record after FEAT-045 was closed.
- User confirmed the plan to add C++ animation driver variables first, then proceed through editor setup step by step.

### 2026-07-05-session73 - C++ driver variables added

- Updated `UEquipmentAnimInstance`.
- Existing linked weapon layers now expose:
  - `bHasMovementInput`
  - `bIsSprinting`
  - `bIsMoving`
  - `bShouldUseRunGait`
  - `MovementStartTrigger`
  - `MovementStopTrigger`
  - `MovingSpeedThreshold`
  - `RunSpeedThreshold`
- `bHasMovementInput` is driven from `UCharacterMovementComponent::GetCurrentAcceleration()`.
- `bIsSprinting` is read from `AFPSCharacterBase::IsSprinting()` when the owning actor is an FPS character.
- `bShouldUseRunGait` is true when sprinting or when `Speed >= RunSpeedThreshold`.
- `MovementStartTrigger` / `MovementStopTrigger` are one-frame pulses. The first editor pass should still use the stable booleans `bHasMovementInput` and `bShouldUseRunGait` for transitions.
- C++ Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.

### 2026-07-05-session74 - Editor state machine first pass

- User confirmed `TABP_Firearm_UpperBodyBase` / rifle upper-body AnimBP can read the new variables.
- Decision: build the logic in template `TABP_Firearm_UpperBodyBase`, not in child `ABP_Rifle_UpperBody`, because the child does not own an AnimGraph.
- `AnimGraph` is empty and was not used.
- In the `WeaponUpperBody` layer, the existing upper-body BlendSpace input to `Layered Blend per Bone` was replaced with a state machine output while keeping the base pose, blend node, and output pose structure intact.
- First-pass states and basic transitions were created:
  - `Idle`
  - `WalkStart`
  - `WalkLoop`
  - `WalkEnd`
  - `RunStart`
  - `RunLoop`
  - `RunEnd`
- Pending next session: compile the template/child AnimBPs, then PIE-test rifle idle, walk start/loop/end, run start/loop/end, and ArmsViewMesh/GetMesh upper-body sync.

### 2026-07-05-session75 - Switched to simple 1D BlendSpace

- User decided the Start/Loop/End animation transitions did not line up well enough.
- New direction: keep `SM_FirearmUpperBody` as the state machine shell, but simplify it to `Idle <-> Locomotion`.
- `Locomotion` will use a 1D BlendSpace driven by `Speed` for rifle upper-body idle/walk/run loops.
- Removed the temporary Start/End-specific C++ variables from `UEquipmentAnimInstance`; it now only exposes `Speed` / `Direction` / `Velocity_Z` / `bIsFalling`.
- User already created the `Idle <-> Locomotion` transitions with `Speed > 3.0` and `Speed <= 3.0`.
- C++ Development Editor / Win64 build succeeded after the cleanup with no new warnings.
- Recommended shared BlendSpace asset name recorded as `BS_Rifle_UpperBody_Locomotion`; user intends MaintenanceWorker + Rifle to use this same rifle upper-body layer.
- Pending: add the 1D BlendSpace Player inside `Locomotion`, compile AnimBPs, and PIE-test ArmsViewMesh/GetMesh sync.

## Verification

- C++ Development Editor / Win64 build succeeded with no new warnings.
- Editor state machine first pass created by user in `TABP_Firearm_UpperBodyBase`.
- Session75 C++ cleanup build succeeded with no new warnings.
- Pending AnimBP compile verification.
- Pending PIE verification.

### 2026-07-26-session80 - MCP audit and handoff

- Unreal MCP found the actual `SM_FirearmUpperBody` states are `Idle <-> WalkRun`, not the recorded `Idle <-> Locomotion`.
- `/Game/Weapons/RepairGun/Animation/BlendSpace/BS_Rifle_UpperBody_IdleWalkRun` is currently a 2D BlendSpace with zero samples, not the planned populated 1D BlendSpace.
- The user removed part of the unsatisfactory retargeted animation set and changed the immediate priority to rebuilding character/enemy AnimBPs around suitable original skeletons.
- FEAT-046 is now `needs_improvement`; FEAT-051 owns the next animation pass.
