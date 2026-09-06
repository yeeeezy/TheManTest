import unreal,time,traceback
unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/VFXTest/VFXTestMap')
E=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
L=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
regions=[('Torso','spine_03'),('Head','head'),('LeftArm','lowerarm_l'),('RightArm','hand_r'),('LeftLeg','calf_l'),('RightLeg','foot_r')]
dirs=[('Front',(-1,0,0)),('Back',(1,0,0)),('Left',(0,1,0)),('Right',(0,-1,0))]
cases=[(r,b,d,v) for r,b in regions for d,v in dirs]
actors=[];enemies=[];stage=0;start=time.monotonic();baseline=[];classes=[];moves=[];root_enemies=[];root_starts=[]
def mesh(e):return e.get_editor_property('mesh')
def reaction(e):return e.get_component_by_class(unreal.EnemyHitReactionComponent)
def post(e):return mesh(e).get_post_process_instance()
def local(e,b):return mesh(e).get_socket_transform(b,unreal.RelativeTransformSpace.RTS_COMPONENT).translation
def fire(e,c):
    r,b,d,v=c
    reaction(e).react_to_explosion(mesh(e).get_socket_location(b),unreal.Vector(1,0,0),1,unreal.Vector(*v))
def check(moving):
    for i,(e,c) in enumerate(zip(enemies,cases)):
        r,b,d,v=c;p=post(e);a=p.get_editor_property('reaction_animation')
        assert a and a.get_name()=='AS_Humanoid_BlastRifle_'+d,(i,str(a))
        assert p.get_editor_property('reaction_alpha')>.95
        assert p.get_editor_property('use_full_body_reaction')==True,(i,moving)
        assert mesh(e).get_anim_instance().get_class()==classes[i]
        weapon=e.get_editor_property('weapon_mesh')
        assert (weapon.get_world_location()-mesh(e).get_socket_location('hand_r_wepSocket')).length()<.1
        delta=(local(e,'head')-baseline[i]).length()
        assert delta>.1,(i,delta)
        if moving:assert (e.get_actor_location()-moves[i]).length()>.1
        print('MIXAMO_RUNTIME',moving,r,d,'bone_cm',delta)
def tick(dt):
    global stage,start,enemies,baseline,classes,moves,first_time,root_enemies,root_starts
    try:
        elapsed=time.monotonic()-start
        if elapsed>45:raise RuntimeError('Timeout stage '+str(stage))
        if stage==0:
            cls=unreal.load_class(None,'/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C')
            for i,c in enumerate(cases):
                a=E.spawn_actor_from_class(cls,unreal.Vector(-8000,(i%4)*400,(i//4)*300+400),unreal.Rotator(0,0,90 if i%2 else 0),transient=False)
                a.set_editor_property('tags',['LimbRuntime'+str(i)]);actors.append(a)
            for j in range(3):
                a=E.spawn_actor_from_class(cls,unreal.Vector(-8000,7000+j*1000,500),unreal.Rotator(),transient=False)
                a.set_editor_property('tags',['RootCheck'+str(j)]);actors.append(a)
            wall=E.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(-7920,8000,500),unreal.Rotator(),transient=False)
            wall.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
            wall.set_actor_scale3d(unreal.Vector(.2,4,6))
            wall.static_mesh_component.set_collision_profile_name('BlockAll')
            actors.append(wall)
            L.editor_request_begin_play();stage=1;start=time.monotonic();return
        if stage==1:
            if elapsed<3:return
            world=unreal.EditorLevelLibrary.get_game_world()
            if not world:return
            enemies=[unreal.GameplayStatics.get_all_actors_with_tag(world,'LimbRuntime'+str(i))[0] for i in range(24)]
            for e in enemies:mesh(e).set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
            stage=2;start=time.monotonic();return
        if stage==2:
            if elapsed<.2:return
            baseline=[local(e,'head') for e,c in zip(enemies,cases)]
            classes=[mesh(e).get_anim_instance().get_class() for e in enemies]
            for e,c in zip(enemies,cases):fire(e,c)
            stage=3;start=time.monotonic();return
        if stage==3:
            if elapsed<.30:return
            check(False)
            first_time=post(enemies[0]).get_editor_property('reaction_time')
            fire(enemies[0],cases[1]);stage=4;start=time.monotonic();return
        if stage==4:
            if elapsed<.15:return
            assert post(enemies[0]).get_editor_property('reaction_animation').get_name().endswith('Rifle_Front')
            assert post(enemies[0]).get_editor_property('reaction_time')>first_time
            stage=5;start=time.monotonic();return
        if stage==5:
            if elapsed<1.6:return
            for e in enemies:
                assert post(e).get_editor_property('reaction_alpha')==0
                m=e.get_component_by_class(unreal.CharacterMovementComponent)
                m.set_movement_mode(unreal.MovementMode.MOVE_FLYING)
                m.set_editor_property('braking_deceleration_flying',0)
                m.set_editor_property('velocity',unreal.Vector(150,0,0))
                m.set_component_tick_enabled(True);m.activate(True)
            stage=6;start=time.monotonic();return
        if stage==6:
            if elapsed<.2:return
            baseline=[local(e,'head') for e,c in zip(enemies,cases)]
            moves=[e.get_actor_location() for e in enemies]
            for e,c in zip(enemies,cases):fire(e,c)
            stage=7;start=time.monotonic();return
        if stage==7:
            if elapsed<.30:return
            check(True)
            print('MIXAMO_RUNTIME_POSES_OK four directions independent of six impact locations, standing + moving')
            world=unreal.EditorLevelLibrary.get_game_world()
            root_enemies=[unreal.GameplayStatics.get_all_actors_with_tag(world,'RootCheck'+str(i))[0] for i in range(3)]
            root_starts=[e.get_actor_location() for e in root_enemies]
            for i,e in enumerate(root_enemies):
                mesh(e).set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
                m=e.get_component_by_class(unreal.CharacterMovementComponent)
                m.set_movement_mode(unreal.MovementMode.MOVE_FLYING)
                reaction(e).set_editor_property('apply_animation_root_motion',i!=2)
                fire(e,('Torso','spine_03','Back',(1,0,0)))
            stage=8;start=time.monotonic();return
        if stage==8:
            if elapsed<2:return
            distances=[(e.get_actor_location()-p).length() for e,p in zip(root_enemies,root_starts)]
            assert distances[0]>80,distances
            assert 1<distances[1]<60,distances
            assert distances[2]<.1,distances
            for e in root_enemies:
                assert e.get_component_by_class(unreal.CharacterMovementComponent).get_editor_property('movement_mode')==unreal.MovementMode.MOVE_FLYING
            print('ROOT_COLLISION_OK free/blocked/disabled cm',distances)
            fire(root_enemies[0],('Torso','spine_03','Back',(1,0,0)))
            stage=9;start=time.monotonic();return
        if stage==9:
            if elapsed<.12:return
            reaction(root_enemies[0]).set_editor_property('enabled',False)
            stage=10;start=time.monotonic();return
        if stage==10:
            if elapsed<.15:return
            assert root_enemies[0].get_component_by_class(unreal.CharacterMovementComponent).get_editor_property('movement_mode')==unreal.MovementMode.MOVE_FLYING
            assert post(root_enemies[0]).get_editor_property('reaction_alpha')==0
            print('MIXAMO_RUNTIME_OK poses/root/collision/movement restore/disable')
            L.editor_request_end_play();stage=11;start=time.monotonic();return
        if stage==11:
            if elapsed<1:return
            for a in actors:E.destroy_actor(a)
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()
    except Exception:
        unreal.log_error(traceback.format_exc());print('MIXAMO_RUNTIME_FAILED',stage)
        L.editor_request_end_play()
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
