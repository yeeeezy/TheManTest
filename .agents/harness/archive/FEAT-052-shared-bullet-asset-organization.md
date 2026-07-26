# [FEAT-052] Shared Bullet Asset Organization

**Created:** 2026-07-26  
**Status:** done

## Scope

Create a reusable generic projectile mesh and organize weapon assets by ownership and asset type.

## Folder Convention

```text
/Game/Weapons/
├─ _Shared/
│  ├─ Mesh/
│  ├─ Material/
│  └─ Textures/
└─ <WeaponName>/
   ├─ Blueprint/
   ├─ Mesh/
   ├─ Material/
   ├─ Textures/
   └─ Animation/
```

- Put assets used by multiple weapons in `_Shared`.
- Keep weapon-specific meshes, materials, textures, blueprints, and animations under that weapon's folder.
- Do not place materials or textures inside a `Mesh` folder.

## Shared Bullet

- Blender project: `D:\Blender Projects\SharedBullet\SM_Shared_Bullet.blend`
- FBX: `D:\Blender Projects\SharedBullet\SM_Shared_Bullet.fbx`
- Preview: `D:\Blender Projects\SharedBullet\SM_Shared_Bullet_Preview.png`
- Unreal mesh: `/Game/Weapons/_Shared/Mesh/SM_Shared_Bullet`
- Materials:
  - `/Game/Weapons/_Shared/Material/M_Shared_Bullet`
  - `/Game/Weapons/_Shared/Material/M_Shared_Bullet_Accent`
- Dimensions: 8 cm long, 2.5 cm wide, 2.5 cm high; X axis forward.
- Geometry: 240 vertices, 226 polygons.
- No separate texture assets; the current appearance is material-driven.

## RepairGun Cleanup

- Mesh remains `/Game/Weapons/RepairGun/Mesh/SM_RepairGun_Bullet`.
- Material moved to `/Game/Weapons/RepairGun/Material/M_RepairGun_Bullet`.
- The mesh dependency was updated by Unreal AssetTools during the move.

## Verification

- Unreal MCP loaded the shared mesh and reported bounds of 8 × 2.5 × 2.5 cm.
- Both shared material slots resolve to their new `_Shared/Material` paths.
- RepairGun mesh dependency resolves to its new `RepairGun/Material` path.
- Recursive Asset Registry scan under `/Game/Weapons` found no `ObjectRedirector` assets.
