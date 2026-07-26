#include "Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"

#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
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

	if (AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(Pawn))
	{
		bIsTurningInPlace = FPSCharacter->IsBodyTurningInPlace();
		TurnInPlaceAngle = FPSCharacter->GetBodyTurnInPlaceAngle();
		TurnInPlaceIndex = FPSCharacter->GetBodyTurnInPlaceIndex();
		TurnInPlacePlayRate = FPSCharacter->GetBodyTurnInPlacePlayRate();

		if (bUseTurnProgressCurve && FPSCharacter->IsBodyTurnVisualInProgress())
		{
			const float CurveValue = GetCurveValue(TurnProgressCurveName);
			const int32 TurnSequenceId = FPSCharacter->GetBodyTurnSequenceId();
			if (!bTurnProgressCurveStarted || LastTurnProgressSequenceId != TurnSequenceId)
			{
				TurnProgressCurveStartValue = CurveValue;
				bTurnProgressCurveStarted = true;
				LastTurnProgressSequenceId = TurnSequenceId;
			}

			const float ProgressAlpha = FMath::Abs(CurveValue - TurnProgressCurveStartValue) / FMath::Max(TurnProgressCurveCompleteValue, KINDA_SMALL_NUMBER);
			FPSCharacter->SetBodyTurnProgressAlpha(ProgressAlpha);
		}
		else
		{
			TurnProgressCurveStartValue = 0.f;
			bTurnProgressCurveStarted = false;
			LastTurnProgressSequenceId = INDEX_NONE;
		}
	}
	else
	{
		bIsTurningInPlace = false;
		TurnInPlaceAngle = 0.f;
		TurnInPlaceIndex = 0;
		TurnInPlacePlayRate = 1.f;
		TurnProgressCurveStartValue = 0.f;
		bTurnProgressCurveStarted = false;
		LastTurnProgressSequenceId = INDEX_NONE;
	}
}
