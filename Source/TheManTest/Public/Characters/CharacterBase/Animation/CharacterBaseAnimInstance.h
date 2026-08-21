#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"
#include "CharacterBaseAnimInstance.generated.h"

// Shared player animation data source. Skeleton-independent Template AnimBPs derive
// from this class; skeleton-specific AnimBPs derive from the template asset.
UCLASS(Blueprintable)
class THEMANTEST_API UCharacterBaseAnimInstance : public UFPSCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void UpdateCharacterAnimationState(
		double InCharacterSpeed,
		bool bInAir,
		double InLeanSidesAmount,
		double InLookUpAmount);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Character Animation")
	bool Is_Moving = false;

	UPROPERTY(BlueprintReadWrite, Category = "Character Animation")
	bool Is_InAir = false;

	UPROPERTY(BlueprintReadWrite, Category = "Character Animation")
	double Character_Speed = 0.0;

};
