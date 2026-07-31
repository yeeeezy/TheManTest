#include "GAS/Abilities/GA_EnemyAutomaticFire.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Characters/Enemy/Components/EnemyMagazineComponent.h"
#include "Engine/World.h"

UGA_EnemyAutomaticFire::UGA_EnemyAutomaticFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_EnemyAutomaticFire::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	const AHumanoidEnemy* Enemy = ActorInfo ? Cast<AHumanoidEnemy>(ActorInfo->AvatarActor.Get()) : nullptr;
	return Enemy && Enemy->GetMagazineComponent() && !Enemy->GetMagazineComponent()->IsEmpty();
}

void UGA_EnemyAutomaticFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	FiringEnemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo());
	ShotsRemaining = ShotsPerActivation;
	if (!FiringEnemy.IsValid() || !FiringEnemy->GetMagazineComponent())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	FireNextRound();
}

void UGA_EnemyAutomaticFire::FireNextRound()
{
	AHumanoidEnemy* Enemy = FiringEnemy.Get();
	UEnemyMagazineComponent* Magazine = Enemy ? Enemy->GetMagazineComponent() : nullptr;
	if (!Enemy || !Magazine || ShotsRemaining <= 0 || !Magazine->ConsumeRound())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FireSingleRound(Enemy);
	--ShotsRemaining;
	if (ShotsRemaining <= 0)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(ShotTimer, this, &UGA_EnemyAutomaticFire::FireNextRound,
		ShotInterval, false);
}

void UGA_EnemyAutomaticFire::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ShotTimer);
	FiringEnemy.Reset();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
