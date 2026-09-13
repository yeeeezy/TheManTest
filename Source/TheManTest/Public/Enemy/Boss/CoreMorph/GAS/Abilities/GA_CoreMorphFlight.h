#pragma once
#include "Abilities/GameplayAbility.h"
#include "GA_CoreMorphFlight.generated.h"

class UCoreMorphFlightComponent;

UCLASS()
class THEMANTEST_API UGA_CoreMorphFlight : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_CoreMorphFlight();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	TWeakObjectPtr<UCoreMorphFlightComponent> Flight;
	FDelegateHandle FinishedHandle;
	void FinishFlight();
};
