#include "GAS/Abilities/GA_EnemyAreaBarrage.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Equipment/Firearms/Bullets/BulletBase.h"

UGA_EnemyAreaBarrage::UGA_EnemyAreaBarrage()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_EnemyAreaBarrage::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo());
	if (!Enemy || !BarrageBulletClass)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	const FVector Center = Enemy->AimTargetWorld.IsNearlyZero() ? Enemy->GetActorLocation() : Enemy->AimTargetWorld;
	for (int32 Index = 0; Index < ProjectileCount; ++Index)
	{
		const FVector2D Offset = FMath::RandPointInCircle(BarrageRadius);
		const FVector SpawnLocation = Center + FVector(Offset.X, Offset.Y, SpawnHeight);
		FActorSpawnParameters Params;
		Params.Owner = Enemy; Params.Instigator = Enemy;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		if (ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(BarrageBulletClass,
			SpawnLocation, FRotator(-90.f, 0.f, 0.f), Params))
		{
			Bullet->Damage *= Enemy->GetDamageMultiplier();
			Bullet->InitBullet(Enemy, Enemy->GetAbilitySystemComponent());
		}
	}
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
