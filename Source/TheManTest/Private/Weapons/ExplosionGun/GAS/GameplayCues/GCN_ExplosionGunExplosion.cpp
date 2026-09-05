#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"
#include "Camera/PlayerCameraManager.h"
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
 const FVector Normal=Ground?Ground->ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector):FVector::UpVector;
 if(ExplosionEffect && Ground)
 {
  if(UNiagaraComponent* Effect=UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target,ExplosionEffect,Ground->ImpactPoint+Normal,FRotationMatrix::MakeFromZ(Normal).Rotator(),FVector(EffectScale)))
  {
   // The source ground effect has a long tail. Bound its lifetime independently of the projectile.
   FTimerHandle Cleanup;
   Target->GetWorld()->GetTimerManager().SetTimer(Cleanup,FTimerDelegate::CreateWeakLambda(Effect,[Effect](){Effect->DestroyComponent();}),FMath::Max(.1f,EffectLifeSpan),false);
  }
 }
 if(ExplosionSound)UGameplayStatics::PlaySoundAtLocation(Target,ExplosionSound,P.Location,VolumeMultiplier);
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
    Camera->StartCameraShake(CameraShakeClass,Strength,ECameraShakePlaySpace::UserDefined,
     Away.IsNearlyZero()?Camera->GetCameraRotation():Away.Rotation());
  }
 }
 return ExplosionEffect||ExplosionSound;
}
