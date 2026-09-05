#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
UGCN_ExplosionGunExplosion::UGCN_ExplosionGunExplosion(){GameplayCueTag=TAG_GameplayCue_Weapon_ExplosionGun_Explosion;}
bool UGCN_ExplosionGunExplosion::OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& P) const
{
 if(!Target||!Target->GetWorld())return false;
 // N_ExplosionGround_006 is authored along +Z, unlike directional muzzle/impact systems.
 const FVector Normal=P.Normal.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector);
 if(ExplosionEffect)
 {
  if(UNiagaraComponent* Effect=UNiagaraFunctionLibrary::SpawnSystemAtLocation(Target,ExplosionEffect,P.Location,FRotationMatrix::MakeFromZ(Normal).Rotator(),FVector(EffectScale)))
  {
   // The source ground effect has a long tail. Bound its lifetime independently of the projectile.
   FTimerHandle Cleanup;
   Target->GetWorld()->GetTimerManager().SetTimer(Cleanup,FTimerDelegate::CreateWeakLambda(Effect,[Effect](){Effect->DestroyComponent();}),FMath::Max(.1f,EffectLifeSpan),false);
  }
 }
 if(ExplosionSound)UGameplayStatics::PlaySoundAtLocation(Target,ExplosionSound,P.Location,VolumeMultiplier);
 return ExplosionEffect||ExplosionSound;
}
