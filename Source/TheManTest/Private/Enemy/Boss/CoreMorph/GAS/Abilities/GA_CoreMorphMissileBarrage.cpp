#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphMissileBarrage.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphAttacking.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphMissileCooldown.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphMissileDamage.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Components/PrimitiveComponent.h"
UGA_CoreMorphMissileBarrage::UGA_CoreMorphMissileBarrage()
{
 InstancingPolicy=EGameplayAbilityInstancingPolicy::InstancedPerActor;NetExecutionPolicy=EGameplayAbilityNetExecutionPolicy::ServerOnly;
 ActivationRequiredTags.AddTag(TAG_State_CoreMorph_Form_Manta);ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Transforming);ActivationBlockedTags.AddTag(TAG_State_CoreMorph_Attacking);ActivationBlockedTags.AddTag(TAG_State_CoreMorph_MissileCooldown);
}
bool UGA_CoreMorphMissileBarrage::CanActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayTagContainer* S,const FGameplayTagContainer* T,FGameplayTagContainer* R) const
{auto* B=I?Cast<ACoreMorphBoss>(I->AvatarActor.Get()):nullptr;return B && B->MissileCombat->CanFire() && Super::CanActivateAbility(H,I,S,T,R);}
void UGA_CoreMorphMissileBarrage::ActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,const FGameplayEventData* E)
{
 Super::ActivateAbility(H,I,A,E);bStarted=false;auto* B=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());Combat=B?B->MissileCombat.Get():nullptr;
 if(!Combat.IsValid() || !CommitAbility(H,I,A) || !Combat->StartSalvo()){EndAbility(H,I,A,true,true);return;}
 bStarted=true;FinishedHandle=Combat->OnFinished.AddUObject(this,&ThisClass::Finish);InvalidHandle=Combat->OnInvalidated.AddUObject(this,&ThisClass::CancelSalvo);ImpactHandle=Combat->OnImpact.AddUObject(this,&ThisClass::ApplyImpact);
 auto* ASC=GetAbilitySystemComponentFromActorInfo();AttackEffect=ASC->ApplyGameplayEffectToSelf(GetDefault<UGE_CoreMorphAttacking>(),1,ASC->MakeEffectContext());
 FGameplayCueParameters P;P.SourceObject=B;P.Location=Combat->GetCoreLocation();ASC->AddGameplayCue(TAG_GameplayCue_CoreMorph_MissileBarrage,P);
}
void UGA_CoreMorphMissileBarrage::Finish(){EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);}
void UGA_CoreMorphMissileBarrage::CancelSalvo(){CancelAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true);}
void UGA_CoreMorphMissileBarrage::ApplyImpact(int32 Index)
{
 if(!IsActive() || !Combat.IsValid() || !Combat->GetMissiles().IsValidIndex(Index))return;
 const auto Missile=Combat->GetMissiles()[Index];auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());auto* ASC=GetAbilitySystemComponentFromActorInfo();
 const FVector Center=Missile.Ground,Normal=Missile.Normal;const float Radius=Combat->GetLockedRadius(),Damage=Missile.Damage;
 TArray<FOverlapResult> Hits;FCollisionQueryParams Q(SCENE_QUERY_STAT(CoreMorphMissileDamage),false,Boss);
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
  FHitResult Cover;FCollisionQueryParams Visibility(SCENE_QUERY_STAT(CoreMorphMissileCover),false,Boss);Visibility.AddIgnoredActor(Victim);
  if(Boss->GetWorld()->LineTraceSingleByObjectType(Cover,Center+Normal*60,Point+Normal*5,FCollisionObjectQueryParams(ECC_WorldStatic),Visibility))continue;
  auto Context=ASC->MakeEffectContext();FHitResult TargetHit(Victim,Surface,Point,(Point-Center).GetSafeNormal());Context.AddHitResult(TargetHit);Context.AddSourceObject(Boss);
  auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphMissileDamage::StaticClass(),1,Context);
  if(Spec.IsValid()){Damaged.Add(TargetASC);Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage,-Damage);ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),TargetASC);}
 }
}
void UGA_CoreMorphMissileBarrage::EndAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,bool Replicate,bool Cancelled)
{
 auto* ASC=GetAbilitySystemComponentFromActorInfo();auto* B=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());
 if(ASC){ASC->RemoveGameplayCue(TAG_GameplayCue_CoreMorph_MissileBarrage);ASC->RemoveActiveGameplayEffect(AttackEffect);}
 if(Combat.IsValid()){Combat->OnFinished.Remove(FinishedHandle);Combat->OnInvalidated.Remove(InvalidHandle);Combat->OnImpact.Remove(ImpactHandle);Combat->StopSalvo();}
 if(ASC && B && !B->IsDead() && bStarted){auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphMissileCooldown::StaticClass(),1,ASC->MakeEffectContext());if(Spec.IsValid()){Spec.Data->SetDuration(FMath::Max(.1f,B->MissileCombat->CooldownDuration),true);ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());}}
 bStarted=false;Combat.Reset();AttackEffect.Invalidate();FinishedHandle.Reset();InvalidHandle.Reset();ImpactHandle.Reset();Super::EndAbility(H,I,A,Replicate,Cancelled);
}
