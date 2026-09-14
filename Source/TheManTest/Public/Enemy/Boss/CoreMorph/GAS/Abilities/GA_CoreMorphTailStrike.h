#pragma once
#include "Abilities/GameplayAbility.h"
#include "GameplayEffectTypes.h"
#include "GA_CoreMorphTailStrike.generated.h"

class UCoreMorphScorpionCombat;

UCLASS()
class THEMANTEST_API UGA_CoreMorphTailStrike : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_CoreMorphTailStrike();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	TWeakObjectPtr<UCoreMorphScorpionCombat> Combat;
	FDelegateHandle StageHandle,InvalidHandle,ContactHandle;
	FActiveGameplayEffectHandle AttackEffect;
 bool bDetonated=false;
	void AdvanceStage();
 void CancelStrike();
 void ApplyContact(const FHitResult& Hit);
};
