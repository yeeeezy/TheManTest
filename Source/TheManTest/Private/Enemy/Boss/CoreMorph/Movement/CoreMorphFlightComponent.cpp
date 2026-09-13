#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"
#include "Components/StaticMeshComponent.h"
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
	Path.Layout = Owner->VisualLayout;
	if (!Path.Layout) return;
	for (const auto& Piece : Path.Layout->Pieces)
	{
		auto* Mesh = NewObject<UStaticMeshComponent>(Owner);
		Mesh->ComponentTags.Add(TEXT("CoreMorphVisual"));
		Mesh->SetupAttachment(Owner->GetRootComponent());
		Mesh->SetAbsolute(true, true, true);
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
	if (!bHaveFrame) ChoreographyFrame = Owner->GetActorTransform();
	UpdatePose();
}

void UCoreMorphFlightComponent::BeginPlay()
{
	Super::BeginPlay();
	ChoreographyFrame = GetOwner()->GetActorTransform();
	bHaveFrame = true;
	RebuildAssembly();
}

void UCoreMorphFlightComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Shutdown();
	Super::EndPlay(Reason);
}

bool UCoreMorphFlightComponent::CanStartFlight() const
{
	return Boss() && !Boss()->IsDead() && Path.Layout && Pieces.Num() == 154 && !bFlying && !bHolding;
}

bool UCoreMorphFlightComponent::StartFlight()
{
	if (!CanStartFlight()) return false;
	Path.SourcePositions.Reset();
	for (const auto& Piece : Pieces)
		Path.SourcePositions.Add((ChoreographyFrame.InverseTransformPosition(Piece->GetComponentLocation()) - Path.GetFormOffset(0)) / Path.MantaScale);
	Path.PrepareDive();
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
}

void UCoreMorphFlightComponent::ResetPreview()
{
	StopFlight();
	bHolding = false;
	FlightSeconds = IdleSeconds = 0;
	Path.SourcePositions.Reset();
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
	if (!Boss() || Boss()->IsDead() || bPaused || Dt <= 0) return;
	if (bFlying) FlightSeconds = FMath::Min(FlightSeconds + Dt, Path.GetReleaseSeconds());
	else if (!bHolding) IdleSeconds += Dt;
	UpdatePose();
	if (bFlying && FlightSeconds >= Path.GetReleaseSeconds())
	{
		StopFlight();
		OnFlightFinished.Broadcast();
	}
}

void UCoreMorphFlightComponent::UpdatePose()
{
	if (!Path.Layout || Path.Layout->Pieces.Num() != Pieces.Num()) return;
	const bool bFlightPose = bFlying || bHolding;
	const float T = FlightSeconds / Path.GetMorphDuration();
	FTransform Body = bFlightPose ? Path.DivePose(T) : FTransform(Path.GetFormOffset(0));
	Body *= ChoreographyFrame;
	if (bHaveFrame)
		GetOwner()->SetActorLocationAndRotation(Body.GetLocation(), FRotator(0, Body.Rotator().Yaw, 0), false, nullptr, ETeleportType::TeleportPhysics);
	for (int32 I = 0; I < Pieces.Num(); ++I)
	{
		if (!IsValid(Pieces[I])) continue;
		FTransform Pose;
		if (bFlightPose) Pose = Path.SourcePose(I, T);
		else
		{
			const auto& Piece = Path.Layout->Pieces[I];
			FVector V = Piece.Position;
			const float U = FMath::Clamp(float(FMath::Abs(V.Y) / 2540.), 0.f, 1.f);
			const float EaseIn = FMath::Clamp(IdleSeconds / 1.5f, 0.f, 1.f);
			const float Blend = EaseIn * EaseIn * EaseIn * (EaseIn * (EaseIn * 6 - 15) + 10);
			if (Piece.Kind == TEXT("Wing")) V.Z += 150 * U * U * FMath::Sin(IdleSeconds * 1.05f - U * 1.2f) * Blend;
			if (Piece.Kind == TEXT("Tail")) V.Y += 55 * Piece.Order * Piece.Order * FMath::Sin(IdleSeconds * .9f - Piece.Order * 3) * Blend;
			Pose = FTransform(FQuat::Identity, V * Path.MantaScale + Path.GetFormOffset(0), FVector(Path.MantaScale));
		}
		Pieces[I]->SetWorldTransform(Pose * ChoreographyFrame);
	}
}
