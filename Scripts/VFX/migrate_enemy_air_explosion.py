"""Run in TMIIR: inspect the source map instance and migrate only Air 007 dependencies."""
import unreal
R=unreal.AssetRegistryHelpers.get_asset_registry()
R.search_all_assets(True)
root='/Game/NiagaraExplosion01/Niagaras/Air/N_ExplosionAir_007'
asset=unreal.load_asset(root)
assert asset
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level('/Game/NiagaraExplosion01/Maps/Overview_Map')
found=0
for actor in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    for component in actor.get_components_by_class(unreal.NiagaraComponent):
        if component.get_asset()==asset:
            print('AIR_SOURCE_INSTANCE',actor.get_actor_label(),component.get_world_transform())
            found+=1
assert found>0
options=unreal.AssetRegistryDependencyOptions(True,True,False,False,False)
pending=[root];closure=set()
while pending:
    p=pending.pop()
    if p in closure or not p.startswith('/Game/'):continue
    closure.add(p);pending.extend(str(x) for x in R.get_dependencies(p,options))
assert all(p.startswith('/Game/NiagaraExplosion01/') for p in closure),closure
print('AIR_SOURCE_DEPENDENCIES',len(closure))
unreal.AssetToolsHelpers.get_asset_tools().migrate_packages([unreal.Name(root)],r'D:\Unreal Projects\TheManTest\Content',unreal.MigrationOptions())
print('AIR_MIGRATION_OK')
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
