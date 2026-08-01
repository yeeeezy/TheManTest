#include "GAS/Abilities/GA_EnemyShoot.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Equipment/Firearms/Bullets/BulletBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "UObject/ConstructorHelpers.h"

UGA_EnemyShoot::UGA_EnemyShoot()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

	// 不注册 GameplayEvent 触发器：由 AEnemyBase::UseRandomSkill 按类激活，
	// 这样一个敌人可拥有多个本类的子类技能（各绑不同子弹），分别独立触发。
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> DefaultHumanoidMuzzle(
		TEXT("/Game/Effects/_Shared/Muzzle/Systems/NS_HumanoidRifle_Muzzle.NS_HumanoidRifle_Muzzle"));
	if (DefaultHumanoidMuzzle.Succeeded())
	{
		MuzzleEffect = DefaultHumanoidMuzzle.Object;
		MuzzleEffectScale = FVector(0.75f);
	}
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

	FireSingleRound(Enemy);
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_EnemyShoot::FireSingleRound(AHumanoidEnemy* Enemy)
{
	if (!Enemy) return false;

	// 枪口位置：武器网格 Muzzle socket → 武器组件原点 → 敌人位置
	UStaticMeshComponent* WeaponMesh = Enemy->GetWeaponMesh();
	FVector MuzzleLocation;
	if (WeaponMesh && MuzzleSocketName != NAME_None && WeaponMesh->DoesSocketExist(MuzzleSocketName))
	{
		MuzzleLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
	}
	else if (WeaponMesh)
	{
		MuzzleLocation = WeaponMesh->GetComponentTransform().TransformPosition(MuzzleRelativeOffset);
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

	// 子弹生成（可被子类重写为散射/连发/hitscan）。散布在公共基类统一处理，
	// 因此三连发、扫射和后续人形怪无需各自复制命中误差逻辑。
	SpawnProjectiles(Enemy, MuzzleLocation, CalculateShotDirection(Enemy, FireDir));
	if (MuzzleEffect)
	{
		// 使用已经验证过的枪口世界位置和射击方向。即使模型缺少同名 Socket，
		// 也不会把特效悄悄挂到武器原点、藏进枪体内部。
		if (UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, MuzzleEffect, MuzzleLocation, FireDir.Rotation() + MuzzleEffectRotation,
			MuzzleEffectScale, true, true, ENCPoolMethod::AutoRelease, true))
		{
			Effect->Activate(true);
		}
	}

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
	else if (FireAnimation)
	{
		if (USkeletalMeshComponent* Mesh = Enemy->GetMesh())
			if (UAnimInstance* AnimInst = Mesh->GetAnimInstance())
				AnimInst->PlaySlotAnimationAsDynamicMontage(FireAnimation, FireAnimationSlot);
	}

	// 开火音效
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, MuzzleLocation,
			FireSoundVolumeMultiplier, FireSoundPitchMultiplier);
	}
	return BulletClass != nullptr;
}

FVector UGA_EnemyShoot::CalculateShotDirection(AHumanoidEnemy* Enemy, const FVector& FireDir)
{
	const UWorld* World = GetWorld();
	const double Now = World ? World->GetTimeSeconds() : 0.0;
	if (LastShotTimeSeconds >= 0.0)
	{
		const float Recovered = static_cast<float>(Now - LastShotTimeSeconds) * SpreadRecoveryDegreesPerSecond;
		CurrentSpreadDegrees = FMath::Max(0.f, CurrentSpreadDegrees - Recovered);
	}

	const bool bMoving = Enemy && Enemy->GetVelocity().Size2D() >= MovingSpeedThreshold;
	const float ShotSpread = FMath::Clamp(BaseSpreadDegrees + CurrentSpreadDegrees
		+ (bMoving ? MovingSpreadPenaltyDegrees : 0.f), 0.f, MaxSpreadDegrees);
	CurrentSpreadDegrees = FMath::Min(MaxSpreadDegrees - BaseSpreadDegrees,
		CurrentSpreadDegrees + SpreadPerShotDegrees);
	LastShotTimeSeconds = Now;

	return ShotSpread > KINDA_SMALL_NUMBER
		? FMath::VRandCone(FireDir.GetSafeNormal(), FMath::DegreesToRadians(ShotSpread))
		: FireDir.GetSafeNormal();
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
