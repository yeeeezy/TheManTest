#pragma once

#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "GCN_EnemyHit.generated.h"

UCLASS(Blueprintable)
class THEMANTEST_API UGCN_EnemyHit : public UGCN_ImpactFeedbackBase
{
	GENERATED_BODY()

public:
	UGCN_EnemyHit();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood")
	TObjectPtr<UMaterialInterface> BloodSprayMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood")
	TObjectPtr<UMaterialInterface> BloodStainMaterial;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood", meta=(ClampMin="0.01"))
	float BloodScale=1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood", meta=(ClampMin="0.1", Units="s"))
	float BloodStainLifeSpan=12.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood", meta=(ClampMin="0",ClampMax="0.8"))
	float BloodSizeVariation=.35f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Blood", meta=(ClampMin="1",Units="cm"))
	float BodyStainProjectionDepth=12.f;
	virtual bool OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
};
