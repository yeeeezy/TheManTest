#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_ProjectileImpact.generated.h"

class USoundBase;

UCLASS(Blueprintable)
class THEMANTEST_API UGCN_ProjectileImpact : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGCN_ProjectileImpact();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float PitchMultiplier = 1.f;

	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
};

UCLASS(Blueprintable)
class THEMANTEST_API UGCN_EnemyHit : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGCN_EnemyHit();
	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
};
