#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
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
		PC->SetViewTarget(this);
	}
}

void ACoreMorphFlightReview::Tick(float Dt)
{
	Super::Tick(Dt);
	if (!IsValid(Boss)) return;
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
		GEngine->AddOnScreenDebugMessage(uint64(GetUniqueID()), 0.f, FColor::White,
			FString::Printf(TEXT("FLIGHT REVIEW | V: Play | R: Reset | P: Pause | F: Camera | %.2f s"), Boss->Flight->GetFlightSeconds()));
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
void ACoreMorphFlightReview::ToggleCamera()
{
	if (auto* PC = GetWorld()->GetFirstPlayerController())
	{
		bFollow = !bFollow;
		PC->SetViewTarget(bFollow || !PC->GetPawn() ? this : static_cast<AActor*>(PC->GetPawn()));
	}
}
