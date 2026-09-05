"""Explicit enemy pain import and consumer validation. -AudioValidateOnly is read-only."""
import unreal
E=unreal.EditorAssetLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
validate='-AudioValidateOnly' in unreal.SystemLibrary.get_command_line()
root='/Game/Enemy/_Shared/Audio/'
wave_path=root+'S_Enemy_Pain'
cue_path=root+'SCue_Enemy_Pain'
conc_path=root+'SC_EnemyPain'
if not E.does_asset_exist(wave_path):
    assert not validate
    task=unreal.AssetImportTask()
    task.set_editor_property('filename','C:/Users/ROG/Downloads/424116-Wizard-Pain-Vocal-Hurt-Uhh.wav')
    task.set_editor_property('destination_path',root.rstrip('/'))
    task.set_editor_property('destination_name','S_Enemy_Pain')
    task.set_editor_property('automated',True)
    task.set_editor_property('save',True)
    T.import_asset_tasks([task])
wave=E.load_asset(wave_path)
assert isinstance(wave,unreal.SoundWave)
def get(path,cls,factory):
    if E.does_asset_exist(path):return E.load_asset(path)
    assert not validate,path
    return T.create_asset(path.rsplit('/',1)[1],path.rsplit('/',1)[0],cls,factory)
conc=get(conc_path,unreal.SoundConcurrency,unreal.SoundConcurrencyFactory())
s=conc.get_editor_property('concurrency')
if not validate:
    s.set_editor_property('max_count',1)
    s.set_editor_property('limit_to_owner',True)
    s.set_editor_property('resolution_rule',unreal.MaxConcurrentResolutionRule.PREVENT_NEW)
    conc.set_editor_property('concurrency',s)
    assert E.save_loaded_asset(conc,False)
assert s.get_editor_property('max_count')==1 and s.get_editor_property('limit_to_owner')
cue=get(cue_path,unreal.SoundCue,unreal.SoundCueFactoryNew())
att=E.load_asset('/Game/Core/_Shared/Audio/SA_ProjectileImpact')
if not validate:
    if not cue.get_editor_property('first_node'):
        assert unreal.TheManAudioAssetLibrary.initialize_variation_cue(cue,[wave],.97,1.03,.95,1.0)
    cue.set_editor_property('attenuation_settings',att)
    cue.set_editor_property('concurrency_set',{conc})
    assert E.save_loaded_asset(cue,False)
assert cue.get_editor_property('attenuation_settings')==att
assert conc in cue.get_editor_property('concurrency_set')
mod=cue.get_editor_property('first_node')
assert isinstance(mod,unreal.SoundNodeModulator)
assert abs(mod.get_editor_property('pitch_min')-.97)<.0001
assert abs(mod.get_editor_property('pitch_max')-1.03)<.0001
bp=E.load_asset('/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit')
cdo=unreal.get_default_object(bp.generated_class())
bullet=E.load_asset('/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet')
bullet_cdo=unreal.get_default_object(bullet.generated_class())
blast=E.load_asset('/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion')
if not validate:
    assert cdo.get_editor_property('pain_sound') in [None,cue]
    cdo.set_editor_property('pain_sound',cue)
    cdo.set_editor_property('pain_volume_multiplier',1.0)
    cdo.set_editor_property('pain_cooldown',.6)
    for b in [bp,bullet,blast]:
        unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([b])
        unreal.BlueprintEditorLibrary.compile_blueprint(b)
        assert E.save_loaded_asset(b,False)
assert cdo.get_editor_property('pain_sound')==cue
assert abs(cdo.get_editor_property('pain_cooldown')-.6)<.0001
assert cdo.get_editor_property('volume_multiplier')==5
h=bullet_cdo.get_editor_property('bullet_time')
assert h.get_editor_property('enabled')
assert abs(h.get_editor_property('slow_in_duration')-.05)<.0001
assert abs(h.get_editor_property('hold_duration')-.08)<.0001
assert abs(h.get_editor_property('time_scale')-.2)<.0001
assert abs(h.get_editor_property('recovery_duration')-.25)<.0001
assert bullet_cdo.get_editor_property('explosion_damage')==20
assert bullet_cdo.get_editor_property('explosion_damage_radius')==400
assert bullet_cdo.get_editor_property('explosion_damage_effect_class') is not None
assert unreal.get_default_object(blast.generated_class()).get_editor_property('enemy_explosion_effect') is None
if validate:
    registry=unreal.AssetRegistryHelpers.get_asset_registry()
    registry.search_all_assets(True)
    options=unreal.AssetRegistryDependencyOptions(include_soft_package_references=True,include_hard_package_references=True)
    deps=[str(p) for p in registry.get_dependencies(cue_path,options)]
    assert wave_path in deps and conc_path in deps and '/Game/Core/_Shared/Audio/SA_ProjectileImpact' in deps,deps
    refs=[str(p) for p in registry.get_referencers(cue_path,options)]
    assert '/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit' in refs,refs
    assert not [a for a in registry.get_assets_by_path(root.rstrip('/'),True) if str(a.asset_class_path.asset_name)=='ObjectRedirector']
print('ENEMY_PAIN_BULLET_TIME_OK',wave.get_editor_property('duration'),wave.get_editor_property('num_channels'))
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
