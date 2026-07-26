# [FEAT-045] New Character Select Camera Toggle

**Created:** 2026-07-04  
**Status:** done  
**Closed:** 2026-07-05-session72  
**Archive file:** `archive/FEAT-045-character-select-camera-toggle.md`

---

## Scope

User paused the upper-body animation work and started a new character select scene flow. This feature does not use the existing `LobbyMap` scene or the old `WBP_CharacterSelect` UI.

First slice: implement a reusable camera toggle for the new character select scene:

- Click once: switch from far view to near view.
- Click again: switch back to far view.
- The implementation should be usable by a new UI or any Blueprint click event in the new scene.

## Implementation Log

### 2026-07-04-session71 - Reusable scene camera switcher

- Added `ACharacterSelectCameraSwitcher`.
- Place one instance in the new character select map.
- Assign two level `CameraActor` references:
  - `FarCamera`
  - `NearCamera`
- On BeginPlay it sets the initial camera (`bStartInNearCamera`, default false).
- Exposes Blueprint-callable functions:
  - `ToggleCameraView()`
  - `SetNearCameraView()`
  - `SetFarCameraView()`
  - `IsUsingNearCamera()`
- Uses `PlayerController::SetViewTargetWithBlend` with configurable `BlendTime`, `BlendFunction`, and `BlendExp`.
- Adds `OnCameraViewChanged(bool bNearCamera)` Blueprint event for UI state updates.

### 2026-07-04-session71 addendum - Dedicated GameMode and Enhanced Input controller

- User clarified the new scene should not use old `LobbyMap` / old `WBP_CharacterSelect`.
- User also required Enhanced Input + IMC because later selection-screen actions will be added.
- Added `ACharacterSelectGameMode`.
  - `DefaultPawnClass = nullptr`.
  - `PlayerControllerClass = ACharacterSelectPlayerController`.
  - Optional `CharacterSelectWidgetClass` creates the new selection UI on BeginPlay.
- Added `ACharacterSelectPlayerController`.
  - Shows mouse cursor.
  - Sets `GameAndUI` input mode.
  - Adds `CharacterSelectMappingContext` through `UEnhancedInputLocalPlayerSubsystem`.
  - Binds `ClickAction` through `UEnhancedInputComponent`.
  - On click, calls `ACharacterSelectCameraSwitcher::ToggleCameraView()` only if `bPointerOverUI` is false.
  - Automatically finds the first `ACharacterSelectCameraSwitcher` in the scene, with optional `CameraSwitcherOverride` for manual assignment.
  - Exposes `SetPointerOverUI(bool)` for the new UI to call on mouse enter/leave so UI clicks do not toggle the camera.

### 2026-07-04-session71 addendum 2 - Mouse parallax camera offset

- User requested a subtle camera follow effect from mouse movement in all four directions.
- Implemented in `ACharacterSelectCameraSwitcher`, no new Enhanced Input action needed.
- `Tick` reads `PlayerController::GetMousePosition()` and viewport size.
- Mouse position is normalized around the screen center:
  - Left/right drives camera local Right axis.
  - Up/down drives camera local Up axis.
- The active camera location is set to `BaseCameraLocation + SmoothedParallaxOffset`.
- Far and near camera base transforms are cached at BeginPlay and restored when toggling so offsets do not accumulate or corrupt the authored Cine Camera positions.
- New editable parameters:
  - `bEnableMouseParallax`
  - `MouseParallaxHorizontalStrength`
  - `MouseParallaxVerticalStrength`
  - `MouseParallaxInterpSpeed`
  - `bInvertMouseParallax`

### 2026-07-04-session71 addendum 3 - Scale parallax by Cine Camera focal length

- User requested far/mid/near shots to feel visually similar when their Cine Camera focal lengths differ.
- Added `CinematicCamera` module dependency.
- `ACharacterSelectCameraSwitcher` now scales mouse parallax by the active Cine Camera's `CurrentFocalLength`:
  - `Scale = ReferenceFocalLength / CurrentFocalLength`
  - Longer focal length reduces world-space offset.
  - Wider focal length increases world-space offset.
- Scaling is clamped by `MinFocalLengthScale` / `MaxFocalLengthScale`.
- If the active camera is not a Cine Camera, scale falls back to `1.0`.
- New editable parameters:
  - `bScaleParallaxByFocalLength`
  - `ReferenceFocalLength`
  - `MinFocalLengthScale`
  - `MaxFocalLengthScale`

### 2026-07-04-session71 addendum 4 - Simple switch overshoot

- User chose the quick approach instead of a full camera rig:
  - Continue using `SetViewTargetWithBlend`.
  - When switching, temporarily offset the target camera past its authored base position along the travel direction.
  - Tick interpolates that overshoot offset back to zero.
- Overshoot and mouse parallax now share one final camera location write:
  - `BaseCameraLocation + CurrentParallaxOffset + CurrentSwitchOvershootOffset`
- Switching restores both far/near cameras to their cached base transforms before applying the new target overshoot, so offsets do not accumulate.
- New editable parameters:
  - `bEnableSwitchOvershoot`
  - `SwitchOvershootDistance`
  - `SwitchOvershootDistanceRatio`
  - `MaxSwitchOvershootDistance`
  - `SwitchOvershootReturnSpeed`

### 2026-07-05-session72 - Replace target-camera offset with runtime Camera Rig

- User reported the quick `SetViewTargetWithBlend` + target camera offset approach had no visible effect.
- Reworked `ACharacterSelectCameraSwitcher` to use an internally spawned runtime `ACineCameraActor` rig.
- Editor setup remains the same:
  - Place far Cine Camera.
  - Place near Cine Camera.
  - Assign both to `CharacterSelectCameraSwitcher`.
  - No manually placed rig actor is needed.
- Runtime behavior:
  - BeginPlay spawns a transient `ACineCameraActor` rig at the initial far/near camera transform.
  - PlayerController view target is the rig, not the authored far/near cameras.
  - Far/Near cameras are now pure authored target points and are not moved at runtime.
  - Switching changes the rig's target transform.
  - Rig location uses a damped spring toward the active target camera base location.
  - Mouse parallax is added to the rig final location.
  - Rig rotation interpolates toward the active target camera rotation.
  - Rig cine settings copy from the active target Cine Camera every tick: Filmback, LensSettings, FocusSettings, CropSettings, CurrentFocalLength, CurrentAperture, ExposureMethod, custom near clipping.
- Existing overshoot parameters are reused:
  - `SwitchOvershootDistance`
  - `SwitchOvershootDistanceRatio`
  - `MaxSwitchOvershootDistance`
  - `bEnableSwitchOvershoot`
- New spring parameters:
  - `SwitchSpringStrength`
  - `SwitchSpringDamping`
  - `SwitchRotationInterpSpeed`
- C++ Development Editor / Win64 build succeeded with no new warnings.

### 2026-07-05-session72 addendum - Spring defaults made more visible

- User felt the camera switch was too fast to read.
- Updated C++ defaults on `ACharacterSelectCameraSwitcher`:
  - `BlendTime = 0.0`
  - `SwitchSpringStrength = 28.0`
  - `SwitchSpringDamping = 5.0`
  - `SwitchRotationInterpSpeed = 4.0`
  - `SwitchOvershootDistance = 80.0`
  - `SwitchOvershootDistanceRatio = 0.08`
  - `MaxSwitchOvershootDistance = 180.0`
- Existing placed switcher instances may keep overridden old values; reset yellow arrows in Details or place a new switcher to use new defaults.
- C++ Development Editor / Win64 build succeeded with no new warnings.

### 2026-07-05-session72 addendum 2 - Initial view skips spring

- User requested the game start directly on the default far camera without spring/overshoot.
- Added internal `bHasAppliedInitialView`.
- First `ApplyCameraView()` now:
  - Places the runtime Camera Rig directly at the selected far/near base transform.
  - Clears `RigVelocity`.
  - Does not call `StartSwitchSpring()`.
- Later user-triggered camera toggles still use spring/overshoot.
- C++ Development Editor / Win64 build succeeded with no new warnings.

### 2026-07-05-session72 addendum 3 - Overshoot defaults increased

- User requested stronger defaults before shutting down.
- Updated C++ defaults:
  - `SwitchOvershootDistance = 500.0`
  - `MaxSwitchOvershootDistance = 1000.0`
- `SwitchOvershootDistanceRatio` remains `0.08`.
- Existing placed switcher instances may keep old overridden values; reset yellow arrows or replace the instance to use these new defaults.
- C++ Development Editor / Win64 build succeeded with no new warnings.

## Editor Setup

1. In the new character select map, place two `CameraActor`s: one far shot and one close shot.
2. Place `CharacterSelectCameraSwitcher`.
3. Assign `FarCamera` and `NearCamera` on the switcher.
4. Create `IA_CharacterSelectClick` as a Boolean input action.
5. Create `IMC_CharacterSelect` and bind Left Mouse Button to `IA_CharacterSelectClick`.
6. Create a BP subclass of `ACharacterSelectGameMode` for the new scene and use it as the map GameMode Override.
7. Create a BP subclass of `ACharacterSelectPlayerController` or set defaults on the GameMode/Controller class:
   - `CharacterSelectMappingContext = IMC_CharacterSelect`
   - `ClickAction = IA_CharacterSelectClick`
8. If the new UI should block camera toggles, call `SetPointerOverUI(true)` on the controller when the pointer enters UI hit areas, and `SetPointerOverUI(false)` when it leaves.
9. Tune mouse parallax on `CharacterSelectCameraSwitcher`:
   - `MouseParallaxHorizontalStrength`
   - `MouseParallaxVerticalStrength`
   - `MouseParallaxInterpSpeed`
   - `bInvertMouseParallax`
   - `bScaleParallaxByFocalLength`
   - `ReferenceFocalLength`
   - `MinFocalLengthScale`
   - `MaxFocalLengthScale`
10. Tune camera rig spring / overshoot on `CharacterSelectCameraSwitcher`:
   - `bEnableSwitchOvershoot`
   - `SwitchOvershootDistance`
   - `SwitchOvershootDistanceRatio`
   - `MaxSwitchOvershootDistance`
   - `SwitchSpringStrength`
   - `SwitchSpringDamping`
   - `SwitchRotationInterpSpeed`

## Verification

- C++ Development Editor / Win64 build succeeded after rerunning with permission for UBT AppData logs.
- Mouse parallax C++ build succeeded after rerunning with permission for UBT AppData logs.
- Focal-length scaled parallax C++ build succeeded after adding `CinematicCamera`.
- Simple switch overshoot C++ build succeeded.
- Runtime Camera Rig spring C++ build succeeded.
- Editor setup completed and PIE verified by user on 2026-07-05-session72:
  - `IA_CharacterSelectClick` / `IMC_CharacterSelect` created and configured.
  - New character-select GameMode / Controller configured.
  - Far / Near Cine Cameras placed and assigned to `CharacterSelectCameraSwitcher`.
  - Initial view starts on far camera.
  - Clicking a non-UI screen area switches to near camera, then clicking again switches back to far camera.
  - Clicking UI does not toggle the camera when UI exclusion is active.
  - Mouse parallax works in all four directions.
  - Runtime Camera Rig overshoots on switch and settles back onto the authored target camera.
