#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EnemyAreaBarrage.generated.h"

class ABulletBase;

UCLASS()
class THEMANTEST_API UGA_EnemyAreaBarrage : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_EnemyAreaBarrage();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="EnemyBarrage")
	TSubclassOf<ABulletBase> BarrageBulletClass;
	UPROPERTY(EditDefaultsOnly, Category="EnemyBarrage", meta=(ClampMin="1"))
	int32 ProjectileCount = 12;
	UPROPERTY(EditDefaultsOnly, Category="EnemyBarrage", meta=(ClampMin="0.0"))
	float BarrageRadius = 500.f;
	UPROPERTY(EditDefaultsOnly, Category="EnemyBarrage", meta=(ClampMin="0.0"))
	float SpawnHeight = 900.f;
};
