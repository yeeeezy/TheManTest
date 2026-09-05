"""Open/compile/save the two affected Blueprints; -FeedbackValidateOnly is read-only."""
import unreal

E=unreal.EditorAssetLibrary
validate='-FeedbackValidateOnly' in unreal.SystemLibrary.get_command_line()
paths=['/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet',
       '/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion']
assets=[E.load_asset(p) for p in paths]
if not validate:
    for bp in assets:
        unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([bp])
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        assert E.save_loaded_asset(bp,False)
bullet,cue=[unreal.get_default_object(bp.generated_class()) for bp in assets]
for name,value in [('explosion_delay',2),('damage',5),('explosion_damage',20),('explosion_damage_radius',400)]:
    assert bullet.get_editor_property(name)==value,(name,bullet.get_editor_property(name))
settings=bullet.get_editor_property('bullet_time')
for name,value in [('time_scale',.2),('slow_in_duration',.05),('hold_duration',.08),('recovery_duration',.25)]:
    assert abs(settings.get_editor_property(name)-value)<.0001
assert cue.get_editor_property('volume_multiplier')==3
assert cue.get_editor_property('camera_shake_scale')==4  # Preserve the user's latest saved override.
assert abs(cue.get_editor_property('shake_duration')-.75)<.0001
assert cue.get_editor_property('enemy_explosion_sound') is None
assert cue.get_editor_property('enemy_explosion_effect') is None
assert cue.get_editor_property('explosion_sound').get_name()=='SCue_ExplosionGun_Detonation'
assert cue.get_editor_property('explosion_effect') is not None
registry=unreal.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
assert not [a for a in registry.get_assets_by_path('/Game/Weapons/ExplosionGun',True) if str(a.asset_class_path.asset_name)=='ObjectRedirector']
print('EXPLOSION_FEEDBACK_OK', 'cold-read' if validate else 'compiled-saved',settings)
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
