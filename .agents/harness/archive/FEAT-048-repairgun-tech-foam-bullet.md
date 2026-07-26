# [FEAT-048] RepairGun Tech-Foam Bullet Mesh

**Created:** 2026-07-26  
**Closed:** 2026-07-26  
**Status:** done

## Scope

Replace the simple spherical RepairGun projectile mesh with a structured, faceted technological foam core while preserving its centered pivot and approximate 10 cm starting size. Do not modify the projectile Blueprint or expansion code.

## Final Design

- Geodesic icosphere base with deterministic directional deformation.
- Flat triangular facets for a technological, manufactured appearance.
- Structured protrusions rather than randomly fused bubbles.
- Dark cyan-blue, semi-matte material with slight metallic response.
- Blender source: `D:\Blender Projects\RepairGunBullet\RepairGunTechFoamBullet.blend`
- Preview: `D:\Blender Projects\RepairGunBullet\RepairGunTechFoamBullet_Preview.png`
- FBX: `D:\Blender Projects\RepairGunBullet\SM_RepairGun_Bullet.fbx`
- Geometry: 162 vertices, 320 triangular faces.
- Bounds: approximately 9.8 × 9.9 × 10 cm.

## Unreal Assets

- Static Mesh overwritten at `/Game/Weapons/RepairGun/Mesh/SM_RepairGun_Bullet`.
- Existing `/Game/Weapons/RepairGun/Mesh/M_RepairGun_Bullet` remains assigned as the single material slot.
- No RepairGun Blueprint reference or C++ expansion behavior was changed.

## Verification

- User approved the final low-exposure preview before Unreal import.
- UE 5.7 Interchange rebuilt and saved the Static Mesh with no missing-smoothing-group warning.
- Headless Interchange again hit the known UE 5.7 Slate Content Browser assertion after the package was saved.
- A separate clean Unreal Python commandlet loaded the overwritten asset as `StaticMesh` with one material slot and exited with `0 error(s), 0 warning(s)`.
