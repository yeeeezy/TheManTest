# [FEAT-055] MaintenanceWorker Lower-Body Mesh

**Created:** 2026-07-26  
**Status:** done

## Source

- Full body: `/Game/Characters/MaintenanceWorker/TempCharacterBody/Meshes/SKM_UE4Mannequin`
- Skeleton: `/Game/Characters/MaintenanceWorker/TempCharacterBody/Meshes/SK_UE4Mannequin`
- Blender project: `D:\Blender Projects\MaintenanceWorkerLowerBody`

## Processing

- Exported the full-body Skeletal Mesh from Unreal as a skinned FBX.
- Kept source LOD0 and the complete 67-bone UE4 Mannequin armature.
- Cut geometry above 105 cm.
- Removed low-position A-pose hand remnants by retaining only vertices primarily weighted to `pelvis`, `thigh_*`, `calf_*`, `foot_*`, or `ball_*` groups.
- Preserved UVs, normals, skin weights, and the remaining body material slot.

## Result

- Unreal asset: `/Game/Characters/MaintenanceWorker/Meshes/SKM_MaintenanceWorker_LowerBody`
- Geometry: 6450 vertices, 11444 polygons.
- Bounds: 48.50 × 29.88 × 104.91 cm.
- Skeleton: existing `/Game/Characters/MaintenanceWorker/TempCharacterBody/Meshes/SK_UE4Mannequin`.
- Material: `/Game/Characters/MaintenanceWorker/TempCharacterBody/Materials/M_MannequinUE4_Body`.

## Remaining Integration

The asset is imported and verified but has not yet been assigned to a MaintenanceWorker character Blueprint/component. Configure that separately after confirming the intended first-person body component.
