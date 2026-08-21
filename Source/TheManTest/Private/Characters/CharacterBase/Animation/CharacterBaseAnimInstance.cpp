#include "Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h"

void UCharacterBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	Character_Speed = Speed;
	Is_Moving = Speed > 0.0f;
	Is_InAir = bIsFalling;
}

void UCharacterBaseAnimInstance::UpdateCharacterAnimationState(
	const double InCharacterSpeed,
	const bool bInAir,
	const double InLeanSidesAmount,
	const double InLookUpAmount)
{
	Character_Speed = InCharacterSpeed;
	Is_Moving = InCharacterSpeed > 0.0;
	Is_InAir = bInAir;
	Lean_Sides_Amount = InLeanSidesAmount;
	Look_Up_Amount = InLookUpAmount;
}
