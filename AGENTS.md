# AGENTS.md

## Codex Startup

This project already uses a Claude Code harness under `.agents/harness/`.
For Codex sessions, treat that harness as the project source of truth.

Before making code or asset-related changes, read these files in order:

1. `.agents/harness/AGENTS.md`
2. `.agents/harness/feature_list.json`
3. `.agents/harness/progress.md`
4. The archive file for the current `active_feature` listed in `feature_list.json`

Use the harness rules for scope, completion criteria, verification commands, and handoff updates.

If `feature_list.json` cannot be parsed, report that first and continue from the readable harness/archive state unless the user asks to repair the harness.

## Animation Retargeting Boundary

- Never perform IK Retargeter setup, animation retargeting, or batch retarget generation inside `TheManTest`.
- `TheManTest` is a destination-only project for finalized animation assets. Do not leave source skeletons, source meshes, IK rigs, IK retargeters, or retargeting work folders in this project.
- Perform retargeting in the external source/resource project, validate it there, and migrate only the finalized animation assets into `TheManTest`.
- Approved external animation resource projects:
  - `D:\Unreal Projects\TMIIR`
  - `D:\Unreal Projects\FPSShooter1`
- When sourcing animations from FPSShooter1, perform any required retargeting in FPSShooter1 and migrate the finished assets to this project.
