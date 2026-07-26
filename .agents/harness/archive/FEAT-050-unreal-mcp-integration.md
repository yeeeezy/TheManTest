# [FEAT-050] UE 5.7 Unreal MCP Isolated Integration

**Created:** 2026-07-26  
**Status:** done  
**Branch:** `test/unreal-mcp-5.7`

## Scope

Evaluate a third-party Unreal MCP without changing the stable UE 5.7.4 baseline on `main`. The integration is local-only and defaults to read-only inspection; all project mutations still require a concrete plan and explicit user confirmation.

## Implementation

- Pinned `ChiR24/Unreal_mcp` at tag `v0.5.30` under `D:\Unreal Plugins\Unreal_mcp-v0.5.30`.
- Added that repository's `plugins` directory through `AdditionalPluginDirectories`.
- Enabled `McpAutomationBridge` only on the isolated test branch.
- Disabled the legacy WebSocket listener and enabled Native MCP at `http://127.0.0.1:3000/mcp`.
- Kept non-loopback access disabled and embedded the project's confirmation-before-mutation rule in the server instructions.
- Registered the endpoint in the Codex user configuration as `unreal-engine`.

## Verification

- UE 5.7.4 Development Editor / Win64 build: succeeded, 25/25 actions.
- Editor log: module/subsystem initialized and 23 self-describing tools registered.
- MCP `initialize`: HTTP 200, protocol version `2025-03-26`, server `unreal-mcp`.
- MCP `tools/list`: HTTP 200, 23 tools returned.
- No asset-mutating MCP tool was invoked.

## Known Notes

- The plugin logs that `Resources/MCP/server-info.json` is absent and uses built-in defaults. Handshake and tool discovery still succeed; this is non-blocking for v0.5.30.
- `StructUtils` produces a UE 5.7 deprecation warning in plugin metadata, but compilation succeeds.
- A new Codex session is required before the newly registered MCP appears as a native tool in the session.
- `.agents/harness/init.ps1` remains invalid due to existing encoding/parser errors; it was reported and not repaired as part of this feature.
- Do not merge the test branch into `main` until the user explicitly approves retaining the plugin.
