#include "GAS/Abilities/GA_EnemyShoot.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Equipment/Firearms/Bullets/BulletBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"

UGA_EnemyShoot::UGA_EnemyShoot()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

	// 不注册 GameplayEvent 触发器：由 AEnemyBase::UseRandomSkill 按类激活，
	// 这样一个敌人可拥有多个本类的子类技能（各绑不同子弹），分别独立触发。
}

void UGA_EnemyShoot::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetAvatarActorFromActorInfo());
	if (!Enemy)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 枪口位置：武器网格 Muzzle socket → 武器组件原点 → 敌人位置
	UStaticMeshComponent* WeaponMesh = Enemy->GetWeaponMesh();
	FVector MuzzleLocation;
	if (WeaponMesh && MuzzleSocketName != NAME_None && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	else if (WeaponMesh)
	{
		MuzzleLocation = WeaponMesh->GetComponentLocation();
	}
	else
	{
		MuzzleLocation = Enemy->GetActorLocation();
	}

	// 方向：朝 AI 写入的 AimTargetWorld；无有效目标则退回敌人正前方
	FVector FireDir = (Enemy->AimTargetWorld - MuzzleLocation).GetSafeNormal();
	if (Enemy->AimTargetWorld.IsNearlyZero() || FireDir.IsNearlyZero())
	{
		FireDir = Enemy->GetActorForwardVector();
	}

	// 子弹生成（可被子类重写为散射/连发/hitscan）
	SpawnProjectiles(Enemy, MuzzleLocation, FireDir);

	// 开火蒙太奇（可选）
	if (FireMontage)
	{
		if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
		{
			if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
			{
				AnimInst->Montage_Play(FireMontage);
			}
		}
	}

	// 开火音效
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation,
			FireSoundVolumeMultiplier, FireSoundPitchMultiplier);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_EnemyShoot::SpawnProjectiles(AHumanoidEnemy* Enemy, const FVector& MuzzleLocation, const FVector& FireDir)
{
	if (!BulletClass || !Enemy) { return; }

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner      = Enemy;
	SpawnParams.Instigator = Enemy;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(
		BulletClass, MuzzleLocation, FireDir.Rotation(), SpawnParams);
	if (Bullet)
	{
		// 强度增强系数：二阶段后敌人伤害倍率 > 1，按其缩放本发子弹基础伤害。
		Bullet->Damage *= Enemy->GetDamageMultiplier();

		// 复用玩家子弹管线：飞行命中后自动施加 HitEffectClass(GE_BulletDamage) 扣血，
		// 并忽略发射者自身，命中带 Health 的玩家即扣血。
		Bullet->InitBullet(Enemy, Enemy->GetAbilitySystemComponent());
	}
}
