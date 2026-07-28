#include "Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"

void UFPSCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (bWeaponTransitionActive)
	{
		// 起播 Montage 的首个动画求值必须保留 Alpha=0，确保真正渲染到起始姿势。
		if (bHoldWeaponTransitionFirstUpdate)
		{
			bHoldWeaponTransitionFirstUpdate = false;
		}
		else
		{
			WeaponTransitionAlpha = FMath::Min(
				1.f,
				WeaponTransitionAlpha + DeltaSeconds / FMath::Max(WeaponTransitionDuration, KINDA_SMALL_NUMBER));

			if (WeaponTransitionAlpha >= 1.f)
			{
				bWeaponTransitionActive = false;
				// 快照已经平滑到 Equip 的 0 秒姿势，现在才允许 Montage 向前推进。
				if (PendingWeaponTransitionMontage)
				{
					Montage_Resume(PendingWeaponTransitionMontage);
					PendingWeaponTransitionMontage = nullptr;
				}
			}
		}
	}

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

void UFPSCharacterAnimInstance::CaptureWeaponTransitionPose()
{
	SavePoseSnapshot(TEXT("WeaponTransitionPose"));
	WeaponTransitionAlpha = 0.f;
	bWeaponTransitionActive = false;
	bHoldWeaponTransitionFirstUpdate = false;
}

void UFPSCharacterAnimInstance::StartWeaponTransition(UAnimMontage* MontageToPlay)
{
	WeaponTransitionAlpha = 0.f;
	bWeaponTransitionActive = true;
	bHoldWeaponTransitionFirstUpdate = true;
	PendingWeaponTransitionMontage = MontageToPlay;

	// 让实时分支输出 Montage 的准确 0 秒姿势，但在快照桥接完成前绝不推进时间。
	if (PendingWeaponTransitionMontage)
	{
		Montage_Play(PendingWeaponTransitionMontage);
		Montage_Pause(PendingWeaponTransitionMontage);
	}
}

void UFPSCharacterAnimInstance::CompleteWeaponTransition()
{
	WeaponTransitionAlpha = 1.f;
	bWeaponTransitionActive = false;
	bHoldWeaponTransitionFirstUpdate = false;
	PendingWeaponTransitionMontage = nullptr;
}
