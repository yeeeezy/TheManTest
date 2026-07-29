# [FEAT-054] InteractableBase Default Mesh

**Created:** 2026-07-26  
**Status:** done

## Result

- Created a reusable hard-surface sci-fi interaction crate without plus, minus, dot, or glyph decoration.
- Blender source directory: `D:\Blender Projects\InteractableBase`.
- Final geometry: 616 vertices, 594 polygons, bottom-center origin, 100 × 100 × 100 cm in Unreal.
- Imported mesh: `/Game/Actors/Interable/InteractableBase/Mesh/SM_InteractableBase_Default`.
- Imported materials:
  - `/Game/Actors/Interable/InteractableBase/Material/M_InteractableBase_Default`
  - `/Game/Actors/Interable/InteractableBase/Material/M_InteractableBase_Panel`
- `BP_InteractableBase` StaticMesh component now uses the new local mesh.

## Verification

- Unreal MCP reported bounds of 100 × 100 × 100 cm.
- Both material slots resolve to the dedicated `Material` directory.
- `BP_InteractableBase` dependencies no longer contain `/Game/SCI_FI_WEAPON_PACK`.
- No ObjectRedirector remains under the InteractableBase directory.

## Default Mesh Effect Material

- Session114 reassigned the staged ShapesFX Icosahedron material from the abandoned RepairGun use to `BP_InteractableBase`.
- The material and its eight actual dependencies now live under `/Game/Actors/Interable/InteractableBase/Effects/{Materials,Functions,Textures}` with project-semantic names.
- `BP_InteractableBase.StaticMesh` keeps the panel material in slot 0 and uses `/Game/Actors/Interable/InteractableBase/Effects/Materials/MI_InteractableDefaultEffect` in body slot 1.
- Unreal MCP compiled and saved the Blueprint, confirmed the new dependency, found no assets under `/Game/Weapons/RepairGun/Effects`, and confirmed `BP_RepairGunBullet.BulletMesh` still uses `/Game/Weapons/RepairGun/Material/M_RepairGun_Bullet`.

Session115 replaced that first effect pass after the user found the old detailed crate geometry and Icosahedron material visually mismatched. A new Blender-authored `SM_InteractableBase_EffectCube` is a clean 100 × 100 × 100 cm cube with a restrained 1.8 cm three-segment bevel, cube-projected UVs, one material slot, and an exact bottom-centre origin (`bounds min Z=0`, `max Z=100`). Blender source, FBX, build script, and preview remain under `D:\Blender Projects\InteractableBase`.

The original TMIIR `/Game/ShapesFX_Pack/Materials/SHAPESFX/Cubes/Hi/MI_ShapesFx_Cube_03` and its dependencies were migrated in the approved external source project, then reorganized under InteractableBase. The active instance is `/Game/Actors/Interable/InteractableBase/Effects/Materials/MI_InteractableCubeEffect`; it retains Cube_03's `T_Mask_09`, animated panning (`AnimationSpeed≈0.45`), white front face, and warm gold outline. `BP_InteractableBase.StaticMesh` now uses the new cube and this single material override. The unused Icosahedron instance and its exclusive MatCap/outline textures were deleted after a zero-referencer audit.

Session116 corrected the first Blender cube after an actual Unreal screenshot showed only isolated gold stripes. The material was active, but the low-density custom mesh had 96 vertices / 98 polygons and ordinary 0..1 cube UVs, while the source `SM_Geo_Cube` uses 1088 vertices / 5832 polygons and a regular -1..1 UV grid required by Cube_03's mask and vertex-driven `NormalPush`/shrink behavior. The final Blender mesh preserves that source topology, normals, and UVs while normalizing size and bottom-centre pivot. Unreal viewport captures `Saved/Screenshots/Cube03_Unreal_Verification_Compatible.png` and `Cube03_Unreal_Verification_Close.png` visibly confirm the full dense white/gold grid. The temporary verification actor was destroyed immediately and the level was not saved.

Session117 followed the user's final preference to use the source asset directly. TMIIR's original `SM_Geo_Cube` was migrated with Unreal AssetTools and renamed `/Game/Actors/Interable/InteractableBase/Mesh/SM_InteractableBase_OriginalCube`; `BP_InteractableBase.StaticMesh` now uses it at the source DemoMap scale `0.5` with the unchanged Cube_03 instance. The source bounds are ±261.25 cm, producing a 261.25 cm displayed cube. `Saved/Screenshots/Cube03_OriginalMesh_Final.png` is the direct Unreal viewport proof. The temporary actor was destroyed and the superseded custom EffectCube was deleted after a zero-referencer check.

## Workflow Rule Added

Blender modeling and revision tasks now generate and immediately open a clear local preview by default. This preview action no longer requires separate user confirmation; Unreal import and asset replacement still do.
