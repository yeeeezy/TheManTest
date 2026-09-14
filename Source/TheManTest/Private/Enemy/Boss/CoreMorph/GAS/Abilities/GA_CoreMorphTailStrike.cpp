#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphAttacking.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailCooldown.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailDamage.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
UGA_CoreMorphTailStrike::UGA_CoreMorphTailStrike()
{
 InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
 ActivationRequiredTags.AddTag(TAG_State_CoreMorph_Form_Scorpion);
 ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Transforming);ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Attacking);ActivationBlockedTags.AddTag(TAG_State_CoreMorph_TailCooldown);
}
bool UGA_CoreMorphTailStrike::CanActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayTagContainer* S,const FGameplayTagContainer* T,FGameplayTagContainer* R) const
{
 auto* B=I?Cast<ACoreMorphBoss>(I->AvatarActor.Get()):nullptr;
 return B && B->ScorpionCombat->CanStrike() && Super::CanActivateAbility(H,I,S,T,R);
}
void UGA_CoreMorphTailStrike::ActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,const FGameplayEventData* E)
{
 Super::ActivateAbility(H,I,A,E);auto* B=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());Combat=B?B->ScorpionCombat.Get():nullptr;
 if(!Combat.IsValid() || !CommitAbility(H,I,A) || !Combat->StartAction(ECoreMorphScorpionAction::Windup)){EndAbility(H,I,A,true,true);return;}
 auto* ASC=GetAbilitySystemComponentFromActorInfo();AttackEffect=ASC->ApplyGameplayEffectToSelf(GetDefault<UGE_CoreMorphAttacking>(),1,ASC->MakeEffectContext());
 StageHandle=Combat->OnStrikeStageFinished.AddUObject(this,&ThisClass::AdvanceStage);
 InvalidHandle=Combat->OnStrikeInvalidated.AddUObject(this,&ThisClass::CancelStrike);
 ContactHandle=Combat->OnStrikeContact.AddUObject(this,&ThisClass::ApplyContact);
 bDetonated=false;FGameplayCueParameters P;P.SourceObject=B;P.Location=Combat->LockedGroundHit.ImpactPoint;P.Normal=Combat->LockedGroundHit.ImpactNormal;P.RawMagnitude=Combat->LockedBlastRadius;
 ASC->AddGameplayCue(TAG_GameplayCue_CoreMorph_TailCharge,P);
}
void UGA_CoreMorphTailStrike::AdvanceStage()
{
 if(!Combat.IsValid() || !Combat->IsReady()){CancelStrike();return;}
 if(Combat->Action==ECoreMorphScorpionAction::Windup)Combat->StartAction(ECoreMorphScorpionAction::Thrust);
 else if(Combat->Action==ECoreMorphScorpionAction::Thrust){GetAbilitySystemComponentFromActorInfo()->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailCharge);Combat->StartAction(ECoreMorphScorpionAction::Recover);}
 else EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}
void UGA_CoreMorphTailStrike::CancelStrike(){CancelAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true);}
void UGA_CoreMorphTailStrike::ApplyContact(const FHitResult& Hit)
{
 if(!Combat.IsValid() || !IsActive() || bDetonated || Combat->Action!=ECoreMorphScorpionAction::Thrust)return;
 bDetonated=true;auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());auto* ASC=GetAbilitySystemComponentFromActorInfo();
 const FVector Center=Combat->LockedGroundHit.ImpactPoint,Normal=Combat->LockedGroundHit.ImpactNormal;
 const float Radius=Combat->LockedBlastRadius;
 // Snapshot the current strength once at impact for every victim of this blast.
 const float Damage=FMath::Max(0.f,Combat->StrikeDamage)*FMath::Max(0.f,Boss->GetDamageMultiplier());
 ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailCharge);
 FGameplayCueParameters P;P.SourceObject=Boss;P.Location=Center;P.Normal=Normal;P.RawMagnitude=Radius;ASC->AddGameplayCue(TAG_GameplayCue_CoreMorph_TailBlast,P);
 // The same locked center/radius drives the telegraph and this one-shot sphere query.
 TArray<FOverlapResult> Hits;FCollisionQueryParams Q(SCENE_QUERY_STAT(CoreMorphThunderDamage),false,Boss);
 FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_Pawn);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
 Boss->GetWorld()->OverlapMultiByObjectType(Hits,Center,FQuat::Identity,Objects,FCollisionShape::MakeSphere(Radius),Q);
 TSet<UAbilitySystemComponent*> Damaged;
 for(const auto& Overlap:Hits)
 {
  AActor* Victim=Overlap.GetActor();auto* Surface=Overlap.GetComponent();auto* TargetASC=UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Victim);
  if(!Surface || !TargetASC || TargetASC==ASC || Damaged.Contains(TargetASC))continue;
  if(auto* Enemy=Cast<AEnemyBase>(Victim))if(Enemy->IsDead())continue;
  FVector Point;const float Distance=Surface->GetClosestPointOnCollision(Center,Point);
  if(Distance<0)Point=Surface->Bounds.GetBox().GetClosestPointTo(Center);
  if(FVector::DistSquared(Point,Center)>FMath::Square(Radius))continue;
  // Static cover blocks damage; another piece of the same target may still be exposed.
  FHitResult Cover;FCollisionQueryParams Visibility(SCENE_QUERY_STAT(CoreMorphThunderCover),false,Boss);Visibility.AddIgnoredActor(Victim);
  if(Boss->GetWorld()->LineTraceSingleByObjectType(Cover,Center+Normal*60,Point+Normal*5,FCollisionObjectQueryParams(ECC_WorldStatic),Visibility))continue;
  auto Context=ASC->MakeEffectContext();FHitResult TargetHit(Victim,Surface,Point,(Point-Center).GetSafeNormal());Context.AddHitResult(TargetHit);Context.AddSourceObject(Boss);
  auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphTailDamage::StaticClass(),1,Context);
  if(Spec.IsValid()){Damaged.Add(TargetASC);Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage,-Damage);ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),TargetASC);}
 }
}

void UGA_CoreMorphTailStrike::EndAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,bool Replicate,bool Cancelled)
{
 if(Combat.IsValid())
 {
  Combat->OnStrikeStageFinished.Remove(StageHandle);Combat->OnStrikeInvalidated.Remove(InvalidHandle);Combat->OnStrikeContact.Remove(ContactHandle);
  if(Cancelled)Combat->ResetStrike();else Combat->StartAction(ECoreMorphScorpionAction::Idle);
 }
 auto* ASC=GetAbilitySystemComponentFromActorInfo();auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());
 if(ASC){ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailCharge);if(Cancelled)ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_TailBlast);ASC->RemoveActiveGameplayEffect(AttackEffect);if(Boss && !Boss->IsDead())
 {auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphTailCooldown::StaticClass(),1,ASC->MakeEffectContext());if(Spec.IsValid()){Spec.Data->SetDuration(FMath::Max(.05f,Boss->ScorpionCombat->CooldownDuration),true);ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());}}}
 Combat.Reset();AttackEffect.Invalidate();StageHandle.Reset();InvalidHandle.Reset();ContactHandle.Reset();
 Super::EndAbility(H,I,A,Replicate,Cancelled);
}
