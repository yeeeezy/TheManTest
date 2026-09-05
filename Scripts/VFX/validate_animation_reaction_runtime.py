"""PIE integration checks, in an isolated editor. No map/package saves."""
import unreal,time,traceback
unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/VFXTest/VFXTestMap')
stage=0;start=time.monotonic();actors=[];enemies=[];baseline=[];initial_classes=[]
cases=[('Front',(1,0,0),.65),('Back',(-1,0,0),1),('Left',(0,-1,0),1),('Right',(0,1,0),1),('HeavyTwist',(1,0,0),1),('Left',(0,-1,0),1)]
def mesh(e):return e.get_editor_property('mesh')
def reaction(e):return e.get_component_by_class(unreal.EnemyHitReactionComponent)
def post(e):return mesh(e).get_post_process_instance()
def local(e,b):return mesh(e).get_socket_transform(b,unreal.RelativeTransformSpace.RTS_COMPONENT).translation
def fire(e,source,strength=1):
    direction=unreal.MathLibrary.transform_direction(e.get_actor_transform(),unreal.Vector(*source))
    origin=mesh(e).get_socket_location('spine_03')+direction*100
    reaction(e).react_to_explosion(origin,-direction,strength,'spine_03')
def tick(dt):
    global stage,start,enemies,baseline,initial_classes,first_time,move_start,slow_start,slow_time
    try:
        elapsed=time.monotonic()-start
        if elapsed>45:raise RuntimeError('Runtime reaction test timed out')
        if stage==0:
            cls=unreal.load_class(None,'/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C')
            for i in range(8):
                a=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(cls,unreal.Vector(-6500,i*220,100),unreal.Rotator(0,0,90 if i==5 else 0),transient=False)
                a.set_editor_property('tags',['RuntimeReaction'+str(i)]);actors.append(a)
            unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
            stage=1;start=time.monotonic();return
        if stage==1:
            if elapsed<3:return
            world=unreal.EditorLevelLibrary.get_game_world()
            if not world:return
            enemies=[unreal.GameplayStatics.get_all_actors_with_tag(world,'RuntimeReaction'+str(i))[0] for i in range(8)]
            for e in enemies:
                mesh(e).set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
                assert reaction(e).get_editor_property('reaction_mode')==unreal.EnemyHitReactionMode.ANIMATION
            baseline=[local(e,'head') for e in enemies]
            initial_classes=[mesh(e).get_anim_instance().get_class() for e in enemies]
            for e,case in zip(enemies,cases):fire(e,case[1],case[2])
            reaction(enemies[6]).set_editor_property('enabled',False);fire(enemies[6],(1,0,0))
            stage=2;start=time.monotonic();return
        if stage==2:
            if elapsed<.16:return
            for e,case in zip(enemies,cases):
                p=post(e)
                assert p.get_editor_property('reaction_animation').get_name()=='AS_Humanoid_RifleHit_'+case[0],case
                assert p.get_editor_property('reaction_alpha')>0
                assert p.get_editor_property('use_animation_reaction')
                assert p.get_editor_property('reaction_rotation').length()<.0001
                assert (local(e,'head')-baseline[enemies.index(e)]).length()>1,case
                weapon=e.get_editor_property('weapon_mesh')
                assert (weapon.get_world_location()-mesh(e).get_socket_location('hand_r_wepSocket')).length()<.1
                assert mesh(e).get_anim_instance().get_class()==initial_classes[enemies.index(e)]
                print('ANIM_RUNTIME_DIRECTION',case[0],'head_delta',(local(e,'head')-baseline[enemies.index(e)]).length())
            assert post(enemies[6]).get_editor_property('reaction_alpha')==0
            first_time=post(enemies[0]).get_editor_property('reaction_time');fire(enemies[0],(-1,0,0))
            stage=3;start=time.monotonic();return
        if stage==3:
            if elapsed<.12:return
            p=post(enemies[0])
            assert p.get_editor_property('reaction_animation').get_name().endswith('_Front')
            assert p.get_editor_property('reaction_time')>first_time
            print('ANIM_RUNTIME_REPEAT_IGNORED')
            stage=4;start=time.monotonic();return
        if stage==4:
            if elapsed<2:return
            for e in enemies:
                assert post(e).get_editor_property('reaction_alpha')==0
                assert (local(e,'head')-baseline[enemies.index(e)]).length()<2
            e=enemies[0];reaction(e).set_editor_property('reaction_mode',unreal.EnemyHitReactionMode.CONTROL_RIG)
            baseline[0]=local(e,'head');fire(e,(1,0,0));stage=5;start=time.monotonic();return
        if stage==5:
            if elapsed<.1:return
            e=enemies[0];p=post(e)
            assert not p.get_editor_property('use_animation_reaction')
            assert p.get_editor_property('reaction_rotation').length()>.01
            assert (local(e,'head')-baseline[0]).length()>1
            print('ANIM_RUNTIME_LEGACY_SWITCH_OK')
            reaction(e).set_editor_property('reaction_mode',unreal.EnemyHitReactionMode.ANIMATION)
            # Equal flying motion avoids floor/navigation dependencies; both use the live locomotion AnimBP.
            for m in [enemies[0],enemies[7]]:
                movement=m.get_component_by_class(unreal.CharacterMovementComponent);movement.set_editor_property('run_physics_with_no_controller',True)
                movement.set_component_tick_enabled(True);movement.set_movement_mode(unreal.MovementMode.MOVE_FLYING)
                movement.set_editor_property('velocity',unreal.Vector(150,0,0))
            stage=6;start=time.monotonic();return
        if stage==6:
            for e in [enemies[0],enemies[7]]:e.get_component_by_class(unreal.CharacterMovementComponent).set_editor_property('velocity',unreal.Vector(150,0,0))
            if elapsed<.4:return
            move_start=enemies[0].get_actor_location();fire(enemies[0],(0,-1,0))
            stage=7;start=time.monotonic();return
        if stage==7:
            for e in [enemies[0],enemies[7]]:e.get_component_by_class(unreal.CharacterMovementComponent).set_editor_property('velocity',unreal.Vector(150,0,0))
            if elapsed<.22:return
            assert not post(enemies[0]).get_editor_property('use_full_body_reaction')
            assert (enemies[0].get_actor_location()-move_start).length()>10
            for bone in ['pelvis','thigh_l','calf_l','foot_l','foot_r']:
                error=(local(enemies[0],bone)-local(enemies[7],bone)).length()
                assert error<.5,(bone,error)
            assert (local(enemies[0],'head')-local(enemies[7],'head')).length()>1
            print('ANIM_RUNTIME_MOVING_LEGS_OK')
            print('ANIMATION_REACTION_RUNTIME_OK')
            unreal.EditorLevelLibrary.editor_end_play();stage=8;start=time.monotonic();return
        if stage==8 and elapsed>2:
            unreal.unregister_slate_post_tick_callback(handle)
            for a in actors:unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(a)
            unreal.SystemLibrary.quit_editor()
    except Exception:
        unreal.log_error(traceback.format_exc());unreal.unregister_slate_post_tick_callback(handle)
        unreal.EditorLevelLibrary.editor_end_play();unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
