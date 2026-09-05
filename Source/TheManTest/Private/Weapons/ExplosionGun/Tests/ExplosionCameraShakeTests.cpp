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
 TestEqual(TEXT("Near-full-scale supplied audio uses unity gain"),Cue->VolumeMultiplier,1.f);
 TestEqual(TEXT("Supplied alien explosion audio is assigned"),Cue->ExplosionSound ? Cue->ExplosionSound->GetName() : FString(),FString(TEXT("S_ExplosionGun_AlienDetonation")));
 UClass* EnemyCueClass=LoadClass<UGCN_EnemyHit>(nullptr,TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"));
 if(TestNotNull(TEXT("Enemy Hit Cue loads"),EnemyCueClass))
 {
  const auto* EnemyCue=EnemyCueClass->GetDefaultObject<UGCN_EnemyHit>();
  TestEqual(TEXT("Supplied flesh hit sound is assigned"),EnemyCue->ImpactSound ? EnemyCue->ImpactSound->GetName() : FString(),FString(TEXT("S_Enemy_FleshHit")));
 }
 TestNotNull(TEXT("Explosion has its own camera shake"),Cue->CameraShakeClass.Get());
 TestEqual(TEXT("Tripled shake inside inner radius"),Cue->GetShakeScaleAtDistance(100),3.f);
 TestEqual(TEXT("Tripled shake with distance falloff at midpoint"),Cue->GetShakeScaleAtDistance(1000),.75f);
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
 TestTrue(TEXT("Opposite blast sides produce opposite lateral offsets"),Left.Location.Y<0&&Right.Location.Y>0);
 TestTrue(TEXT("Blast produces camera rotation"),!Right.Rotation.IsNearlyZero());
 const auto Far=Sample(FRotator(0,90,0),.25f);
 TestTrue(TEXT("Far blast is proportionally weaker"),FMath::IsNearlyEqual(Far.Location.Y,Right.Location.Y*.25,.01));
 UExplosionCameraShake* Shake=NewObject<UExplosionCameraShake>();
 Shake->StartShake(nullptr,1.f,ECameraShakePlaySpace::CameraLocal);
 FMinimalViewInfo View;
 for(int i=0;i<40;++i)Shake->UpdateAndApplyCameraShake(.02f,1.f,View);
 TestTrue(TEXT("Short explosion shake finishes automatically"),Shake->IsFinished());
 Shake->TeardownShake();return true;
}
#endif
