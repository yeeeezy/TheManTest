# [FEAT-056] Gameplay Effect Materials

**Created:** 2026-07-29  
**Status:** superseded

## Scope

- Migrate the TMIIR Icosahedron effect material and dependencies into the RepairGun-owned `Effects` hierarchy, rename by gameplay purpose, and apply it to `BP_RepairGunBullet.BulletMesh`.
- Migrate the TMIIR high-detail Cube effect material and dependencies into the Infiltrator scan-owned `Effects` hierarchy, rename by gameplay purpose, and display it as a terrain overlay during scanning without replacing the terrain's base material.
- Add the external-asset directory rule to the project harness: feature ownership first, asset type second, no vendor/package directory retained in production paths.

## Target Layout

- `/Game/Weapons/RepairGun/Effects/Materials`
- `/Game/Weapons/RepairGun/Effects/Textures`
- `/Game/Weapons/RepairGun/Effects/Functions`
- `/Game/Characters/Infiltrator/Effects/Scan/Materials`
- `/Game/Characters/Infiltrator/Effects/Scan/Textures`
- `/Game/Characters/Infiltrator/Effects/Scan/Functions`
- Shared dependencies only when genuinely reused: `/Game/Effects/_Shared/<AssetType>`

## Verification

- [ ] Migrated assets and all dependencies load from project-semantic paths; no production reference remains under `/Game/ShapesFX_Pack`.
- [ ] `BP_RepairGunBullet` uses the migrated Icosahedron effect and retains projectile movement, expansion, hit, and damage behavior.
- [ ] Infiltrator scan displays the Cube terrain effect as an overlay and restores the original terrain appearance when inactive.
- [ ] Scan origin/radius/alpha remain synchronized through the existing scan material parameter pipeline.
- [ ] Related Blueprints/materials compile and save; scoped Redirector and reference audits pass.
- [ ] PIE validates RepairGun projectile visuals and Infiltrator terrain scan visuals without regressions.

## Log

- Session121 handoff: user paused visual iteration for the day. The current adaptive white/gold world-space band remains as the working implementation, but it is not the desired final art direction. The desired target is the original TMIIR effect's green, animated, high-tech motion rather than a static color approximation. Next session must begin read-only in `D:\Unreal Projects\TMIIR`: run the original DemoMap effect on its intended cube, capture/record the full animation cycle, and audit panner speed/direction, mask evolution, front/back layers, grid outline, MatCap response, Normal Push, Shrink, and fade timing. Only after that comparison should the main-project terrain/world-space version be redesigned. Do not infer the target from a still screenshot alone. Current assets are saved, PIE is stopped, and no final result commit was requested.

- Session120: user found the copied Cube_03 terrain treatment visually incompatible with the open TestMap. Comparison confirmed the original effect depends on the source cube's dense topology, per-face UVs, normal push, shrink, and front/back/outline composition; forcing those parameters through a 3000 cm decal washed the whole scene and amplified chromatic separation. A new native-node deferred decal material `/Game/Characters/Infiltrator/Effects/Scan/Materials/M_InfiltratorScanTerrainAdaptive` now renders a world-space warm white/gold double scan band (42 cm core, 180 cm halo) using only Emissive and Opacity. `UScanEffectComponent` creates a MID and drives independent `ScanOriginWS`, `ScanRadius`, and `ScanOpacity` parameters; the original red MPC remains separate. The component now has project defaults for both the MPC and adaptive material, and missing MPC no longer returns before terrain scanning. An initial UE Python CustomInput attempt crashed the editor in `python311.dll`; no partial asset was saved, the editor was automatically restarted, and the material was rebuilt entirely with native graph nodes. Both Development Editor and the actual DebugGame Editor configuration compiled successfully; DebugGame required a full 41/41 build and automatic restart because prior Live Coding patches were lost in the crash. Real E input screenshots `ScanAdaptive_DebugGame_Early.png` and `ScanAdaptive_DebugGame_Mid.png` show the gold/white band crossing terrain and the test cube while the new decal leaves sky and weapon untouched. Second E hides the runtime decal. W moved about 241 cm; two jumps ended at Z=90.15, zero velocity, `MOVE_WALKING`, with arms/shadow/legs visible. Existing red/green screen separation in the screenshots comes from the intentionally retained original red post-process scan, not the adaptive decal.

- Session119: user confirmed the hologram UI asset was intentionally deleted because its visual result was poor. Diagnosis showed `BGA_InfiltratorScan.HologramActorClass=None`, while both the original red MPC scan and the new terrain overlay `TriggerScan` call were incorrectly nested inside `if (HologramActorClass)`, so real E input ended the ability without either effect. `UGA_InfiltratorScan` now owns an independent per-actor `bScanActive` toggle: first E always triggers `UScanEffectComponent` and activation audio, second E retracts it; hologram spawning/hiding remains optional and cannot gate gameplay visuals. Development Editor/Win64 and Live Coding compiled successfully. Real Enhanced Input E verification created a visible `ScanTerrainOverlayDecal` with no hologram actors; screenshot `Saved/Screenshots/WindowsEditor/Scan_E_Input_NoHologram.png` shows the restored red outline/scan treatment and terrain effect. The second real E input changed the runtime decal visibility to false.

- Session118: user explicitly requested the scan effect again for temporary testing and identified that `DefaultAbilityClasses` incorrectly existed only on `AFPSInfiltrator`. The array and its grant loop were moved to `AFPSCharacterBase::PossessedBy`, so every concrete player Blueprint can configure character-owned active/passive/event-driven GAS abilities. The Infiltrator override was removed. Development Editor/Win64 and Live Coding compile successfully. After restart, the Infiltrator's serialized scan ability survived and MaintenanceWorker exposed an empty inherited array as intended. The scan ability, `MPC_ScanEffect`, and restored terrain material were temporarily assigned to MaintenanceWorker; both character Blueprints compiled. The nine scan-only material dependencies were restored from the rollback backup without restoring RepairGun assets. Runtime diagnosis found MaintenanceWorker initially had no `ScanMPC`, causing `TriggerScan` to return before decal creation. After assigning the MPC and registering the runtime decal as an owned instance component, PIE confirmed `ScanTerrainOverlayDecal` visible at size `(5000,3000,3000)` and screenshot `Saved/Screenshots/WindowsEditor/ScanTerrain_MaintenanceWorker_Working.png` shows the green scan ring plus terrain/world outline effect. W movement advanced about 325 cm; two successive SpaceBar jumps ended at Z=90.15, zero velocity, `MOVE_WALKING`, with arms/shadow/legs visible. The unrelated untracked TestMap External Actor was preserved and excluded.

- Session114: the user withdrew both original gameplay assignments after the movement-regression investigation. RepairGun and Infiltrator changes were restored to checkpoint `fb48d59`; FEAT-056 must not reapply those effects. The user then explicitly reassigned only the Icosahedron effect to `BP_InteractableBase`; that result is recorded under FEAT-054.

- Session112: feature created and external-asset directory rules added. Existing FEAT-051 work is paused without changing its implementation state.
- Session112 migration audit confirmed both requested assets are `MaterialInstanceConstant` instances sharing `/Game/ShapesFX_Pack/Materials/SHAPESFX/M_ShapesFx`. Shared dependencies include the MatCap projection function, mask/gradient textures, and a default MatCap; Icosahedron and Cube each have their own MatCap and outline textures.
- Unreal AssetTools successfully migrated the two instances and dependencies from TMIIR into the main project's temporary `/Game/ShapesFX_Pack` path. This is an intermediate staging path only; no production Blueprint currently references it.
- The follow-up AssetTools organization/application command was interrupted by the user before completion. Read-only filesystem and Git checks show no assets under the intended `/Game/Effects`, RepairGun `Effects`, or Infiltrator `Effects` destinations; all 11 migrated assets remain under the staging path. `BP_RepairGunBullet` is not dirty, so its material assignment did not land.
- A main-project commandlet audit loaded the materials successfully: the shared parent is an opaque, unlit Surface material. Loading `TestMap` in commandlet mode failed on the unrelated invalid-skeleton asset `/Game/Weapons/TestGun/Animation/Sequence/A_HandFire`; therefore terrain actor type and runtime overlay compatibility remain unverified. Do not treat this as a FEAT-056 implementation failure or silently repair that unrelated asset.
- Pause requested by user. On resume, first confirm the editor/process is idle and re-check Git/asset paths. Then run the prepared Unreal AssetTools organization step once, verify destinations/references before applying either gameplay use, and do not delete the staging folder or fix Redirectors without fresh confirmation.
