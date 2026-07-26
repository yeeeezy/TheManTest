#include "Characters/Animation/BaseLocomotionAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void UBaseLocomotionAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PawnOwner = TryGetPawnOwner();
	if (PawnOwner)
	{
		MovementComponent = PawnOwner->FindComponentByClass<UCharacterMovementComponent>();
	}
}

void UBaseLocomotionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!PawnOwner || !MovementComponent) return;

	const FVector Velocity = PawnOwner->GetVelocity();

	Speed      = Velocity.Size2D();
	Velocity_Z = Velocity.Z;
	bIsFalling = MovementComponent->IsFalling();

	const FMatrix RotMatrix = FRotationMatrix(PawnOwner->GetActorRotation());
	Direction = FMath::RadiansToDegrees(FMath::Atan2(
		FVector::DotProduct(Velocity, RotMatrix.GetScaledAxis(EAxis::Y)),
		FVector::DotProduct(Velocity, RotMatrix.GetScaledAxis(EAxis::X))));

	float RawPitch = PawnOwner->GetBaseAimRotation().Pitch;
	if (RawPitch > 180.f) RawPitch -= 360.f;
	AimPitch = FMath::Clamp(RawPitch, -90.f, 90.f) / 90.f;
}
