#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "GameFramework/PlayerController.h"
UGCN_ExplosionGunExplosion::UGCN_ExplosionGunExplosion()
{
 GameplayCueTag=TAG_GameplayCue_Weapon_ExplosionGun_Explosion;
 CameraShakeClass=UExplosionCameraShake::StaticClass();
}
float UGCN_ExplosionGunExplosion::GetShakeScaleAtDistance(float Distance) const
{
 const float Inner=FMath::Max(0.f,ShakeInnerRadius);
 const float Outer=FMath::Max(Inner+1.f,ShakeOuterRadius);
 return CameraShakeScale*FMath::Square(1.f-FMath::Clamp((Distance-Inner)/(Outer-Inner),0.f,1.f));
}
bool UGCN_ExplosionGunExplosion::OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& P) const
{
 if(!Target||!Target->GetWorld())return false;
 // N_ExplosionGround_006 is authored along +Z, unlike directional muzzle/impact systems.
 const FHitResult* Ground=P.EffectContext.GetHitResult();
 const bool bEnemy=P.AggregatedTargetTags.HasTagExact(TAG_Data_Explosion_EnemyImpact);
 UNiagaraSystem* SelectedEffect=bEnemy?EnemyExplosionEffect.Get():ExplosionEffect.Get();
 const FVector Normal=bEnemy?FVector(P.Normal).GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector):Ground?Ground->ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector):FVector::UpVector;
 if(SelectedEffect && (bEnemy || Ground))
 {
  const FVector Point=bEnemy?FVector(P.Location):Ground->ImpactPoint+Normal;
  if(UNiagaraComponent* Effect=UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target,SelectedEffect,Point,FRotationMatrix::MakeFromZ(Normal).Rotator(),FVector(bEnemy?EnemyEffectScale:EffectScale)))
  {
   // The source ground effect has a long tail. Bound its lifetime independently of the projectile.
   FTimerHandle Cleanup;
   Target->GetWorld()->GetTimerManager().SetTimer(Cleanup,FTimerDelegate::CreateWeakLambda(Effect,[Effect](){Effect->DestroyComponent();}),FMath::Max(.1f,EffectLifeSpan),false);
  }
 }
 USoundBase* SelectedSound=GetExplosionSound(bEnemy);
 if(SelectedSound)UGameplayStatics::PlaySoundAtLocation(Target,SelectedSound,P.Location,bEnemy?EnemyVolumeMultiplier:VolumeMultiplier);
 if(CameraShakeClass)
 {
  for(FConstPlayerControllerIterator It=Target->GetWorld()->GetPlayerControllerIterator();It;++It)
  {
   APlayerController* PC=It->Get();
   if(!PC||!PC->IsLocalController()||!PC->PlayerCameraManager)continue;
   APlayerCameraManager* Camera=PC->PlayerCameraManager;
   const FVector Away=Camera->GetCameraLocation()-FVector(P.Location);
   const float Strength=GetShakeScaleAtDistance(Away.Size());
   if(Strength>UE_KINDA_SMALL_NUMBER)
   {
    // Explicitly replace this explosion class; custom initializers otherwise allow stacking.
    Camera->StopAllInstancesOfCameraShake(CameraShakeClass,true);
    FAddCameraShakeParams ShakeParams(FMath::Min(Strength,8.f),ECameraShakePlaySpace::UserDefined,
     Away.IsNearlyZero()?Camera->GetCameraRotation():Away.Rotation());
    ShakeParams.Initializer=FOnInitializeCameraShake::CreateLambda([this](UCameraShakeBase* Shake)
    {
     if(auto* Pattern=Cast<UExplosionCameraShakePattern>(Shake->GetRootShakePattern()))
     {
      Pattern->Duration=FMath::Clamp(ShakeDuration,.1f,2.f);
      Pattern->RotationDegrees=FMath::Clamp(ShakeRotationDegrees,0.f,3.f);
      Pattern->Frequency=FMath::Clamp(ShakeFrequency,1.f,30.f);
     }
    });
    Camera->StartCameraShake(CameraShakeClass,ShakeParams);
   }
  }
 }
 return SelectedEffect||SelectedSound||CameraShakeClass;
}
