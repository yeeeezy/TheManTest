# [FEAT-049] Git and Git LFS Baseline

**Created:** 2026-07-26  
**Closed:** 2026-07-26  
**Status:** done

## Scope

Initialize local version control for the existing Unreal Engine 5.7 project before evaluating a third-party Unreal MCP plugin.

## Implementation

- Initialized a local Git repository on branch `main`.
- Initialized Git LFS for this repository.
- Added an Unreal-specific `.gitignore` that excludes generated build, cache, IDE, and local settings directories.
- Added `.gitattributes` rules for Unreal assets and common binary source-art formats.
- Excluded `.claude/settings.local.json` because it is machine-local configuration.
- Created baseline commit `33c70d1` (`Initial Unreal Engine 5.7 project baseline`).
- No Git remote was configured and no project data was uploaded.

## Verification

- Initial baseline contains 1653 tracked files.
- 1463 binary assets are managed by Git LFS.
- All 89 staged files larger than 10 MB were `.uasset` files covered by LFS.
- `Binaries`, `DerivedDataCache`, `Intermediate`, `Saved`, `.vs`, and `.idea` were absent from the commit.
- `git lfs fsck` completed successfully.
- `git fsck --full` found only two harmless dangling blobs left by pre-commit restaging; no missing or corrupt objects were reported.
