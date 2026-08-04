#include "Enemy/Nightmare/FlyingBug2/NightmareFlyingBug.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

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

	// The authored walk is the source pose and the embedded Control Rig only corrects
	// the six ground-contact legs. This preserves head/tentacle motion and gives FBIK
	// the authored joint bend directions as its starting pose.
	static ConstructorHelpers::FClassFinder<UAnimInstance> LocomotorAnimClass(
		TEXT("/Game/Enemy/Nightmare/FlyingBug2/Animations/Logic/ABP_NightmareFlyingBug2_WalkLocomotor"));
	if (LocomotorAnimClass.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(LocomotorAnimClass.Class);
	}
}

void ANightmareFlyingBug::BeginPlay()
{
	Super::BeginPlay();

	RoamOrigin = GetActorLocation();
	VelocityDamper.Reset(FVector::ZeroVector);
	GroundNormalDamper.Reset(FVector::UpVector);
	AuthoredMeshRelativeRotation = GetMesh()->GetRelativeRotation().Quaternion();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	if (UClass* WalkLocomotorClass = LoadClass<UAnimInstance>(nullptr,
		TEXT("/Game/Enemy/Nightmare/FlyingBug2/Animations/Logic/ABP_NightmareFlyingBug2_WalkLocomotor.ABP_NightmareFlyingBug2_WalkLocomotor_C")))
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(WalkLocomotorClass);
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
			const FRotator SurfaceRotation = FRotationMatrix::MakeFromXZ(Forward, SmoothNormal).Rotator();
			// ACharacter's collision capsule must remain vertical. Tilting the whole actor
			// makes the capsule wedge into convex/concave slope seams. Only yaw belongs to
			// the actor; the visual mesh follows the full ground-normal orientation.
			SetActorRotation(FRotator(0.f, SurfaceRotation.Yaw, 0.f));
			// Preserve the skeletal mesh import/Blueprint orientation. Applying the
			// surface frame directly discarded that authored basis on flat ground,
			// stood this creature up, and made Locomotor solve feet in the wrong frame.
			GetMesh()->SetWorldRotation(SurfaceRotation.Quaternion() * AuthoredMeshRelativeRotation);
		}
	}

}

void ANightmareFlyingBug::ChooseNextDestination()
{
	const FVector2D PlanarOffset = FMath::RandPointInCircle(RoamRadius);
	RoamDestination = RoamOrigin + FVector(PlanarOffset.X, PlanarOffset.Y, 0.f);
}
