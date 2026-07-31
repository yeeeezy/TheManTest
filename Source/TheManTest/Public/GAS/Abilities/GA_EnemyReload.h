#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyReload.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

UCLASS()
class THEMANTEST_API UGA_EnemyReload : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EnemyReload();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="EnemyReload", meta=(ClampMin="0.0"))
	float ReloadDuration = 1.8f;
	UPROPERTY(EditDefaultsOnly, Category="EnemyReload")
	TObjectPtr<UAnimMontage> ReloadMontage;
	UPROPERTY(EditDefaultsOnly, Category="EnemyReload")
	TObjectPtr<UAnimSequenceBase> ReloadAnimation;
	UPROPERTY(EditDefaultsOnly, Category="EnemyReload")
	FName ReloadAnimationSlot = TEXT("DefaultSlot");

private:
	void FinishReload();
	FTimerHandle ReloadTimer;
};
