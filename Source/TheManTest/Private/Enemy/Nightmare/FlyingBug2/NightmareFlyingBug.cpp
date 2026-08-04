#include "Enemy/Nightmare/FlyingBug2/NightmareFlyingBug.h"

#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"

ANightmareFlyingBug::ANightmareFlyingBug()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->DefaultLandMovementMode = MOVE_Flying;
	Movement->MaxFlySpeed = RoamSpeed;
	Movement->GravityScale = 0.f;
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 220.f, 0.f);
}

void ANightmareFlyingBug::BeginPlay()
{
	Super::BeginPlay();

	RoamOrigin = GetActorLocation();
	VelocityDamper.Reset(FVector::ZeroVector);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	// Do not rely on the editor preview state of Animation Single Node. The concrete
	// Blueprint supplies the finalized native sequence and runtime starts it explicitly.
	if (RoamAnimation)
	{
		GetMesh()->PlayAnimation(RoamAnimation, true);
	}
	ChooseNextDestination();
}

void ANightmareFlyingBug::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const FVector ToDestination = RoamDestination - GetActorLocation();
	if (ToDestination.SizeSquared() <= FMath::Square(AcceptanceRadius))
	{
		ChooseNextDestination();
	}

	const FVector DesiredVelocity = (RoamDestination - GetActorLocation()).GetSafeNormal() * RoamSpeed;
	const FVector SmoothedVelocity = VelocityDamper.Update(DesiredVelocity, DeltaSeconds, VelocityHalfLife);
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxFlySpeed = RoamSpeed;
	Movement->Velocity = SmoothedVelocity;
}

void ANightmareFlyingBug::ChooseNextDestination()
{
	const FVector2D PlanarOffset = FMath::RandPointInCircle(RoamRadius);
	const float VerticalOffset = FMath::FRandRange(-RoamHeight, RoamHeight);
	RoamDestination = RoamOrigin + FVector(PlanarOffset.X, PlanarOffset.Y, VerticalOffset);
}
