"""Install enemy blast presentation and an additive Control Rig. Never saves maps."""
import unreal
E=unreal.EditorAssetLibrary
T=unreal.AssetToolsHelpers.get_asset_tools()
validate='-EnemyExplosionValidateOnly' in unreal.SystemLibrary.get_command_line()
rig_path='/Game/Enemy/Humanoid/Phantom/Animations/ControlRig/CR_Phantom_ExplosionReaction'
mesh=E.load_asset('/Game/Enemy/Humanoid/Phantom/OriginalRifle/Meshes/SK_Mannequin')
rig=E.load_asset(rig_path) if E.does_asset_exist(rig_path) else None
if not rig:
    assert not validate
    rig=unreal.ControlRigBlueprintFactory.create_control_rig_from_skeletal_mesh_or_skeleton(mesh)
    old=rig.get_path_name().split('.')[0]
    assert E.rename_asset(old,rig_path)
    rig=E.load_asset(rig_path)
    print('ENEMY_RIG_FACTORY_OLD',old)
if not validate:
    c=rig.get_controller()
    nodes={n.get_node_path() for n in rig.get_default_model().get_nodes()}
    if 'BeginExecution' not in nodes:
        assert c.add_unit_node_from_struct_path('/Script/ControlRig.RigUnit_BeginExecution','Execute',unreal.Vector2D(0,0),'BeginExecution')
    if 'EnemyReaction' not in nodes:
        assert c.add_unit_node_from_struct_path('/Script/TheManTest.RigUnit_EnemyHitReaction','Execute',unreal.Vector2D(400,0),'EnemyReaction')
    members={str(v.name) for v in rig.get_member_variables()}
    for name,cpp,obj,default in [('ReactionRotation','FVector',unreal.load_object(None,'/Script/CoreUObject.Vector'),'(X=0,Y=0,Z=0)'),('ReactionBone','FName',None,'None')]:
        if name not in members:rig.add_member_variable(name,obj.get_path_name() if obj else cpp,True,False,default)
        getter='Get'+name
        if getter not in nodes:
            assert c.add_variable_node(name,cpp,obj,True,default,unreal.Vector2D(100,200),getter)
        assert c.add_link(getter+'.Value','EnemyReaction.'+name)
    assert c.add_link('BeginExecution.ExecuteContext','EnemyReaction.ExecuteContext')
    rig.recompile_vm()
    assert E.save_loaded_asset(rig,False)
    post_path='/Game/Enemy/Humanoid/Phantom/Animations/ControlRig/ABP_Phantom_ExplosionReaction'
    host=E.load_asset(post_path) if E.does_asset_exist(post_path) else unreal.TheManAnimationAssetLibrary.create_enemy_hit_reaction_post_process(mesh,rig,post_path)
    assert host,'Rig graph installation failed'
    for p in [host,E.load_asset('/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom')]:
        unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([p])
        unreal.BlueprintEditorLibrary.compile_blueprint(p)
        assert E.save_loaded_asset(p,False)
    mesh.set_editor_property('post_process_anim_blueprint',host.generated_class())
    assert E.save_loaded_asset(mesh,False)
root='/Game/Weapons/ExplosionGun/Audio/'
wave_path=root+'S_ExplosionGun_EnemyDetonation'
if not E.does_asset_exist(wave_path):
    assert not validate
    task=unreal.AssetImportTask()
    task.filename=unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()+'Weapons/ExplosionGun/Audio/S_ExplosionGun_EnemyDetonation.wav')
    task.destination_path=root.rstrip('/');task.destination_name='S_ExplosionGun_EnemyDetonation'
    task.automated=True;task.save=True
    T.import_asset_tasks([task])
wave=E.load_asset(wave_path)
cue_path=root+'SCue_ExplosionGun_EnemyDetonation'
sound=E.load_asset(cue_path) if E.does_asset_exist(cue_path) else None
if not sound:
    assert not validate
    sound=T.create_asset('SCue_ExplosionGun_EnemyDetonation',root.rstrip('/'),unreal.SoundCue,unreal.SoundCueFactoryNew())
    assert unreal.TheManAudioAssetLibrary.initialize_variation_cue(sound,[wave],.95,1.05,.95,1)
    sound.set_editor_property('attenuation_settings',E.load_asset(root+'SA_ExplosionGun_Detonation'))
    sound.set_editor_property('concurrency_set',{E.load_asset(root+'SC_ExplosionGun_Detonation')})
    assert E.save_loaded_asset(sound,False)
bp=E.load_asset('/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion')
obj=unreal.get_default_object(bp.generated_class())
effect=E.load_asset('/Game/Weapons/ExplosionGun/Effects/Explosion/Systems/NS_ExplosionGun_Detonation')
if not validate:
    obj.set_editor_property('enemy_explosion_effect',effect)
    obj.set_editor_property('enemy_effect_on_ground',True)
    obj.set_editor_property('enemy_explosion_sound',sound)
    unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).open_editor_for_assets([bp,rig])
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    assert E.save_loaded_asset(bp,False)
    unreal.get_editor_subsystem(unreal.AssetEditorSubsystem).close_all_asset_editors()
assert obj.get_editor_property('enemy_explosion_sound')==sound
assert obj.get_editor_property('enemy_explosion_effect')==effect
assert obj.get_editor_property('enemy_effect_on_ground')
assert abs(wave.get_editor_property('duration')-1.3)<.001
assert wave.get_editor_property('num_channels')==1
assert isinstance(sound.get_editor_property('first_node'),unreal.SoundNodeModulator)
assert sound.get_editor_property('attenuation_settings') is not None
assert len(sound.get_editor_property('concurrency_set'))==1
assert obj.get_editor_property('explosion_sound')!=sound
post=E.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/ControlRig/ABP_Phantom_ExplosionReaction')
assert mesh.get_editor_property('post_process_anim_blueprint')==post.generated_class()
registry=unreal.AssetRegistryHelpers.get_asset_registry()
registry.search_all_assets(True)
for asset_root in ['/Game/Enemy/Humanoid/Phantom','/Game/Weapons/ExplosionGun']:
    redirects=[a.package_name for a in registry.get_assets_by_path(asset_root,True) if str(a.asset_class_path.asset_name)=='ObjectRedirector']
    assert not redirects,redirects
options=unreal.AssetRegistryDependencyOptions(include_hard_package_references=True,include_soft_package_references=True)
assert rig_path in [str(x) for x in registry.get_dependencies(post.get_path_name().split('.')[0],options)]
assert wave_path in [str(x) for x in registry.get_dependencies(cue_path,options)]
bullet=unreal.get_default_object(E.load_asset('/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet').generated_class())
print('ENEMY_EXPLOSION_PRESERVED',bullet.get_editor_property('bullet_time'),obj.get_editor_property('camera_shake_scale'),obj.get_editor_property('volume_multiplier'))
print('ENEMY_EXPLOSION_ASSETS_OK', 'cold' if validate else 'installed',rig.get_path_name())
unreal.SystemLibrary.execute_console_command(None,'QUIT_EDITOR')
