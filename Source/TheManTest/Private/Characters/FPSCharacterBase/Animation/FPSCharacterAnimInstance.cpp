#include "Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void UFPSCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
	{
		return;
	}

	FVector Accel = FVector::ZeroVector;
	if (UCharacterMovementComponent* CMC = Pawn->FindComponentByClass<UCharacterMovementComponent>())
	{
		Accel = CMC->GetCurrentAcceleration();
	}

	bHasAcceleration = !Accel.IsNearlyZero();
	if (bHasAcceleration)
	{
		const FMatrix RotMatrix = FRotationMatrix(Pawn->GetActorRotation());
		AccelDirection = FMath::RadiansToDegrees(FMath::Atan2(
			FVector::DotProduct(Accel, RotMatrix.GetScaledAxis(EAxis::Y)),
			FVector::DotProduct(Accel, RotMatrix.GetScaledAxis(EAxis::X))));
	}
}
