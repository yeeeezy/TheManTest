# FEAT-091 Character dynamic shadows

User authorized enabling CharacterMesh0.CastDynamicShadow on Executive/Infiltrator combat BPs to match Worker. Prior copy missed this flag. Other audited shadow/visibility flags match. No shared source or lobby changes.

Status: done. Both BPs compiled/saved; shadow-fix.json ok=true. Separate rendered editor session on secondary monitor: shadow-pie.json ok=true for all3 characters; body shadow flags/hidden-owner settings correct, arms/legs do not cast duplicate shadows, old shadow meshes empty. Existing weapon framing comparison also passed. Only2 combat BPs edited, no C++/map changes. Safety checkpoint9bc1c8b before repair; repair uncommitted and not pushed.

- User authorized committing and pushing all current work to origin/main. Publication includes prior drone and viewmodel checkpoints plus final shadow repair. Existing build/PIE evidence retained; no implementation changes for publication.
