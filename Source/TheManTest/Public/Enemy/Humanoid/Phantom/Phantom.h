#pragma once

#include "CoreMinimal.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Phantom.generated.h"

UCLASS()
class THEMANTEST_API APhantom : public AHumanoidEnemy
{
	GENERATED_BODY()

public:
	// Temporary directional hit testing. Evaluated at spawn; restart PIE after changing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Phantom|Testing")
	bool bStationaryHitTest = true;

	virtual void ReactToProjectileHit(AActor* HitInstigator) override;
	virtual void SetCombatPhase(int32 NewPhase) override;
	virtual bool ShouldProjectilePassThrough() const override { return !IsDead() && bCloaked; }
	virtual void OnDeath() override { if (!IsDead()) SetCloaked(false); Super::OnDeath(); }

	UFUNCTION(BlueprintCallable, Category="Phantom|Phase")
	void SetCloaked(bool bEnabled);

	UFUNCTION(BlueprintPure, Category="Phantom|Phase")
	bool IsCloaked() const { return bCloaked; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Phantom|Phase", meta=(ClampMin="0.0", ClampMax="1.0"))
	float CloakedOpacity = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Phantom|Phase")
	FName OpacityParameterName = TEXT("Opacity");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Phantom|Phase")
	TObjectPtr<class UMaterialInterface> CloakMaterial;

private:
	UPROPERTY(VisibleInstanceOnly, Category="Phantom|Phase")
	bool bCloaked = false;

	UPROPERTY(Transient)
	TArray<TObjectPtr<class UMaterialInterface>> OriginalMaterials;
};
