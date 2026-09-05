"""Rehome migrated Air 007 dependencies and configure only the enemy explosion slot."""
import unreal
E=unreal.EditorAssetLibrary
R=unreal.AssetRegistryHelpers.get_asset_registry();R.search_all_assets(True)
T=unreal.AssetToolsHelpers.get_asset_tools()
M=unreal.MaterialEditingLibrary
root='/Game/Weapons/ExplosionGun/Effects/EnemyExplosion'
assets=[a.get_asset() for a in R.get_assets_by_path('/Game/NiagaraExplosion01',True) if str(a.asset_class_path.asset_name)!='ObjectRedirector']
folders={'NiagaraSystem':('Systems','NS'),'NiagaraEmitter':('Niagara','NE'),'NiagaraScript':('Niagara','NMS'),'Material':('Materials','M'),'MaterialInstanceConstant':('Materials','MI'),'MaterialFunction':('Materials/Functions','MF'),'Texture2D':('Textures','T'),'StaticMesh':('Meshes','SM'),'CurveLinearColor':('Curves','Curve'),'CurveLinearColorAtlas':('Curves','CA')}
renames=[];textures=[]
for a in assets:
    folder,prefix=folders[a.get_class().get_name()]
    name='NS_ExplosionGun_EnemyDetonation' if a.get_name()=='N_ExplosionAir_007' else prefix+'_ExplosionGun_EnemyDetonation_'+a.get_name().split('_',1)[-1]
    assert not E.does_asset_exist(root+'/'+folder+'/'+name)
    renames.append(unreal.AssetRenameData(a,root+'/'+folder,name))
    if isinstance(a,unreal.MaterialInstanceConstant):
        for param in M.get_texture_parameter_names(a):
            value=M.get_material_instance_texture_parameter_value(a,param)
            if value:textures.append((a,param,value))
if renames:assert T.rename_assets(renames)
for a,param,value in textures:M.set_material_instance_texture_parameter_value(a,param,value)
for a in assets:assert E.save_loaded_asset(a,False)
path='/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion'
bp=E.load_asset(path);obj=unreal.get_default_object(bp.generated_class())
effect=E.load_asset(root+'/Systems/NS_ExplosionGun_EnemyDetonation');assert effect
obj.set_editor_property('enemy_explosion_effect',effect)
obj.set_editor_property('enemy_effect_on_ground',False)
unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([bp])
unreal.BlueprintEditorLibrary.compile_blueprint(bp);assert E.save_loaded_asset(bp,False)
unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).close_all_editors_for_asset(bp)
print('AIR_INSTALL_OK',len(assets),'texture_parameters',len(textures))
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
