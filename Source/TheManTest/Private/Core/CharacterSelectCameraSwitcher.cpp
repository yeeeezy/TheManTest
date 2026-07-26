#include "Core/CharacterSelectCameraSwitcher.h"

#include "Camera/CameraActor.h"
#include "CineCameraActor.h"
#include "CineCameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogCharacterSelectCamera, Log, All);

ACharacterSelectCameraSwitcher::ACharacterSelectCameraSwitcher()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACharacterSelectCameraSwitcher::BeginPlay()
{
	Super::BeginPlay();

	CacheBaseCameraTransforms();
	CreateCameraRig();
	ApplyCameraView(bStartInNearCamera, 0.0f);
}

void ACharacterSelectCameraSwitcher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMouseParallax(DeltaSeconds);
	UpdateRigTransform(DeltaSeconds);
	SyncRigCameraSettings();
}

void ACharacterSelectCameraSwitcher::ToggleCameraView()
{
	ApplyCameraView(!bUsingNearCamera, BlendTime);
}

void ACharacterSelectCameraSwitcher::SetNearCameraView()
{
	ApplyCameraView(true, BlendTime);
}

void ACharacterSelectCameraSwitcher::SetFarCameraView()
{
	ApplyCameraView(false, BlendTime);
}

void ACharacterSelectCameraSwitcher::ApplyCameraView(bool bNearCamera, float OverrideBlendTime)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	ACameraActor* TargetCamera = bNearCamera ? NearCamera : FarCamera;
	if (!TargetCamera || !CameraRig)
	{
		UE_LOG(LogCharacterSelectCamera, Warning, TEXT("%s missing %s camera."),
			*GetName(), bNearCamera ? TEXT("Near") : TEXT("Far"));
		return;
	}

	bUsingNearCamera = bNearCamera;
	CurrentParallaxOffset = FVector::ZeroVector;
	if (!bHasAppliedInitialView)
	{
		const FTransform& BaseTransform = GetCurrentBaseTransform();
		RigBaseLocation = BaseTransform.GetLocation();
		RigVelocity = FVector::ZeroVector;
		if (CameraRig)
		{
			CameraRig->SetActorLocation(RigBaseLocation);
			CameraRig->SetActorRotation(BaseTransform.GetRotation());
		}
		bHasAppliedInitialView = true;
	}
	else
	{
		StartSwitchSpring(bNearCamera);
	}
	SyncRigCameraSettings();

	PC->SetViewTargetWithBlend(CameraRig, OverrideBlendTime, BlendFunction, BlendExp);
	OnCameraViewChanged(bUsingNearCamera);
}

void ACharacterSelectCameraSwitcher::CacheBaseCameraTransforms()
{
	if (FarCamera)
	{
		FarCameraBaseTransform = FarCamera->GetActorTransform();
	}
	if (NearCamera)
	{
		NearCameraBaseTransform = NearCamera->GetActorTransform();
	}
}

void ACharacterSelectCameraSwitcher::CreateCameraRig()
{
	if (CameraRig || !GetWorld())
	{
		return;
	}

	const FTransform& InitialTransform = bStartInNearCamera ? NearCameraBaseTransform : FarCameraBaseTransform;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CameraRig = GetWorld()->SpawnActor<ACineCameraActor>(
		ACineCameraActor::StaticClass(), InitialTransform, SpawnParams);

	if (CameraRig)
	{
		CameraRig->SetActorHiddenInGame(true);
		RigBaseLocation = InitialTransform.GetLocation();
		SyncRigCameraSettings();
	}
}

void ACharacterSelectCameraSwitcher::SyncRigCameraSettings() const
{
	if (!CameraRig)
	{
		return;
	}

	const ACineCameraActor* SourceCineCamera = Cast<ACineCameraActor>(GetCurrentCamera());
	if (!SourceCineCamera)
	{
		return;
	}

	const UCineCameraComponent* SourceComponent = SourceCineCamera->GetCineCameraComponent();
	UCineCameraComponent* RigComponent = CameraRig->GetCineCameraComponent();
	if (!SourceComponent || !RigComponent)
	{
		return;
	}

	RigComponent->SetFilmback(SourceComponent->Filmback);
	RigComponent->SetLensSettings(SourceComponent->LensSettings);
	RigComponent->SetFocusSettings(SourceComponent->FocusSettings);
	RigComponent->SetCropSettings(SourceComponent->CropSettings);
	RigComponent->SetCurrentFocalLength(SourceComponent->CurrentFocalLength);
	RigComponent->SetCurrentAperture(SourceComponent->CurrentAperture);
	RigComponent->ExposureMethod = SourceComponent->ExposureMethod;
	RigComponent->bOverride_CustomNearClippingPlane = SourceComponent->bOverride_CustomNearClippingPlane;
	RigComponent->SetCustomNearClippingPlane(SourceComponent->CustomNearClippingPlane);
}

void ACharacterSelectCameraSwitcher::UpdateMouseParallax(float DeltaSeconds)
{
	if (!bEnableMouseParallax)
	{
		CurrentParallaxOffset = FMath::VInterpTo(
			CurrentParallaxOffset, FVector::ZeroVector, DeltaSeconds, MouseParallaxInterpSpeed);
		return;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !CameraRig)
	{
		return;
	}

	int32 ViewportSizeX = 0;
	int32 ViewportSizeY = 0;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);
	if (ViewportSizeX <= 0 || ViewportSizeY <= 0)
	{
		return;
	}

	float MouseX = 0.0f;
	float MouseY = 0.0f;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	const float NormalizedX = FMath::Clamp((MouseX / static_cast<float>(ViewportSizeX) - 0.5f) * 2.0f, -1.0f, 1.0f);
	const float NormalizedY = FMath::Clamp((0.5f - MouseY / static_cast<float>(ViewportSizeY)) * 2.0f, -1.0f, 1.0f);
	const float DirectionScale = bInvertMouseParallax ? -1.0f : 1.0f;
	const float FocalLengthScale = GetCurrentFocalLengthScale();

	const FTransform& BaseTransform = GetCurrentBaseTransform();
	const FVector TargetOffset =
		BaseTransform.GetUnitAxis(EAxis::Y) * NormalizedX * MouseParallaxHorizontalStrength * FocalLengthScale * DirectionScale +
		BaseTransform.GetUnitAxis(EAxis::Z) * NormalizedY * MouseParallaxVerticalStrength * FocalLengthScale * DirectionScale;

	CurrentParallaxOffset = FMath::VInterpTo(
		CurrentParallaxOffset, TargetOffset, DeltaSeconds, MouseParallaxInterpSpeed);
}

void ACharacterSelectCameraSwitcher::UpdateRigTransform(float DeltaSeconds)
{
	if (!CameraRig)
	{
		return;
	}

	const FTransform& BaseTransform = GetCurrentBaseTransform();
	const FVector TargetBaseLocation = BaseTransform.GetLocation();
	const FVector Displacement = TargetBaseLocation - RigBaseLocation;
	const FVector Acceleration = Displacement * SwitchSpringStrength - RigVelocity * SwitchSpringDamping;

	RigVelocity += Acceleration * DeltaSeconds;
	RigBaseLocation += RigVelocity * DeltaSeconds;

	const FVector FinalLocation = RigBaseLocation + CurrentParallaxOffset;
	const FRotator NewRotation = FMath::RInterpTo(
		CameraRig->GetActorRotation(), BaseTransform.GetRotation().Rotator(), DeltaSeconds, SwitchRotationInterpSpeed);

	CameraRig->SetActorLocation(FinalLocation);
	CameraRig->SetActorRotation(NewRotation);
}

void ACharacterSelectCameraSwitcher::StartSwitchSpring(bool bNearCamera)
{
	if (!bEnableSwitchOvershoot)
	{
		return;
	}

	const FVector SourceLocation = bNearCamera
		? FarCameraBaseTransform.GetLocation()
		: NearCameraBaseTransform.GetLocation();
	const FVector TargetLocation = bNearCamera
		? NearCameraBaseTransform.GetLocation()
		: FarCameraBaseTransform.GetLocation();

	const FVector Travel = TargetLocation - SourceLocation;
	const float TravelDistance = Travel.Size();
	if (TravelDistance <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	const float OvershootAmount = FMath::Min(
		SwitchOvershootDistance + TravelDistance * SwitchOvershootDistanceRatio,
		MaxSwitchOvershootDistance);

	RigVelocity += Travel.GetSafeNormal() * OvershootAmount * SwitchOvershootReturnSpeed;
}

ACameraActor* ACharacterSelectCameraSwitcher::GetCurrentCamera() const
{
	return bUsingNearCamera ? NearCamera : FarCamera;
}

const FTransform& ACharacterSelectCameraSwitcher::GetCurrentBaseTransform() const
{
	return bUsingNearCamera ? NearCameraBaseTransform : FarCameraBaseTransform;
}

float ACharacterSelectCameraSwitcher::GetCurrentFocalLengthScale() const
{
	if (!bScaleParallaxByFocalLength)
	{
		return 1.0f;
	}

	const ACineCameraActor* CineCamera = Cast<ACineCameraActor>(GetCurrentCamera());
	const UCineCameraComponent* CineCameraComponent = CineCamera ? CineCamera->GetCineCameraComponent() : nullptr;
	if (!CineCameraComponent || CineCameraComponent->CurrentFocalLength <= KINDA_SMALL_NUMBER)
	{
		return 1.0f;
	}

	const float RawScale = ReferenceFocalLength / CineCameraComponent->CurrentFocalLength;
	return FMath::Clamp(RawScale, MinFocalLengthScale, MaxFocalLengthScale);
}
