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
