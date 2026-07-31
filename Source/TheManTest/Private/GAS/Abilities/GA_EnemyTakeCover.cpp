#include "GAS/Abilities/GA_EnemyTakeCover.h"
#include "Characters/Enemy/Cover/EnemyCoverPoint.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

UGA_EnemyTakeCover::UGA_EnemyTakeCover()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_EnemyTakeCover::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo());
	AEnemyCoverPoint* Cover = Enemy ? AEnemyCoverPoint::FindBestCover(Enemy, Enemy->GetActorLocation(),
		Enemy->AimTargetWorld, MaxCoverDistance) : nullptr;
	AAIController* AI = Enemy ? Cast<AAIController>(Enemy->GetController()) : nullptr;
	if (!Enemy || !Cover || !AI)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	if (RollMontage && Enemy->GetMesh() && Enemy->GetMesh()->GetAnimInstance())
		Enemy->GetMesh()->GetAnimInstance()->Montage_Play(RollMontage);
	AI->MoveToLocation(Cover->GetStandLocation(), 60.f, true, true, true, false);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
