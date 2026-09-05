"""Cold read of the Air 007 installation; never saves maps or changes tuning."""
import unreal,os
E=unreal.EditorAssetLibrary
R=unreal.AssetRegistryHelpers.get_asset_registry();R.search_all_assets(True)
root='/Game/Weapons/ExplosionGun/Effects/EnemyExplosion'
path=root+'/Systems/NS_ExplosionGun_EnemyDetonation'
effect=E.load_asset(path);assert isinstance(effect,unreal.NiagaraSystem)
bp=E.load_asset('/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion')
obj=unreal.get_default_object(bp.generated_class())
assert obj.get_editor_property('enemy_explosion_effect')==effect
assert not obj.get_editor_property('enemy_effect_on_ground')
assert obj.get_editor_property('explosion_effect').get_path_name()=='/Game/Weapons/ExplosionGun/Effects/Explosion/Systems/NS_ExplosionGun_Detonation.NS_ExplosionGun_Detonation'
assert obj.get_editor_property('enemy_explosion_sound').get_name()=='SCue_ExplosionGun_EnemyDetonation'
assert obj.get_editor_property('explosion_sound').get_name()=='SCue_ExplosionGun_Detonation'
options=unreal.AssetRegistryDependencyOptions(True,True,False,False,False)
todo=[path];closure=set()
while todo:
    p=todo.pop()
    if p in closure or not p.startswith('/Game/'):continue
    assert p.startswith(root+'/'),p
    assert E.load_asset(p),p
    closure.add(p);todo.extend(str(x) for x in R.get_dependencies(p,options))
assets=R.get_assets_by_path(root,True)
assert len(closure)==115,(len(closure),len(assets))
assert not [a for a in assets if str(a.asset_class_path.asset_name)=='ObjectRedirector']
assert not R.get_assets_by_path('/Game/NiagaraExplosion01',True)
assert not os.path.exists(os.path.join(unreal.Paths.project_content_dir(),'NiagaraExplosion01'))
bullet=unreal.get_default_object(E.load_blueprint_class('/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet'))
print('AIR_COLD_OK',len(closure),'bullet_time',bullet.get_editor_property('bullet_time'),'shake',obj.get_editor_property('camera_shake_scale'),'enemy_scale',obj.get_editor_property('enemy_effect_scale'))
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
