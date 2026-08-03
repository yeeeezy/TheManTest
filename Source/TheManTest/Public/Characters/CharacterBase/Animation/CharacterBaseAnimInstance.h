#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CharacterBaseAnimInstance.generated.h"

// Shared player animation data source. Skeleton-independent Template AnimBPs derive
// from this class; skeleton-specific AnimBPs derive from the template asset.
UCLASS(Blueprintable)
class THEMANTEST_API UCharacterBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	void UpdateCharacterAnimationState(
		double InCharacterSpeed,
		bool bInAir,
		double InLeanSidesAmount,
		double InLookUpAmount);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Character Animation")
	bool Is_Moving = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character Animation")
	bool Is_InAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character Animation")
	double Character_Speed = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Character Animation")
	double Lean_Sides_Amount = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Character Animation")
	double Look_Up_Amount = 0.0;
};
