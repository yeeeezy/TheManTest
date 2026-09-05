"""Isolated editor PIE preview validation. Never saves the temporary scene."""
import unreal,time,traceback
unreal.EditorLoadingAndSavingUtils.load_map('/Game/Maps/VFXTest/VFXTestMap')
names=['Front','Left','HeavyTwist','Right','Back']
stage=0;index=0;start=time.monotonic();actors=[];peak=0.
def tick(dt):
    global stage,index,start,enemy,body,weapon,baseline,peak,world
    try:
        elapsed=time.monotonic()-start
        if elapsed>40:raise RuntimeError('Reaction PIE timeout')
        if stage==0:
            cls=unreal.load_class(None,'/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C')
            a=unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(cls,unreal.Vector(-6000,0,100),unreal.Rotator(),transient=False)
            a.set_editor_property('tags',['ReactionPreviewTest']);actors.append(a)
            unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
            stage=1;start=time.monotonic();return
        if stage==1:
            if elapsed<3:return
            world=unreal.EditorLevelLibrary.get_game_world()
            if not world:return
            enemy=unreal.GameplayStatics.get_all_actors_with_tag(world,'ReactionPreviewTest')[0]
            body=enemy.get_editor_property('mesh');weapon=enemy.get_editor_property('weapon_mesh')
            body.set_editor_property('visibility_based_anim_tick_option',unreal.VisibilityBasedAnimTickOption.ALWAYS_TICK_POSE_AND_REFRESH_BONES)
            stage=2
        if stage==2:
            anim=unreal.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_RifleHit_'+names[index])
            body.play_animation(anim,False)
            baseline=None;peak=0.;stage=3;start=time.monotonic();return
        if stage==3:
            p=body.get_socket_location('head')
            if baseline is None:baseline=p
            peak=max(peak,(p-baseline).length())
            error=(body.get_socket_location('hand_r_wepSocket')-weapon.get_world_location()).length()
            assert error<.1,error
            anim=unreal.load_asset('/Game/Enemy/Humanoid/Phantom/Animations/Reactions/AS_Humanoid_RifleHit_'+names[index])
            if elapsed<anim.get_play_length()+.4:return
            assert peak>3,(names[index],peak)
            # Start may be one tick into the clip; authored raw endpoints are checked separately.
            print('REACTION_PIE_VALID',names[index],'head_motion_cm',peak,'weapon_socket_error',error)
            index+=1
            if index<len(names):stage=2;return
            print('REACTION_PREVIEW_PIE_OK',index)
            unreal.EditorLevelLibrary.editor_end_play();stage=4;start=time.monotonic();return
        if stage==4 and elapsed>2:
            unreal.unregister_slate_post_tick_callback(handle)
            for a in actors:unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(a)
            unreal.SystemLibrary.quit_editor()
    except Exception:
        unreal.log_error(traceback.format_exc());unreal.unregister_slate_post_tick_callback(handle)
        unreal.EditorLevelLibrary.editor_end_play();unreal.SystemLibrary.quit_editor()
handle=unreal.register_slate_post_tick_callback(tick)
