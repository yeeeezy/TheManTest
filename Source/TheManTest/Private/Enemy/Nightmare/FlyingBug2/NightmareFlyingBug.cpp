#include "Enemy/Nightmare/FlyingBug2/NightmareFlyingBug.h"

#include "Animation/AnimSequence.h"
#include "GameFramework/CharacterMovementComponent.h"

ANightmareFlyingBug::ANightmareFlyingBug()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->DefaultLandMovementMode = MOVE_Walking;
	Movement->MaxWalkSpeed = RoamSpeed;
	Movement->GravityScale = 1.f;
	Movement->bRunPhysicsWithNoController = true;
	Movement->bOrientRotationToMovement = false;
	Movement->RotationRate = FRotator(0.f, 220.f, 0.f);
}

void ANightmareFlyingBug::BeginPlay()
{
	Super::BeginPlay();

	RoamOrigin = GetActorLocation();
	VelocityDamper.Reset(FVector::ZeroVector);
	GroundNormalDamper.Reset(FVector::UpVector);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

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

	FVector PlanarDelta = RoamDestination - GetActorLocation();
	PlanarDelta.Z = 0.f;
	const FVector DesiredVelocity = PlanarDelta.GetSafeNormal() * RoamSpeed;
	FVector SmoothedVelocity = VelocityDamper.Update(DesiredVelocity, DeltaSeconds, VelocityHalfLife);
	SmoothedVelocity.Z = 0.f;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = RoamSpeed;
	Movement->RequestDirectMove(SmoothedVelocity, false);

	FHitResult GroundHit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NightmareGroundProbe), false, this);
	const FVector ProbeStart = GetActorLocation() + FVector::UpVector * GroundProbeDistance * 0.5f;
	const FVector ProbeEnd = GetActorLocation() - FVector::UpVector * GroundProbeDistance;
	if (GetWorld()->LineTraceSingleByChannel(GroundHit, ProbeStart, ProbeEnd, ECC_Visibility, QueryParams))
	{
		const FVector SmoothNormal = GroundNormalDamper.Update(
			GroundHit.ImpactNormal.GetSafeNormal(), DeltaSeconds, GroundAlignmentHalfLife).GetSafeNormal();
		const FVector Forward = FVector::VectorPlaneProject(
			SmoothedVelocity.IsNearlyZero() ? GetActorForwardVector() : SmoothedVelocity,
			SmoothNormal).GetSafeNormal();
		if (!Forward.IsNearlyZero())
		{
			SetActorRotation(FRotationMatrix::MakeFromXZ(Forward, SmoothNormal).Rotator());
		}
	}
}

void ANightmareFlyingBug::ChooseNextDestination()
{
	const FVector2D PlanarOffset = FMath::RandPointInCircle(RoamRadius);
	RoamDestination = RoamOrigin + FVector(PlanarOffset.X, PlanarOffset.Y, 0.f);
}
