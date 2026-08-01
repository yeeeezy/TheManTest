#include "Characters/Enemy/_Shared/GAS/Abilities/GA_EnemyReload.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Characters/Enemy/Components/EnemyMagazineComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UGA_EnemyReload::UGA_EnemyReload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool UGA_EnemyReload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
	const AHumanoidEnemy* Enemy = ActorInfo ? Cast<AHumanoidEnemy>(ActorInfo->AvatarActor.Get()) : nullptr;
	return Enemy && Enemy->GetMagazineComponent() && Enemy->GetMagazineComponent()->IsEmpty();
}

void UGA_EnemyReload::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo());
	if (!Enemy || !Enemy->GetMagazineComponent())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (ReloadMontage && Enemy->GetMesh() && Enemy->GetMesh()->GetAnimInstance())
		Enemy->GetMesh()->GetAnimInstance()->Montage_Play(ReloadMontage);
	else if (ReloadAnimation && Enemy->GetMesh() && Enemy->GetMesh()->GetAnimInstance())
		Enemy->GetMesh()->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(ReloadAnimation, ReloadAnimationSlot);
	if (ReloadDuration <= 0.f) FinishReload();
	else GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &UGA_EnemyReload::FinishReload, ReloadDuration, false);
}

void UGA_EnemyReload::FinishReload()
{
	if (AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo()))
		if (UEnemyMagazineComponent* Magazine = Enemy->GetMagazineComponent()) Magazine->Reload();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
