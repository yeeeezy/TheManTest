#pragma once

#include "CoreMinimal.h"
#include "Enemy/Nightmare/NightmareEnemy.h"
#include "LocomotorCore.h"
#include "NightmareFlyingBug.generated.h"

class UAnimSequence;

/** Concrete Nightmare bug that continuously roams through the air. */
UCLASS(Blueprintable)
class THEMANTEST_API ANightmareFlyingBug : public ANightmareEnemy
{
	GENERATED_BODY()

public:
	ANightmareFlyingBug();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Roaming", meta = (ClampMin = "50.0"))
	float RoamRadius = 900.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Roaming", meta = (ClampMin = "0.0"))
	float RoamHeight = 260.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Roaming", meta = (ClampMin = "1.0"))
	float RoamSpeed = 220.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Roaming", meta = (ClampMin = "0.01"))
	float VelocityHalfLife = 0.18f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Roaming", meta = (ClampMin = "10.0"))
	float AcceptanceRadius = 90.f;

	/** Finalized native bug animation assigned by the concrete Blueprint. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nightmare|Animation")
	TObjectPtr<UAnimSequence> RoamAnimation;

private:
	void ChooseNextDestination();

	FVector RoamOrigin = FVector::ZeroVector;
	FVector RoamDestination = FVector::ZeroVector;
	FVectorDamper VelocityDamper;
};
