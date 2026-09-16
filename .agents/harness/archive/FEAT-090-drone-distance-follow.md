# FEAT-090 Distance-only drone follow

User authorized removing player-yaw coupling in gameplay. Follow starts beyond300cm and settles within180cm of owner elevated center; hold world position/yaw while nearby. Lobby unchanged. Existing changes checkpointed first.

- Implemented start300cm/stop180cm hysteresis around owner position+FollowOffset.Z. Gameplay anchor uses drone radial direction, never owner yaw. Nearby drone retains world goal/yaw; moving drone faces velocity. Lobby anchor/turn unchanged. distance-build.log Succeeded; actual PIE underway.

Status: done. distance-build.log Succeeded. distance-pie.json ok=true: lobby full turn preserved;160deg and reverse player turns do not alter settled drone position/yaw;20cm movement holds,900cm movement resumes following and settles184.98cm. Initial assertion sampled before drone finished decelerating; corrected external test waits velocity<1cm/s. distance-obstacles.json ok=true:343 samples no wall penetration,184.94cm final radius,pause freeze,unpossess cleans Pawn/controller. No Content/map changes; all UE processes exited. Checkpointff37920 before work; final changes uncommitted, no push.
