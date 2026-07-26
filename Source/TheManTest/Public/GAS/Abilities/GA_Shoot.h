#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Shoot.generated.h"

/**
 * UGA_Shoot
 * 通用射击技能，监听 Input.Weapon.PrimaryFire 事件。
 * 具体射击参数（射程、伤害 GE 等）从当前装备的 AFirearm 读取，实现所有枪械复用。
 */
UCLASS()
class THEMANTEST_API UGA_Shoot : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Shoot();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;
};
