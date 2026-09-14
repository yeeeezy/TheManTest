#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "InputCoreTypes.h"

ACoreMorphFlightReview::ACoreMorphFlightReview()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	GetCameraComponent()->SetFieldOfView(60.f);
	GetCameraComponent()->bConstrainAspectRatio = false;
	auto& PP = GetCameraComponent()->PostProcessSettings;
	PP.bOverride_AutoExposureMethod = true;
	PP.AutoExposureMethod = AEM_Manual;
	PP.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
	PP.AutoExposureApplyPhysicalCameraExposure = false;
	PP.bOverride_AutoExposureBias = true;
	PP.AutoExposureBias = 0;
}

void ACoreMorphFlightReview::BeginPlay()
{
	Super::BeginPlay();
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		EnableInput(PC);
		InputComponent->BindKey(EKeys::V, IE_Pressed, this, &ThisClass::PlayFlight);
		InputComponent->BindKey(EKeys::R, IE_Pressed, this, &ThisClass::ResetFlight);
		InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ThisClass::PauseFlight);
		InputComponent->BindKey(EKeys::F, IE_Pressed, this, &ThisClass::ToggleCamera);
		InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ThisClass::SlowRoll);
		InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ThisClass::FastRoll);
		if (!Routes.IsEmpty())
		{
			InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::RouteOne);
			InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::RouteTwo);
			InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::RouteThree);
			bStartFirstRoute = true;
		}
		PC->SetViewTarget(this);
	}
}

void ACoreMorphFlightReview::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!IsValid(Boss)) return;
	// Wait for the boss's BeginPlay to initialize and grant its GA, regardless
	// of serialized actor order in the review map.
	if (bStartFirstRoute && Boss->HasActorBegunPlay())
	{
		bStartFirstRoute = false;
		SelectRoute(0);
	}
	FVector Target, Extent;
	Boss->GetActorBounds(false, Target, Extent);
	int32 Width = 16, Height = 9;
	if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->GetViewportSize(Width, Height);
	const float Aspect = Height > 0 ? float(Width) / Height : 16.f / 9.f;
	const float HalfVerticalFOV = FMath::Atan(FMath::Tan(FMath::DegreesToRadians(30.f)) / FMath::Max(1.f, Aspect));
	const float Distance = FMath::Max(14000.f, float(Extent.Size()) * 1.15f / FMath::Sin(HalfVerticalFOV));
	SetActorLocation(Target + FVector(-8500, -11500, 5000).GetSafeNormal() * Distance);
	SetActorRotation((Target - GetActorLocation()).Rotation());
	if (GEngine)
	{
		const FString RouteText = Routes.IsValidIndex(SelectedRoute)
			? FString::Printf(TEXT("ROUTE %d: %s | 1/2/3: Switch & Play\n"), SelectedRoute + 1, *Routes[SelectedRoute].Label) : FString();
		GEngine->AddOnScreenDebugMessage(uint64(GetUniqueID()), 0.f, FColor::White,
			RouteText + FString::Printf(TEXT("Q: Slow roll | E: Fast roll\nFLIGHT REVIEW | V: Play | R: Reset | P: Pause | F: Camera | %.2f s"), Boss->Flight->GetFlightSeconds()));
	}
}

bool ACoreMorphFlightReview::SelectRoute(int32 Index)
{
	if (!IsValid(Boss) || Boss->IsDead() || !Boss->HasActorBegunPlay()
		|| !Routes.IsValidIndex(Index) || !IsValid(Routes[Index].Route)) return false;
	Boss->ResetFlightPreview();
	Boss->Flight->FlightRoute = Routes[Index].Route;
	Boss->Flight->RouteSpeed = Routes[Index].Speed;
	SelectedRoute = Index;
	bFollow = true;
	if (auto* PC = GetWorld()->GetFirstPlayerController()) PC->SetViewTarget(this);
	return Boss->StartFlightPreview();
}

void ACoreMorphFlightReview::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* PC = GetWorld()->GetFirstPlayerController()) DisableInput(PC);
	if (GEngine) GEngine->RemoveOnScreenDebugMessage(GetUniqueID());
	Super::EndPlay(Reason);
}

void ACoreMorphFlightReview::PlayFlight() { if (IsValid(Boss)) Boss->StartFlightPreview(); }
void ACoreMorphFlightReview::ResetFlight() { if (IsValid(Boss)) Boss->ResetFlightPreview(); }
void ACoreMorphFlightReview::PauseFlight() { if (IsValid(Boss)) Boss->Flight->SetPaused(!Boss->Flight->IsPaused()); }
void ACoreMorphFlightReview::SlowRoll() { if (IsValid(Boss)) Boss->Flight->RequestRoll(false); }
void ACoreMorphFlightReview::FastRoll() { if (IsValid(Boss)) Boss->Flight->RequestRoll(true); }
void ACoreMorphFlightReview::ToggleCamera()
{
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		bFollow = !bFollow;
		PC->SetViewTarget(bFollow || !PC->GetPawn() ? this : static_cast<AActor*>(PC->GetPawn()));
	}
}
