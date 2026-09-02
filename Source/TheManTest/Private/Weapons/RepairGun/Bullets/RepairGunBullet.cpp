#include "Weapons/RepairGun/Bullets/RepairGunBullet.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"

ARepairGunBullet::ARepairGunBullet()
{
	PrimaryActorTick.bCanEverTick         = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 泡泡命中后自行膨胀并定时销毁，不走基类的命中即销毁逻辑
	bDestroyOnHit = false;
}

void ARepairGunBullet::ProcessHit_Implementation(
	const FHitResult& HitResult,
	AActor* HitInstigator,
	UAbilitySystemComponent* SourceASC)
{
	AEnemyBase* HitEnemy = Cast<AEnemyBase>(HitResult.GetActor());
	if (HitEnemy && HitEnemy->ShouldProjectilePassThrough())
	{
		Super::ProcessHit_Implementation(HitResult, HitInstigator, SourceASC);
		return;
	}

	Super::ProcessHit_Implementation(HitResult, HitInstigator, SourceASC);

	if (!HitEnemy && EnvironmentImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EnvironmentImpactSound,
			HitResult.ImpactPoint,
			EnvironmentImpactSoundVolumeMultiplier,
			EnvironmentImpactSoundPitchMultiplier);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

	// 命中敌人：施加子弹自身配置的减速并立即消失，不生成驻留泡泡。
	if (HitEnemy)
	{
		HitEnemy->ApplyMovementSlow(SlowPercent, SlowDuration);
		Destroy();
		return;
	}

	// 命中环境/危险区仍保留原有膨胀与压制生命周期。
	SetActorScale3D(FVector(1.f));
	bIsExpanding = true;
	SetActorTickEnabled(true);
}

void ARepairGunBullet::OnLifetimeExpired()
{
	Destroy();
}

void ARepairGunBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsExpanding) { return; }

	ExpansionElapsed += DeltaTime;

	// ExpansionDuration 秒内从 1 线性插值到 MaxExpansionScale（指数曲线）
	// Rate = ln(MaxExpansionScale) / ExpansionDuration，保证在设定时间内恰好到达
	const float Rate  = FMath::Loge(MaxExpansionScale) / FMath::Max(ExpansionDuration, KINDA_SMALL_NUMBER);
	const float Scale = FMath::Exp(Rate * ExpansionElapsed);

	if (Scale >= MaxExpansionScale)
	{
		SetActorScale3D(FVector(MaxExpansionScale));
		SetActorTickEnabled(false);
		bIsExpanding = false;

		GetWorldTimerManager().SetTimer(
			LifetimeTimerHandle,
			this,
			&ARepairGunBullet::OnLifetimeExpired,
			LifetimeAfterExpansion,
			false);
		return;
	}

	SetActorScale3D(FVector(Scale));
}
