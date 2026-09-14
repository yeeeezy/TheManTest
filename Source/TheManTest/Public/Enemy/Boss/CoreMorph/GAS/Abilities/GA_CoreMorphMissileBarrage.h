#pragma once
#include "Abilities/GameplayAbility.h"
#include "GA_CoreMorphMissileBarrage.generated.h"
class UCoreMorphMissileCombat;
UCLASS()
class THEMANTEST_API UGA_CoreMorphMissileBarrage : public UGameplayAbility
{
 GENERATED_BODY()
public:
 UGA_CoreMorphMissileBarrage();
 virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayTagContainer* S,const FGameplayTagContainer* T,FGameplayTagContainer* R) const override;
 virtual void ActivateAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,const FGameplayEventData* E) override;
 virtual void EndAbility(const FGameplayAbilitySpecHandle H,const FGameplayAbilityActorInfo* I,const FGameplayAbilityActivationInfo A,bool Replicate,bool Cancelled) override;
private:
 TWeakObjectPtr<UCoreMorphMissileCombat> Combat;
 FActiveGameplayEffectHandle AttackEffect;
 FDelegateHandle FinishedHandle,InvalidHandle,ImpactHandle;
 bool bStarted=false;
 void Finish();void CancelSalvo();void ApplyImpact(int32 Index);
};
