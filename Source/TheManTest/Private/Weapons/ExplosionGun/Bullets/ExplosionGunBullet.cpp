#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "TimerManager.h"

AExplosionGunBullet::AExplosionGunBullet()
{
 bDestroyOnHit=false;
 ExplosionCueTag=TAG_GameplayCue_Weapon_ExplosionGun_Explosion;
}
void AExplosionGunBullet::ProcessHit_Implementation(const FHitResult& Hit,AActor* Shooter,UAbilitySystemComponent* Source)
{
 if(bAttached || HasProcessedHit())return;
 // The base path handles pass-through, exactly-once direct damage and the existing impact Cue.
 Super::ProcessHit_Implementation(Hit,Shooter,Source);
 if(!HasProcessedHit() || IsActorBeingDestroyed())return;
 bAttached=true;ExplosionSourceASC=Source;ExplosionInstigator=Shooter;
 ProjectileMovement->StopMovementImmediately();ProjectileMovement->Deactivate();ProjectileMovement->SetComponentTickEnabled(false);
 CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 SetLifeSpan(0.f);
 FHitResult Surface=Hit;
 UPrimitiveComponent* Parent=Hit.GetComponent();
 // Capsule hits still deal the original damage; only refine the visual attachment onto the animated mesh.
 if(ACharacter* Character=Cast<ACharacter>(Hit.GetActor());IsValid(Character) && Character->GetMesh())
 {
  FHitResult MeshHit;
  const FVector Direction=GetActorForwardVector();
  FCollisionQueryParams Query(SCENE_QUERY_STAT(StickyBulletAttachment),true);
  if(Character->GetMesh()->LineTraceComponent(MeshHit,Hit.ImpactPoint-Direction*40.f,Hit.ImpactPoint+Direction*120.f,Query))
  { Surface=MeshHit;Parent=Character->GetMesh(); }
 }
 const FVector Normal=Surface.ImpactNormal.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector);
 SetActorLocation(Surface.ImpactPoint+Normal*AttachmentOffset,false,nullptr,ETeleportType::TeleportPhysics);
 if(IsValid(Parent) && IsValid(Parent->GetOwner()) && !Parent->GetOwner()->IsActorBeingDestroyed())
  AttachToComponent(Parent,FAttachmentTransformRules::KeepWorldTransform,Surface.BoneName);
 LocalImpactPoint=GetActorTransform().InverseTransformPosition(Surface.ImpactPoint);
 LocalImpactNormal=GetActorQuat().UnrotateVector(Normal);
 // Even a zero delay goes through the next tick, never re-enters the collision callback.
 if(ExplosionDelay<=0.f)ExplosionTimer=GetWorldTimerManager().SetTimerForNextTick(this,&AExplosionGunBullet::Detonate);
 else GetWorldTimerManager().SetTimer(ExplosionTimer,this,&AExplosionGunBullet::Detonate,ExplosionDelay,false);
}
float AExplosionGunBullet::GetRemainingExplosionTime() const
{
 return bAttached&&!bDetonated ? FMath::Max(0.f,GetWorldTimerManager().GetTimerRemaining(ExplosionTimer)) : 0.f;
}
void AExplosionGunBullet::Detonate()
{
 if(!bAttached||bDetonated)return;
 bDetonated=true;
 FGameplayCueParameters Params;
 Params.Location=GetActorTransform().TransformPosition(LocalImpactPoint);
 Params.Normal=GetActorQuat().RotateVector(LocalImpactNormal);
 Params.Instigator=ExplosionInstigator.Get();Params.EffectCauser=this;
 if(ExplosionCueTag.IsValid())
 {
  if(UAbilitySystemComponent* ASC=ExplosionSourceASC.Get())ASC->InvokeGameplayCueEvent(ExplosionCueTag,EGameplayCueEvent::Executed,Params);
  else UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(this,ExplosionCueTag,EGameplayCueEvent::Executed,Params);
 }
 // No radial damage: this milestone intentionally retains only the existing first-hit damage.
 Destroy();
}
void AExplosionGunBullet::EndPlay(const EEndPlayReason::Type Reason)
{
 GetWorldTimerManager().ClearTimer(ExplosionTimer);
 Super::EndPlay(Reason);
}
