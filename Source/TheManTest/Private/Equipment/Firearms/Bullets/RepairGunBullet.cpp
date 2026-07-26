#include "Equipment/Firearms/Bullets/RepairGunBullet.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

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
	Super::ProcessHit_Implementation(HitResult, HitInstigator, SourceASC);

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
	}

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
