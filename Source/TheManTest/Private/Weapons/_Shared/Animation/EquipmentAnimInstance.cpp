#include "Weapons/_Shared/Animation/EquipmentAnimInstance.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UEquipmentAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	AActor* Owner = GetOwningActor();
	if (!Owner)
	{
		Speed = 0.f;
		Direction = 0.f;
		Velocity_Z = 0.f;
		bIsFalling = false;
		return;
	}

	const FVector Velocity = Owner->GetVelocity();
	Speed = Velocity.Size2D();
	Velocity_Z = Velocity.Z;

	const FRotator OwnerRotation(0.f, Owner->GetActorRotation().Yaw, 0.f);
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwnerRotation);

	if (const ACharacter* CharacterOwner = Cast<ACharacter>(Owner))
	{
		const UCharacterMovementComponent* Movement = CharacterOwner->GetCharacterMovement();
		bIsFalling = Movement ? Movement->IsFalling() : false;
	}
	else
	{
		bIsFalling = false;
	}
}
