#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyTakeCover.generated.h"

class UAnimMontage;

UCLASS()
class THEMANTEST_API UGA_EnemyTakeCover : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EnemyTakeCover();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="EnemyCover", meta=(ClampMin="0.0"))
	float MaxCoverDistance = 2000.f;
	UPROPERTY(EditDefaultsOnly, Category="EnemyCover")
	TObjectPtr<UAnimMontage> RollMontage;
};
