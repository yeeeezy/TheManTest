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
#include "Enemy/EnemyBase.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Field/FieldSystemObjects.h"
#include "Engine/OverlapResult.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "GameplayEffect.h"
#include "Components/CapsuleComponent.h"

const FName AExplosionGunBullet::ExplosionGroundTag(TEXT("ExplosionGround"));

AExplosionGunBullet::AExplosionGunBullet()
{
 bDestroyOnHit=false;
 CollisionSphere->SetCollisionResponseToChannel(ECC_Destructible,ECR_Block);
 ExplosionCueTag=TAG_GameplayCue_Weapon_ExplosionGun_Explosion;
 ExplosionDamageEffectClass=HitEffectClass;
}
void AExplosionGunBullet::ProcessHit_Implementation(const FHitResult& Hit,AActor* Shooter,UAbilitySystemComponent* Source)
{
 if(bAttached || HasProcessedHit())return;
 const bool bEnemyImpact=IsValid(Hit.GetActor()) && Hit.GetActor()->IsA<AEnemyBase>();
 // The base path handles pass-through, exactly-once direct damage and the existing impact Cue.
 Super::ProcessHit_Implementation(Hit,Shooter,Source);
 if(!HasProcessedHit() || IsActorBeingDestroyed())return;
 bHitEnemy=bEnemyImpact;
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
 if(bHitEnemy)Params.AggregatedTargetTags.AddTag(TAG_Data_Explosion_EnemyImpact);
 // Location remains the actual attached blast origin for sound, shake and physics.
 // Only the context hit is projected onto explicitly marked ground for the ground Niagara/decal.
 FHitResult GroundHit;
 if(FindExplosionGround(Params.Location,GroundHit))
 {
  Params.EffectContext=FGameplayEffectContextHandle(new FGameplayEffectContext());
  Params.EffectContext.AddHitResult(GroundHit);
 }
 // Resolve visibility before Chaos opens holes in the blocking geometry.
 ApplyExplosionDamage(FVector(Params.Location)+FVector(Params.Normal)*2.f);
 TriggerChaos(Params.Location);
 if(ExplosionCueTag.IsValid())
 {
  if(UAbilitySystemComponent* ASC=ExplosionSourceASC.Get())ASC->InvokeGameplayCueEvent(ExplosionCueTag,EGameplayCueEvent::Executed,Params);
  else UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(this,ExplosionCueTag,EGameplayCueEvent::Executed,Params);
 }
 if(auto* Feedback=GetWorld()->GetSubsystem<UBulletTimeSubsystem>())
  Feedback->RequestBulletTimeAtLocation(Params.Location,BulletTime);
 Destroy();
}
void AExplosionGunBullet::ApplyExplosionDamage(const FVector& Origin)
{
 if(!GetWorld()||ExplosionDamage<=0.f||ExplosionDamageRadius<=0.f||!ExplosionDamageEffectClass)return;
 TArray<FOverlapResult> Overlaps;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(ExplosionDamage),false,this);
 GetWorld()->OverlapMultiByObjectType(Overlaps,Origin,FQuat::Identity,
  FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),FCollisionShape::MakeSphere(ExplosionDamageRadius),Query);
 TSet<TWeakObjectPtr<AEnemyBase>> Candidates;
 for(const auto& Overlap:Overlaps)
  if(auto* Enemy=Cast<AEnemyBase>(Overlap.GetActor());IsValid(Enemy))Candidates.Add(Enemy);
 TArray<TWeakObjectPtr<AEnemyBase>> Visible;
 for(const auto& Candidate:Candidates)
 {
  auto* Enemy=Candidate.Get();
  if(!Enemy||Enemy->IsActorBeingDestroyed())continue;
  auto* ASC=Enemy->GetAbilitySystemComponent();
  if(!ASC||ASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute())<=0.f)continue;
  FCollisionQueryParams Sight(SCENE_QUERY_STAT(ExplosionDamageVisibility),true,this);
  // Enemies do not act as walls shielding other enemies in the same blast.
  for(const auto& Other:Candidates)if(Other.IsValid())Sight.AddIgnoredActor(Other.Get());
  if(ExplosionInstigator.IsValid())Sight.AddIgnoredActor(ExplosionInstigator.Get());
  FHitResult Block;
  if(!GetWorld()->LineTraceSingleByChannel(Block,Origin,Enemy->GetActorLocation(),ECC_Visibility,Sight))Visible.Add(Enemy);
 }
 for(const auto& Candidate:Visible)
 {
  auto* Enemy=Candidate.Get();
  if(!IsValid(Enemy)||Enemy->IsActorBeingDestroyed())continue;
  auto* TargetASC=Enemy->GetAbilitySystemComponent();
  auto* SourceASC=ExplosionSourceASC.IsValid()?ExplosionSourceASC.Get():TargetASC;
  if(!TargetASC||!SourceASC)continue;
  FGameplayEffectContextHandle Context=SourceASC->MakeEffectContext();
  Context.AddInstigator(ExplosionInstigator.Get(),this);
  const FVector Point=Enemy->GetActorLocation();
  FHitResult Hit(Enemy,Enemy->GetCapsuleComponent(),Point,(Point-Origin).GetSafeNormal());
  Hit.bBlockingHit=true;
  Context.AddHitResult(Hit,true);
  auto Spec=SourceASC->MakeOutgoingSpec(ExplosionDamageEffectClass,1.f,Context);
  if(Spec.IsValid())
  {
   Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage,-ExplosionDamage);
   SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),TargetASC);
  }
 }
}
bool AExplosionGunBullet::FindExplosionGround(const FVector& Origin,FHitResult& OutHit) const
{
 OutHit=FHitResult();
 if(!GetWorld() || GroundSearchDistance<=0.f)return false;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(ExplosionGround),true,this);
 FCollisionObjectQueryParams Objects(FCollisionObjectQueryParams::AllObjects);
 TArray<FHitResult> Hits;
 // Object multi traces collect through blocking cubes instead of stopping at their top face.
 GetWorld()->LineTraceMultiByObjectType(Hits,Origin+FVector(0,0,2),Origin-FVector(0,0,GroundSearchDistance),Objects,Query);
 const float MinUp=FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(GroundMaxSlope,0.f,89.f)));
 for(const FHitResult& Hit:Hits)
 {
  AActor* Actor=Hit.GetActor();
  if(!IsValid(Actor)||Actor->IsA<AEnemyBase>()||Actor->IsA<ABulletBase>()||Cast<UGeometryCollectionComponent>(Hit.GetComponent()))continue;
  if(Actor->ActorHasTag(ExplosionGroundTag) && Hit.ImpactNormal.Z>=MinUp)
  { OutHit=Hit;return true; }
 }
 return false;
}
void AExplosionGunBullet::TriggerChaos(const FVector& Origin)
{
 if(ChaosRadius<=0.f || !GetWorld())return;
 TArray<FOverlapResult> Overlaps;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(ExplosionChaos),false,this);
 GetWorld()->OverlapMultiByObjectType(Overlaps,Origin,FQuat::Identity,
  FCollisionObjectQueryParams(FCollisionObjectQueryParams::AllObjects),FCollisionShape::MakeSphere(ChaosRadius),Query);
 TSet<UGeometryCollectionComponent*> Applied;
 for(const FOverlapResult& Overlap:Overlaps)
 {
  auto* Collection=Cast<UGeometryCollectionComponent>(Overlap.GetComponent());
  if(!IsValid(Collection)||!IsValid(Collection->GetOwner())||Collection->GetOwner()->IsA<AEnemyBase>()||Applied.Contains(Collection))continue;
  Applied.Add(Collection);
  auto* Strain=NewObject<URadialFalloff>(Collection);
  Strain->SetRadialFalloff(ChaosStrain,1.f,1.f,0.f,ChaosRadius,Origin,EFieldFalloffType::Field_FallOff_None);
  Collection->ApplyPhysicsField(true,EGeometryCollectionPhysicsTypeEnum::Chaos_ExternalClusterStrain,nullptr,Strain);
  // Delay impulse one physics step so the newly released pieces receive individual outward kicks.
  const TWeakObjectPtr<UGeometryCollectionComponent> WeakCollection(Collection);
  const float Radius=ChaosRadius,Strength=ChaosImpulse,Spin=ChaosAngularSpeed;
  FTimerHandle ImpulseTimer;
  GetWorldTimerManager().SetTimer(ImpulseTimer,FTimerDelegate::CreateLambda([WeakCollection,Origin,Radius,Strength,Spin]()
  {
   if(auto* GC=WeakCollection.Get())
   {
    GC->AddRadialImpulse(Origin,Radius,Strength,ERadialImpulseFalloff::RIF_Linear,true);
    if(Spin>0.f)
    {
     auto* Mask=NewObject<URadialFalloff>(GC);
     Mask->SetRadialFalloff(1.f,1.f,1.f,0.f,Radius,Origin,EFieldFalloffType::Field_FallOff_None);
     auto* RandomSpin=NewObject<URandomVector>(GC);
     RandomSpin->SetRandomVector(Spin);
     auto* CulledSpin=NewObject<UCullingField>(GC);
     CulledSpin->SetCullingField(Mask,RandomSpin,EFieldCullingOperationType::Field_Culling_Outside);
     GC->ApplyPhysicsField(true,EGeometryCollectionPhysicsTypeEnum::Chaos_AngularVelocity,nullptr,CulledSpin);
    }
   }
  }),.05f,false);
 }
}
void AExplosionGunBullet::EndPlay(const EEndPlayReason::Type Reason)
{
 GetWorldTimerManager().ClearTimer(ExplosionTimer);
 Super::EndPlay(Reason);
}
