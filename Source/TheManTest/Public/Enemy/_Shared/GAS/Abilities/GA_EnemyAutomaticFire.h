#pragma once

#include "CoreMinimal.h"
#include "Enemy/_Shared/GAS/Abilities/GA_EnemyShoot.h"
#include "GA_EnemyAutomaticFire.generated.h"

class UEnemyMagazineComponent;

// 同一能力通过 ShotsPerActivation/ShotInterval 数据配置为三连发或持续扫射。
UCLASS()
class THEMANTEST_API UGA_EnemyAutomaticFire : public UGA_EnemyShoot
{
	GENERATED_BODY()

public:
	UGA_EnemyAutomaticFire();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="EnemyShoot|Automatic", meta=(ClampMin="1"))
	int32 ShotsPerActivation = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="EnemyShoot|Automatic", meta=(ClampMin="0.01"))
	float ShotInterval = 0.12f;

private:
	void FireNextRound();
	FTimerHandle ShotTimer;
	int32 ShotsRemaining = 0;
	TWeakObjectPtr<AHumanoidEnemy> FiringEnemy;
};
