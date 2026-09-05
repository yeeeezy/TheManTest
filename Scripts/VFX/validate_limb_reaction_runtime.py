import unreal,time,traceback
unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/VFXTest/VFXTestMap')
E=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
L=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
regions=[('Torso','spine_03'),('Head','head'),('LeftArm','lowerarm_l'),('RightArm','hand_r'),('LeftLeg','calf_l'),('RightLeg','foot_r')]
dirs=[('Front',(-1,0,0)),('Back',(1,0,0)),('Left',(0,1,0)),('Right',(0,-1,0))]
cases=[(r,b,d,v) for r,b in regions for d,v in dirs]
actors=[];enemies=[];stage=0;start=time.monotonic();baseline=[];classes=[];moves=[]
def mesh(e):return e.get_editor_property('mesh')
def reaction(e):return e.get_component_by_class(unreal.EnemyHitReactionComponent)
def post(e):return mesh(e).get_post_process_instance()
def local(e,b):return mesh(e).get_socket_transform(b,unreal.RelativeTransformSpace.RTS_COMPONENT).translation
def fire(e,c):
    r,b,d,v=c
    reaction(e).react_to_explosion(mesh(e).get_socket_location(b),unreal.Vector(1,0,0),1,b,unreal.Vector(*v))
def check(moving):
    for i,(e,c) in enumerate(zip(enemies,cases)):
        r,b,d,v=c;p=post(e);a=p.get_editor_property('reaction_animation')
        assert a and a.get_name()=='AS_Humanoid_Blast_'+r+'_'+d,(i,str(a))
        assert p.get_editor_property('reaction_alpha')>.95
        assert p.get_editor_property('use_animation_reaction')
        assert p.get_editor_property('reaction_rotation').length()<.0001
        assert p.get_editor_property('use_full_body_reaction')==(not moving or r.endswith('Leg')),(i,moving)
        assert mesh(e).get_anim_instance().get_class()==classes[i]
        weapon=e.get_editor_property('weapon_mesh')
        assert (weapon.get_world_location()-mesh(e).get_socket_location('hand_r_wepSocket')).length()<.1
        delta=(local(e,b)-baseline[i]).length()
        assert delta>3,(i,delta)
        if moving:assert (e.get_actor_location()-moves[i]).length()>5
        print('LIMB_RUNTIME',moving,r,d,'bone_cm',delta)
def tick(dt):
    global stage,start,enemies,baseline,classes,moves,first_time
    try:
        elapsed=time.monotonic()-start
        if elapsed>45:raise RuntimeError('Timeout stage '+str(stage))
        if stage==0:
            cls=unreal.load_class(None,'/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C')
            for i,c in enumerate(cases):
                a=E.spawn_actor_from_class(cls,unreal.Vector(-8000,(i%4)*400,(i//4)*300+400),unreal.Rotator(0,0,90 if i%2 else 0),transient=False)
                a.set_editor_property('tags',['LimbRuntime'+str(i)]);actors.append(a)
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
            baseline=[local(e,c[1]) for e,c in zip(enemies,cases)]
            classes=[mesh(e).get_anim_instance().get_class() for e in enemies]
            for e,c in zip(enemies,cases):fire(e,c)
            stage=3;start=time.monotonic();return
        if stage==3:
            if elapsed<.19:return
            check(False)
            first_time=post(enemies[0]).get_editor_property('reaction_time')
            fire(enemies[0],cases[1]);stage=4;start=time.monotonic();return
        if stage==4:
            if elapsed<.15:return
            assert post(enemies[0]).get_editor_property('reaction_animation').get_name().endswith('Torso_Front')
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
            baseline=[local(e,c[1]) for e,c in zip(enemies,cases)]
            moves=[e.get_actor_location() for e in enemies]
            for e,c in zip(enemies,cases):fire(e,c)
            stage=7;start=time.monotonic();return
        if stage==7:
            if elapsed<.19:return
            check(True)
            print('LIMB_RUNTIME_OK 24 standing + 24 moving')
            L.editor_request_end_play();stage=8;start=time.monotonic();return
        if stage==8:
            if elapsed<1:return
            for a in actors:E.destroy_actor(a)
            unreal.unregister_slate_post_tick_callback(handle)
            unreal.SystemLibrary.quit_editor()
    except Exception:
        unreal.log_error(traceback.format_exc());print('LIMB_RUNTIME_FAILED',stage)
        L.editor_request_end_play()
        unreal.unregister_slate_post_tick_callback(handle)
        unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
