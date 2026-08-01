#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseLocomotionAnimInstance.generated.h"

class UCharacterMovementComponent;

UCLASS()
class THEMANTEST_API UBaseLocomotionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Velocity_Z;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bIsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float Direction;

private:
	UPROPERTY()
	TObjectPtr<APawn> PawnOwner;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent;
};
