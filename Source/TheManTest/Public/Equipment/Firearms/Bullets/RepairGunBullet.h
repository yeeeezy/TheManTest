#pragma once

#include "CoreMinimal.h"
#include "Equipment/Firearms/Bullets/BulletBase.h"
#include "Environment/Hazards/HazardSuppressorInterface.h"
#include "RepairGunBullet.generated.h"

/**
 * ARepairGunBullet
 * 命中后以指数曲线快速膨胀，达到最大 Scale 后销毁。
 */
// 实现 IHazardSuppressor：泡泡进入危险区时压制特效和伤害，销毁后自动恢复
UCLASS()
class THEMANTEST_API ARepairGunBullet : public ABulletBase, public IHazardSuppressor
{
	GENERATED_BODY()

public:
	ARepairGunBullet();

	virtual void Tick(float DeltaTime) override;
	virtual void ProcessHit_Implementation(const FHitResult& HitResult, AActor* HitInstigator, UAbilitySystemComponent* SourceASC) override;

	// 膨胀持续时间（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Expansion")
	float ExpansionDuration = 0.5f;

	// 达到此 Scale 后停止膨胀
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Expansion")
	float MaxExpansionScale = 8.f;

	// 膨胀完成后保留多少秒再销毁
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Expansion")
	float LifetimeAfterExpansion = 5.f;

private:
	bool  bIsExpanding     = false;
	float ExpansionElapsed = 0.f;

	FTimerHandle LifetimeTimerHandle;

	UFUNCTION()
	void OnLifetimeExpired();
};
