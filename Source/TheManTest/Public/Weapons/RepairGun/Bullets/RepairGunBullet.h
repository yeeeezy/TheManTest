#pragma once

#include "CoreMinimal.h"
#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
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

	// 命中敌人时降低其移动速度的百分比。0.4 = 减速40%。
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Slow", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SlowPercent = 0.4f;

	// 敌人减速持续时间；连续命中刷新持续时间，不叠加强度。
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Bullet|Slow", meta = (ClampMin = "0.0"))
	float SlowDuration = 2.5f;

private:
	bool  bIsExpanding     = false;
	float ExpansionElapsed = 0.f;

	FTimerHandle LifetimeTimerHandle;

	UFUNCTION()
	void OnLifetimeExpired();
};
