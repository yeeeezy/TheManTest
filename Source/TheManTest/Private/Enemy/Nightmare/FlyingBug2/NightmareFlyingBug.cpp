#include "Enemy/Nightmare/FlyingBug2/NightmareFlyingBug.h"

#include "Animation/AnimSequence.h"
#include "ControlRig.h"
#include "ControlRigComponent.h"
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

	ProceduralLocomotor = CreateDefaultSubobject<UControlRigComponent>(TEXT("ProceduralLocomotor"));
	// Keep the rig host outside the mapped mesh's attachment chain. Parenting the rig
	// component to the mesh while also mapping that mesh creates a tick dependency cycle.
	ProceduralLocomotor->SetupAttachment(RootComponent);
	// ANightmareFlyingBug ticks the rig explicitly after movement/surface alignment so
	// the mapped skeletal output is refreshed in the same frame and cannot lag behind.
	ProceduralLocomotor->bUpdateRigOnTick = false;
	ProceduralLocomotor->bEnableLazyEvaluation = false;
	ProceduralLocomotor->bResetTransformBeforeTick = false;
	static ConstructorHelpers::FClassFinder<UControlRig> LocomotorRigClass(
		TEXT("/Game/Enemy/Nightmare/FlyingBug2/Animations/ControlRig/CR_NightmareFlyingBug2_Locomotor"));
	if (LocomotorRigClass.Succeeded())
	{
		ProceduralLocomotor->ControlRigClass = LocomotorRigClass.Class;
	}
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
	if (ProceduralLocomotor && ProceduralLocomotor->ControlRigClass)
	{
		// The SkeletalMesh "Default Animating Rig" field is editor preview only. Runtime
		// must explicitly map the complete mesh so Locomotor/FullBodyIK outputs reach bones.
		ProceduralLocomotor->ClearMappedElements();
		ProceduralLocomotor->AddMappedCompleteSkeletalMesh(
			GetMesh(), EControlRigComponentMapDirection::Output);
		ProceduralLocomotor->Initialize();
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
			GetMesh()->SetWorldRotation(SurfaceRotation);
		}
	}

	if (ProceduralLocomotor && ProceduralLocomotor->CanExecute())
	{
		ProceduralLocomotor->Update(DeltaSeconds);
		GetMesh()->TickAnimation(DeltaSeconds, false);
		GetMesh()->RefreshBoneTransforms();
	}
}

void ANightmareFlyingBug::ChooseNextDestination()
{
	const FVector2D PlanarOffset = FMath::RandPointInCircle(RoamRadius);
	RoamDestination = RoamOrigin + FVector(PlanarOffset.X, PlanarOffset.Y, 0.f);
}
