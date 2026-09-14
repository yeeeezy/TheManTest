#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphAttacking.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailCooldown.h"
#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailDamage.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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
}
void UGA_CoreMorphTailStrike::AdvanceStage()
{
 if(!Combat.IsValid() || !Combat->IsReady()){CancelStrike();return;}
 if(Combat->Action==ECoreMorphScorpionAction::Windup)Combat->StartAction(ECoreMorphScorpionAction::Thrust);
 else if(Combat->Action==ECoreMorphScorpionAction::Thrust)Combat->StartAction(ECoreMorphScorpionAction::Recover);
 else EndAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true,false);
}
void UGA_CoreMorphTailStrike::CancelStrike(){CancelAbility(CurrentSpecHandle,CurrentActorInfo,CurrentActivationInfo,true);}
void UGA_CoreMorphTailStrike::ApplyContact(const FHitResult& Hit)
{
 if(!Combat.IsValid() || !IsActive() || Combat->Action!=ECoreMorphScorpionAction::Thrust)return;
 auto* TargetASC=UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Hit.GetActor());auto* ASC=GetAbilitySystemComponentFromActorInfo();
 if(!TargetASC || TargetASC==ASC)return;
 auto Context=ASC->MakeEffectContext();Context.AddHitResult(Hit);Context.AddSourceObject(GetAvatarActorFromActorInfo());
 auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphTailDamage::StaticClass(),1,Context);
 if(Spec.IsValid()){Spec.Data->SetSetByCallerMagnitude(TAG_Data_Damage,-FMath::Max(0.f,Combat->StrikeDamage));ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),TargetASC);}
}
void UGA_CoreMorphTailStrike::EndAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,bool Replicate,bool Cancelled)
{
 if(Combat.IsValid())
 {
  Combat->OnStrikeStageFinished.Remove(StageHandle);Combat->OnStrikeInvalidated.Remove(InvalidHandle);Combat->OnStrikeContact.Remove(ContactHandle);
  if(Cancelled)Combat->ResetStrike();else Combat->StartAction(ECoreMorphScorpionAction::Idle);
 }
 auto* ASC=GetAbilitySystemComponentFromActorInfo();auto* Boss=Cast<ACoreMorphBoss>(GetAvatarActorFromActorInfo());
 if(ASC){ASC->RemoveActiveGameplayEffect(AttackEffect);if(Boss && !Boss->IsDead())
 {auto Spec=ASC->MakeOutgoingSpec(UGE_CoreMorphTailCooldown::StaticClass(),1,ASC->MakeEffectContext());if(Spec.IsValid()){Spec.Data->SetDuration(FMath::Max(.05f,Boss->ScorpionCombat->CooldownDuration),true);ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());}}}
 Combat.Reset();AttackEffect.Invalidate();StageHandle.Reset();InvalidHandle.Reset();ContactHandle.Reset();
 Super::EndAbility(H,I,A,Replicate,Cancelled);
}
