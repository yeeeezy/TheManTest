#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundAttenuation.h"
#include "AudioMixerBlueprintLibrary.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Misc/Paths.h"
#include "Materials/Material.h"
#include "MaterialShared.h"
#include "AssetCompilingManager.h"
#include "Misc/App.h"

namespace
{
class FSpatialImpactCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int Stage=0,Sample=0;
 float Start=0;
 float PreviousUnfocusedVolume=0.f;
 bool bChangedUnfocusedVolume=false;
 UGCN_EnemyHit* Cue=nullptr;
 FVector Listener=FVector(0,0,160);
 TArray<TWeakObjectPtr<UDecalComponent>> Decals;
public:
 explicit FSpatialImpactCommand(FAutomationTestBase* T):Test(T){}
 virtual ~FSpatialImpactCommand() override
 {
  if(bChangedUnfocusedVolume)FApp::SetUnfocusedVolumeMultiplier(PreviousUnfocusedVolume);
 }
 virtual bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  if(!World){Test->AddError(TEXT("Missing PIE world"));return true;}
  auto* PC=World->GetFirstPlayerController();
  if(Stage==0)
  {
   PreviousUnfocusedVolume=FApp::GetUnfocusedVolumeMultiplier();
   FApp::SetUnfocusedVolumeMultiplier(1.f);
   bChangedUnfocusedVolume=true;
   auto* Class=LoadClass<UGCN_EnemyHit>(nullptr,TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"));
   if(!Class){Test->AddError(TEXT("Missing enemy Cue"));return true;}
   Cue=Class->GetDefaultObject<UGCN_EnemyHit>();
   Test->TestNotNull(TEXT("3D attenuation assigned"),Cue->ImpactAttenuation.Get());
   if(!Cue->ImpactAttenuation)return true;
   const auto& Settings=Cue->ImpactAttenuation->Attenuation;
   Test->TestTrue(TEXT("Spatialization enabled"),bool(Settings.bSpatialize));
   Test->TestTrue(TEXT("Distance attenuation enabled"),bool(Settings.bAttenuate));
   Test->TestEqual(TEXT("Point sound has no omnidirectional radius"),Settings.NonSpatializedRadiusStart,0.f);
   Test->TestEqual(TEXT("User flesh gain preserved"),Cue->VolumeMultiplier,5.f);
   // Same impact position must still yield varying projected rotation, size and noise pattern.
   auto* WeaponClass=LoadClass<UGCN_ImpactFeedbackBase>(nullptr,TEXT("/Game/Weapons/ElectricGun/GAS/GameplayCues/GC_Weapon_ElectricGun_Impact.GC_Weapon_ElectricGun_Impact_C"));
   auto* Weapon=WeaponClass->GetDefaultObject<UGCN_ImpactFeedbackBase>();
   Test->TestNull(TEXT("Weapon creates no voice when hitting a character"),Weapon->SpawnImpactSound(World,Listener,true));
   Test->TestFalse(TEXT("Weapon owns no character hit voice"),Weapon->ShouldPlayImpactSound(true));
   Test->TestTrue(TEXT("Character Cue owns character hit voice"),Cue->ShouldPlayImpactSound(true));
   // Exercise the real OnExecute route for every weapon, not only the audio helper.
   FGameplayCueParameters CharacterParams;
   CharacterParams.Location=Listener;
   CharacterParams.Normal=FVector::UpVector;
   CharacterParams.EffectContext=FGameplayEffectContextHandle(new FGameplayEffectContext());
   CharacterParams.EffectContext.AddHitResult(FHitResult(PC->GetPawn(),nullptr,Listener,FVector::UpVector));
   for(const TCHAR* Name:{TEXT("RepairGun"),TEXT("ElectricGun"),TEXT("ExplosionGun")})
   {
    const FString Path=FString::Printf(TEXT("/Game/Weapons/%s/GAS/GameplayCues/GC_Weapon_%s_Impact.GC_Weapon_%s_Impact_C"),Name,Name,Name);
    auto* HitClass=LoadClass<UGCN_ImpactFeedbackBase>(nullptr,*Path);
    if(!HitClass){Test->AddError(TEXT("Missing weapon hit Cue"));continue;}
    auto* HitCue=HitClass->GetDefaultObject<UGCN_ImpactFeedbackBase>();
    auto CountVoices=[World,HitCue]()
    {
     int Count=0;
     for(TObjectIterator<UAudioComponent> It;It;++It)
      if(It->GetWorld()==World&&It->Sound==HitCue->ImpactSound)++Count;
     return Count;
    };
    const int Before=CountVoices();
    HitCue->OnExecute_Implementation(PC->GetPawn(),CharacterParams);
    Test->TestEqual(FString::Printf(TEXT("%s OnExecute on character creates no weapon voice"),Name),CountVoices(),Before);
   }
   FGameplayCueParameters Params;Params.Location=FVector(0,300,1);Params.Normal=FVector::UpVector;
   for(int I=0;I<5;++I)Weapon->OnExecute_Implementation(PC->GetPawn(),Params);
   for(TObjectIterator<UDecalComponent> It;It;++It)
    if(It->GetWorld()==World&&It->GetComponentLocation().Equals(FVector(0,300,2),.1))Decals.Add(*It);
   Test->TestEqual(TEXT("Five impact decals spawned"),Decals.Num(),5);
   if(Decals.Num()>1)
   {
    Test->TestFalse(TEXT("Random rotation differs"),Decals[0]->GetComponentQuat().Equals(Decals[1]->GetComponentQuat(),.001));
    Test->TestFalse(TEXT("Random dimensions differ"),Decals[0]->DecalSize.Equals(Decals[1]->DecalSize,.001));
    auto* A=Cast<UMaterialInstanceDynamic>(Decals[0]->GetDecalMaterial());auto* B=Cast<UMaterialInstanceDynamic>(Decals[1]->GetDecalMaterial());
    Test->TestTrue(TEXT("Pattern variation uses independent material instances"),A&&B&&A!=B);
    if(A&&B)Test->TestFalse(TEXT("Pattern offsets differ"),A->K2_GetVectorParameterValue(TEXT("DecalPatternOffset")).Equals(B->K2_GetVectorParameterValue(TEXT("DecalPatternOffset"))));
   }
   auto* Camera=World->SpawnActor<ACameraActor>(Listener,FRotator::ZeroRotator);
   PC->SetViewTarget(Camera);PC->SetAudioListenerOverride(Camera->GetCameraComponent(),FVector::ZeroVector,FRotator::ZeroRotator);
   Start=World->GetTimeSeconds();Stage=1;return false;
  }
  if(Stage==1)
  {
   if(World->GetTimeSeconds()-Start<2.f)return false; // prior weapon feedback completely decays
   UAudioMixerBlueprintLibrary::StartRecordingOutput(World,1.5f);
   const int Position=Sample%3;
   const FVector Offset=Position==0?FVector(180,-360,0):Position==1?FVector(180,360,0):FVector(850,-1700,0);
   UGCN_ImpactFeedbackBase* Source=Cue;
   if(Sample>=3)Source=LoadClass<UGCN_ImpactFeedbackBase>(nullptr,TEXT("/Game/Weapons/ElectricGun/GAS/GameplayCues/GC_Weapon_ElectricGun_Impact.GC_Weapon_ElectricGun_Impact_C"))->GetDefaultObject<UGCN_ImpactFeedbackBase>();
   auto* Sound=Source->SpawnImpactSound(World,Listener+Offset,false);
   Test->TestNotNull(TEXT("Flesh audio voice created"),Sound);
   if(Sound)
   {
    Test->TestTrue(TEXT("Flesh voice is playing"),Sound->IsPlaying());
    Test->TestTrue(TEXT("Voice is at hit position"),Sound->GetComponentLocation().Equals(Listener+Offset,.01));
    Test->TestTrue(TEXT("Voice uses shared attenuation"),Sound->AttenuationSettings==Cue->ImpactAttenuation);
   }
   Start=World->GetTimeSeconds();Stage=2;return false;
  }
  if(World->GetTimeSeconds()-Start<1.25f)return false;
  const int Position=Sample%3;
  const FString Name=FString(Sample<3?TEXT("Hit_"):TEXT("Electric_"))+(Position==0?TEXT("NearLeft"):Position==1?TEXT("NearRight"):TEXT("FarLeft"));
  UAudioMixerBlueprintLibrary::StopRecordingOutput(World,EAudioRecordingExportType::WavFile,Name,FPaths::ProjectSavedDir()/TEXT("AudioCaptures"));
  if(++Sample<6){Stage=1;Start=World->GetTimeSeconds();return false;}
  PC->ClearAudioListenerOverride();
  FApp::SetUnfocusedVolumeMultiplier(PreviousUnfocusedVolume);
  bChangedUnfocusedVolume=false;
  for(auto D:Decals)if(D.IsValid())D->DestroyComponent();
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpatialImpactTest,"TheManTest.Player.Weapons.SpatialImpactFeedback",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FSpatialImpactTest::RunTest(const FString&)
{
 for(const TCHAR* Path:{TEXT("/Game/Enemy/_Shared/Effects/Hit/Materials/M_Enemy_BloodStain.M_Enemy_BloodStain"),TEXT("/Game/Weapons/ElectricGun/Effects/Impact/Materials/M_ElectricGun_ImpactDecalVaried.M_ElectricGun_ImpactDecalVaried"),TEXT("/Game/Weapons/ExplosionGun/Effects/Impact/Materials/M_ExplosionGun_ImpactDecalVaried.M_ExplosionGun_ImpactDecalVaried")})
 {
  auto* Mat=LoadObject<UMaterial>(nullptr,Path);
  FAssetCompilingManager::Get().FinishAllCompilation();
  if(!TestNotNull(TEXT("Varied decal material loads"),Mat))return false;
  const auto* Resource=Mat->GetMaterialResource(GMaxRHIShaderPlatform);
  if(!TestNotNull(TEXT("Compiled material resource exists"),Resource))return false;
  TestEqual(TEXT("No decal shader compile errors"),Resource->GetCompileErrors().Num(),0);
 }
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FSpatialImpactCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
