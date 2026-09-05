"""Run in Unreal Editor Python. Add -AudioValidateOnly for a read-only cold audit.

Only explicit manifest consumers are changed. Source waves, maps and unrelated assets
are never rewritten. New sounds should be authored as Sound Cues using the same policy.
"""
import unreal

E = unreal.EditorAssetLibrary
T = unreal.AssetToolsHelpers.get_asset_tools()
VALIDATE = '-AudioValidateOnly' in unreal.SystemLibrary.get_command_line()
IMPACT = '/Game/Core/_Shared/Audio/SA_ProjectileImpact'
FIRE = '/Game/Weapons/_Shared/Audio/SA_WeaponFire'
MECHANICAL = '/Game/Weapons/_Shared/Audio/SA_WeaponMechanical'
EXPLOSION = '/Game/Weapons/ExplosionGun/Audio/SA_ExplosionGun_Detonation'


def asset(path, cls, factory):
    if E.does_asset_exist(path):
        result = E.load_asset(path)
    else:
        assert not VALIDATE, 'Missing ' + path
        result = T.create_asset(path.rsplit('/', 1)[1], path.rsplit('/', 1)[0], cls, factory)
    assert isinstance(result, cls), path
    return result


def attenuation(path, inner, falloff):
    a = asset(path, unreal.SoundAttenuation, unreal.SoundAttenuationFactory())
    s = a.get_editor_property('attenuation')
    values = dict(attenuate=True, spatialize=True, attenuation_shape=unreal.AttenuationShape.SPHERE,
                  attenuation_shape_extents=unreal.Vector(inner, 0, 0), falloff_distance=falloff,
                  distance_algorithm=unreal.AttenuationDistanceModel.LINEAR, stereo_spread=0.0)
    if not VALIDATE:
        for key, value in values.items():
            s.set_editor_property(key, value)
        a.set_editor_property('attenuation', s)
        assert E.save_loaded_asset(a, False)
    for key, value in values.items():
        assert s.get_editor_property(key) == value, (path, key)
    return a


def concurrency(path, count):
    a = asset(path, unreal.SoundConcurrency, unreal.SoundConcurrencyFactory())
    s = a.get_editor_property('concurrency')
    if not VALIDATE:
        s.set_editor_property('max_count', count)
        s.set_editor_property('resolution_rule', unreal.MaxConcurrentResolutionRule.STOP_QUIETEST)
        a.set_editor_property('concurrency', s)
        assert E.save_loaded_asset(a, False)
    assert s.get_editor_property('max_count') == count
    return a


attenuations = {IMPACT: attenuation(IMPACT, 180, 2200),
                FIRE: attenuation(FIRE, 250, 4750),
                MECHANICAL: attenuation(MECHANICAL, 100, 900),
                EXPLOSION: attenuation(EXPLOSION, 300, 5700)}
fire_concurrency = concurrency('/Game/Weapons/_Shared/Audio/SC_WeaponFire', 16)
mechanical_concurrency = concurrency('/Game/Weapons/_Shared/Audio/SC_WeaponMechanical', 8)
explosion_concurrency = concurrency('/Game/Weapons/ExplosionGun/Audio/SC_ExplosionGun_Detonation', 8)
impact_concurrency = E.load_asset('/Game/Core/_Shared/Audio/SC_ProjectileImpact')
flesh_concurrency = E.load_asset('/Game/Enemy/_Shared/Audio/SC_EnemyFleshHit')

# wave, cue, consumer, property, attenuation, concurrency, pitch variation
manifest = []
for weapon in ['RepairGun', 'ElectricGun', 'ExplosionGun']:
    root = '/Game/Weapons/' + weapon
    for kind, prop, att, conc, pitch in [('Fire', 'fire_sound', FIRE, fire_concurrency, .04),
                                        ('DryFire', 'dry_fire_sound', MECHANICAL, mechanical_concurrency, .03),
                                        ('Impact', 'impact_sound', IMPACT, impact_concurrency, .05)]:
        owner = root + '/Blueprint/BP_' + weapon if kind != 'Impact' else root + '/GAS/GameplayCues/GC_Weapon_' + weapon + '_Impact'
        manifest.append((root + '/Audio/S_' + weapon + '_' + kind,
                         root + '/Audio/SCue_' + weapon + '_' + kind, owner, prop, att, conc, pitch))
manifest += [('/Game/Enemy/_Shared/Audio/S_Enemy_FleshHit', '/Game/Enemy/_Shared/Audio/SCue_Enemy_FleshHit',
              '/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit', 'impact_sound', IMPACT, flesh_concurrency, .08),
             ('/Game/Weapons/ExplosionGun/Audio/S_ExplosionGun_AlienDetonation', '/Game/Weapons/ExplosionGun/Audio/SCue_ExplosionGun_Detonation',
              '/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion', 'explosion_sound', EXPLOSION, explosion_concurrency, .04),
             ('/Game/Weapons/TestGun/Audio/408399-Future_Weapons_3_-Blaster_2_-Shot_5', '/Game/Weapons/TestGun/Audio/SCue_TestGun_Fire',
              '/Game/Weapons/TestGun/Blueprint/BP_TestGun', 'fire_sound', FIRE, fire_concurrency, .04)]

changed = {}
for wave_path, cue_path, owner_path, prop, att, conc, pitch in manifest:
    wave = E.load_asset(wave_path)
    assert isinstance(wave, unreal.SoundWave)
    cue = asset(cue_path, unreal.SoundCue, unreal.SoundCueFactoryNew())
    if not VALIDATE:
        if not cue.get_editor_property('first_node'):
            assert unreal.TheManAudioAssetLibrary.initialize_variation_cue(cue, [wave], 1-pitch, 1+pitch, .95, 1.0)
        cue.set_editor_property('attenuation_settings', attenuations[att])
        cue.set_editor_property('concurrency_set', {conc})
        # Preserve the flesh bus; routing belongs to the Cue when it is the playing SoundBase.
        cue.set_editor_property('sound_submix_object', wave.get_editor_property('sound_submix_object'))
        assert E.save_loaded_asset(cue, False)
    node = cue.get_editor_property('first_node')
    assert isinstance(node, unreal.SoundNodeModulator)
    assert abs(node.get_editor_property('pitch_min') - (1-pitch)) < .0001
    assert abs(node.get_editor_property('pitch_max') - (1+pitch)) < .0001
    assert abs(node.get_editor_property('volume_min') - .95) < .0001
    assert cue.get_editor_property('attenuation_settings') == attenuations[att]
    assert conc in cue.get_editor_property('concurrency_set')
    assert abs(cue.get_editor_property('volume_multiplier') - 1) < .0001
    bp = E.load_asset(owner_path)
    cdo = unreal.get_default_object(bp.generated_class())
    if not VALIDATE:
        previous = cdo.get_editor_property(prop)
        assert previous in [wave, cue], ('Unexpected user sound override', owner_path, prop, previous)
        cdo.set_editor_property(prop, cue)
        changed[owner_path] = bp
    assert cdo.get_editor_property(prop) == cue
    print('AUDIO_POLICY_OK', cue_path, 'pitch', 1-pitch, 1+pitch, 'volume', .95, 1.0)

if not VALIDATE:
    for bp in changed.values():
        unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([bp])
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        assert E.save_loaded_asset(bp, False)

enemy = unreal.get_default_object(E.load_blueprint_class('/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit'))
explosion = unreal.get_default_object(E.load_blueprint_class('/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion'))
assert enemy.get_editor_property('volume_multiplier') == 5
assert explosion.get_editor_property('volume_multiplier') == 3
assert explosion.get_editor_property('camera_shake_scale') == 8
if VALIDATE:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.search_all_assets(True)
    options = unreal.AssetRegistryDependencyOptions(include_soft_package_references=True, include_hard_package_references=True)
    for wave_path, cue_path, owner_path, prop, att, conc, pitch in manifest:
        refs = [str(r) for r in registry.get_referencers(wave_path, options)]
        assert cue_path in refs, ('Missing original wave dependency', cue_path, refs)
        assert owner_path not in refs, ('Consumer still references bare wave', owner_path, refs)
    for root in ['/Game/Weapons', '/Game/Enemy/_Shared/Audio', '/Game/Core/_Shared/Audio']:
        redirects = [str(a.package_name) for a in registry.get_assets_by_path(root, True)
                     if str(a.asset_class_path.asset_name) == 'ObjectRedirector']
        assert not redirects, redirects
print('AUDIO_POLICY_DONE', len(manifest), 'cues; scan activation is a stable identification-tone exception; unused equip/template waves untouched')
unreal.SystemLibrary.execute_console_command(None, 'QUIT_EDITOR')
