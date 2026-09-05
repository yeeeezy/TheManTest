#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"
#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Sound/SoundBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplosionDirectionalShakeTest,"TheManTest.Player.Weapons.ExplosionDirectionalShake",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplosionDirectionalShakeTest::RunTest(const FString& Parameters)
{
 UClass* CueClass=LoadClass<UGCN_ExplosionGunExplosion>(nullptr,TEXT("/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion.GC_Weapon_ExplosionGun_Explosion_C"));
 if(!TestNotNull(TEXT("Explosion Cue loads"),CueClass))return false;
 const auto* Cue=CueClass->GetDefaultObject<UGCN_ExplosionGunExplosion>();
 TestEqual(TEXT("Supplied explosion uses user-requested threefold gain"),Cue->VolumeMultiplier,3.f);
 TestEqual(TEXT("Supplied alien explosion Sound Cue is assigned"),Cue->ExplosionSound ? Cue->ExplosionSound->GetName() : FString(),FString(TEXT("SCue_ExplosionGun_Detonation")));
 UClass* EnemyCueClass=LoadClass<UGCN_EnemyHit>(nullptr,TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"));
 if(TestNotNull(TEXT("Enemy Hit Cue loads"),EnemyCueClass))
 {
  const auto* EnemyCue=EnemyCueClass->GetDefaultObject<UGCN_EnemyHit>();
  TestEqual(TEXT("Supplied flesh hit Sound Cue is assigned"),EnemyCue->ImpactSound ? EnemyCue->ImpactSound->GetName() : FString(),FString(TEXT("SCue_Enemy_FleshHit")));
 }
 TestNotNull(TEXT("Explosion has its own camera shake"),Cue->CameraShakeClass.Get());
 TestEqual(TEXT("Configured shake inside inner radius"),Cue->GetShakeScaleAtDistance(100),Cue->CameraShakeScale);
 TestEqual(TEXT("Configured shake with distance falloff at midpoint"),Cue->GetShakeScaleAtDistance(1000),Cue->CameraShakeScale*.25f);
 TestEqual(TEXT("No shake beyond outer radius"),Cue->GetShakeScaleAtDistance(2000),0.f);
 auto Sample=[](const FRotator& Direction,float Scale)
 {
  UExplosionCameraShake* Shake=NewObject<UExplosionCameraShake>();
  Shake->StartShake(nullptr,Scale,ECameraShakePlaySpace::UserDefined,Direction);
  FMinimalViewInfo View;View.Location=FVector::ZeroVector;View.Rotation=FRotator::ZeroRotator;
  Shake->UpdateAndApplyCameraShake(.05f,1.f,View);
  Shake->StopShake(true);Shake->TeardownShake();return View;
 };
 const auto Left=Sample(FRotator(0,-90,0),1.f);
 const auto Right=Sample(FRotator(0,90,0),1.f);
 TestTrue(TEXT("Explosion never displaces camera"),Left.Location.IsNearlyZero()&&Right.Location.IsNearlyZero());
 TestTrue(TEXT("Opposite blast directions change rotation"),!Left.Rotation.Equals(Right.Rotation,.01f));
 TestTrue(TEXT("Blast produces camera rotation"),!Right.Rotation.IsNearlyZero());
 const auto Far=Sample(FRotator(0,90,0),.25f);
 TestTrue(TEXT("Far blast is proportionally weaker"),FMath::IsNearlyEqual(Far.Rotation.Roll,Right.Rotation.Roll*.25,.02));
 auto* MutableCue=NewObject<UGCN_ExplosionGunExplosion>();
 MutableCue->ExplosionSound=Cue->ExplosionSound;
 TestNull(TEXT("Enemy sound never falls back to environment"),MutableCue->GetExplosionSound(true));
 MutableCue->EnemyExplosionSound=GetDefault<UGCN_EnemyHit>()->ImpactSound;
 TestEqual(TEXT("Environment selection is independent"),MutableCue->GetExplosionSound(false),Cue->ExplosionSound.Get());
 UExplosionCameraShake* Shake=NewObject<UExplosionCameraShake>();
 Shake->StartShake(nullptr,1.f,ECameraShakePlaySpace::CameraLocal);
 FMinimalViewInfo View;
 int Crossings=0;float Previous=0;
 for(int i=0;i<40;++i)
 {
  View.Location=FVector::ZeroVector;View.Rotation=FRotator::ZeroRotator;
  Shake->UpdateAndApplyCameraShake(.02f,1.f,View);
  if(Previous*View.Rotation.Pitch<0)++Crossings;
  Previous=View.Rotation.Pitch;
  TestTrue(TEXT("No translation throughout aftershock"),View.Location.IsNearlyZero());
 }
 TestTrue(TEXT("Multiple oscillations, not one kick"),Crossings>=6);
 TestTrue(TEXT("Short explosion shake finishes automatically"),Shake->IsFinished());
 Shake->TeardownShake();return true;
}
#endif
