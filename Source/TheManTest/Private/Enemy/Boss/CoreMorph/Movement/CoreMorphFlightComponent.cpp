#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Engine/World.h"

UCoreMorphFlightComponent::UCoreMorphFlightComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

ACoreMorphBoss* UCoreMorphFlightComponent::Boss() const { return Cast<ACoreMorphBoss>(GetOwner()); }

void UCoreMorphFlightComponent::RebuildAssembly()
{
	ACoreMorphBoss* Owner = Boss();
	if (!Owner || Owner->IsTemplate()) return;
	TInlineComponentArray<UStaticMeshComponent*> Old(Owner);
	for (auto* Piece : Old) if (Piece->ComponentHasTag(TEXT("CoreMorphVisual"))) Piece->DestroyComponent();
	Pieces.Reset();
	Layout = Owner->VisualLayout;
	if (!Layout) return;
	for (const auto& Piece : Layout->Pieces)
	{
		auto* Mesh = NewObject<UStaticMeshComponent>(Owner);
		Mesh->ComponentTags.Add(TEXT("CoreMorphVisual"));
		Mesh->SetupAttachment(Owner->GetRootComponent());
		// Editor pieces follow the placement gizmo, including during a drag. Runtime
		// pieces use world poses while the root independently follows the flight body.
		Mesh->SetAbsolute(bHaveFrame, bHaveFrame, bHaveFrame);
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetStaticMesh(Piece.Mesh);
		Mesh->SetCanEverAffectNavigation(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionObjectType(ECC_Pawn);
		Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		Mesh->RegisterComponent();
		Mesh->SetCustomPrimitiveDataFloat(0, 1.05f);
		Mesh->SetCustomPrimitiveDataVector3(1, Piece.RevealAxis);
		Mesh->SetCustomPrimitiveDataFloat(5, Piece.RevealMin);
		Mesh->SetCustomPrimitiveDataFloat(6, 1.f / FMath::Max(1.f, Piece.RevealSpan));
		Pieces.Add(Mesh);
	}
	// Placement denotes the body, not the distant origin of the source choreography.
	// Compose in local space so moving, rotating or scaling an instance preserves placement.
	if (!bHaveFrame)
	{
		ChoreographyFrame = FTransform(-Path.GetFormOffset(0)) * Owner->GetActorTransform();
		ResetMotion(RestBody());
	}
	UpdatePose();
}

void UCoreMorphFlightComponent::BeginPlay()
{
	Super::BeginPlay();
	ChoreographyFrame = FTransform(-Path.GetFormOffset(0)) * GetOwner()->GetActorTransform();
	bHaveFrame = true;
	Layout = Boss()->VisualLayout;
	ResetMotion(RestBody());
	RebuildAssembly();
}

void UCoreMorphFlightComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

FTransform UCoreMorphFlightComponent::RestBody() const
{
	return FTransform(ChoreographyFrame.GetRotation(), ChoreographyFrame.TransformPosition(Path.GetFormOffset(0)));
}

void UCoreMorphFlightComponent::ResetMotion(const FTransform& Body)
{
	float TailLength = 100.f;
	if (Layout) for (const auto& Piece : Layout->Pieces)
		if (Piece.Kind == TEXT("Tail")) TailLength = FMath::Max(TailLength, float(-Piece.Position.X * Path.MantaScale * ChoreographyFrame.GetScale3D().X));
	Motion.Reset(Body, MotionSeed ? MotionSeed : FMath::Rand(), MotionRandomness, TailLength);
}

bool UCoreMorphFlightComponent::CanStartFlight() const
{
	if (!Boss() || Boss()->IsDead() || !Layout || Pieces.Num() != 154 || bFlying || bHolding) return false;
	return !FlightRoute || (IsValid(FlightRoute) && FlightRoute->Spline && FlightRoute->Spline->GetSplineLength() > 1.f);
}

bool UCoreMorphFlightComponent::StartFlight()
{
	if (!CanStartFlight()) return false;
	Path.PrepareDive();
	ActiveRoute = FlightRoute;
	bUsingRoute = ActiveRoute.IsValid();
	RouteDistance = 0;
	CurrentRouteSpeed = 0;
	if (bUsingRoute)
	{
		const auto* Spline = ActiveRoute->Spline.Get();
		ResetMotion(FTransform(Spline->GetDirectionAtDistanceAlongSpline(0, ESplineCoordinateSpace::World).Rotation(),
			Spline->GetLocationAtDistanceAlongSpline(0, ESplineCoordinateSpace::World)));
	}
	FlightSeconds = 0;
	bFlying = true;
	bPaused = bHolding = false;
	SetComponentTickEnabled(true);
	UpdatePose();
	return true;
}

void UCoreMorphFlightComponent::StopFlight()
{
	bFlying = false;
	bHolding = true;
	bPaused = false;
	ActiveRoute.Reset();
}

void UCoreMorphFlightComponent::ResetPreview()
{
	StopFlight();
	bHolding = bUsingRoute = false;
	FlightSeconds = 0;
	RouteDistance = 0;
	CurrentRouteSpeed = 0;
	ResetMotion(RestBody());
	SetComponentTickEnabled(true);
	UpdatePose();
}

void UCoreMorphFlightComponent::Shutdown()
{
	StopFlight();
	SetComponentTickEnabled(false);
	OnFlightFinished.Clear();
	for (const auto& Piece : Pieces) if (IsValid(Piece)) Piece->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UCoreMorphFlightComponent::TickComponent(float Dt, ELevelTick Tick, FActorComponentTickFunction* Function)
{
	Super::TickComponent(Dt, Tick, Function);
	if (!Boss() || Boss()->IsDead() || bPaused || bHolding || Dt <= 0 || !FMath::IsFinite(Dt)) return;
	// Sample routes at a bounded step so curvature, damping and trail quality
	// remain stable at low frame rates. Only the route owns progress/end conditions.
	for (float Left = Dt; Left > SMALL_NUMBER;)
	{
		float Step = FMath::Min(Left, 1.f / 120.f);
		Left -= Step;
		FVector Position = Motion.GetBody().GetLocation();
		bool bFinished = false;
		Motion.AccelerationIntent = 0;
		if (bFlying && bUsingRoute)
		{
			const auto* Spline = ActiveRoute.IsValid() ? ActiveRoute->Spline.Get() : nullptr;
			if (!IsValid(Spline) || Spline->GetSplineLength() <= 1.f)
			{
				StopFlight();
				OnFlightFinished.Broadcast();
				break;
			}
			const float Length = Spline->GetSplineLength();
			const float DesiredSpeed = FMath::Max(0.f, RouteSpeed);
			Motion.AccelerationIntent = FMath::Clamp((DesiredSpeed - CurrentRouteSpeed) / 6000.f, 0.f, 1.f);
			if (DesiredSpeed <= CurrentRouteSpeed) CurrentRouteSpeed = DesiredSpeed;
			else if (Motion.PowerStroke >= .7f * Motion.AccelerationIntent)
			{
				// The visible preparation leads acceleration; the downstroke then
				// contributes more thrust than the recovery stroke.
				const float Downstroke = FMath::Max(0.f, -FMath::Cos(Motion.Phase - 1.8f));
				const float Thrust = (.15f + .85f * Motion.PowerStroke) * (.35f + .65f * Downstroke);
				CurrentRouteSpeed = FMath::FInterpConstantTo(CurrentRouteSpeed, DesiredSpeed, Step, FMath::Max(1.f, RouteAcceleration) * Thrust);
			}
			RouteDistance += CurrentRouteSpeed * Step;
			bFinished = !Spline->IsClosedLoop() && RouteDistance >= Length;
			RouteDistance = Spline->IsClosedLoop() ? FMath::Fmod(RouteDistance, double(Length)) : FMath::Min(RouteDistance, double(Length));
			Position = Spline->GetLocationAtDistanceAlongSpline(float(RouteDistance), ESplineCoordinateSpace::World);
			FlightSeconds += Step;
		}
		else if (bFlying)
		{
			Step = FMath::Min(Step, Path.GetReleaseSeconds() - FlightSeconds);
			FlightSeconds += Step;
			Position = ChoreographyFrame.TransformPosition(Path.FlightPosition(FlightSeconds / Path.GetMorphDuration()));
			// Preserve the accepted reference path. Look ahead for acceleration so
			// its presentation can anticipate the motion without a timestamp gate.
			const float Future = FMath::Min(FlightSeconds + .4f, Path.GetReleaseSeconds());
			const float Before = FMath::Max(0.f, Future - .02f);
			const FVector FutureDelta = ChoreographyFrame.TransformVector(Path.FlightPosition(Future / Path.GetMorphDuration()) - Path.FlightPosition(Before / Path.GetMorphDuration()));
			Motion.AccelerationIntent = FMath::Clamp((float(FutureDelta.Size()) / FMath::Max(.001f, Future - Before) - Motion.GetSpeed()) / 6000.f, 0.f, 1.f);
			bFinished = FlightSeconds >= Path.GetReleaseSeconds();
		}
		Motion.Update(Position, Step);
		if (bFinished)
		{
			UpdatePose();
			StopFlight();
			OnFlightFinished.Broadcast();
			break;
		}
	}
	UpdatePose();
}

void UCoreMorphFlightComponent::UpdatePose()
{
	if (!Layout || Layout->Pieces.Num() != Pieces.Num()) return;
	const FTransform& Body = Motion.GetBody();
	if (bHaveFrame)
		GetOwner()->SetActorLocationAndRotation(Body.GetLocation(), FRotator(0, Body.Rotator().Yaw, 0), false, nullptr, ETeleportType::TeleportPhysics);
	const FVector Scale = ChoreographyFrame.GetScale3D() * Path.MantaScale;
	for (int32 I = 0; I < Pieces.Num(); ++I)
		if (IsValid(Pieces[I])) Pieces[I]->SetWorldTransform(Motion.PiecePose(Layout->Pieces[I], Scale));
}
