#include "Core/CharacterSelectCameraSwitcher.h"
#include "Core/CharacterSelectPlayerController.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

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

	CacheBaseCameraTransforms();
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

	if (bHasAppliedInitialView && bUsingNearCamera == bNearCamera)
	{
		return;
	}
	TransitionStart = CameraRig->GetActorTransform();
	// Include the visible mouse offset in the departure pose, so a click never snaps.
	CurrentParallaxOffset = FVector::ZeroVector;
	const UCineCameraComponent* Lens = CameraRig->GetCineCameraComponent();
	StartFocalLength = Lens->CurrentFocalLength;
	StartFocusDistance = Lens->FocusSettings.ManualFocusDistance;
	StartAperture = Lens->CurrentAperture;
	bUsingNearCamera = bNearCamera;
	TransitionElapsed = 0.f;
	TransitionDuration = bHasAppliedInitialView ? FMath::Max(0.f, OverrideBlendTime) : 0.f;
	TransitionAlpha = TransitionDuration > 0.f ? 0.f : 1.f;
	if (TransitionDuration == 0.f)
	{
		RigBaseLocation = GetCurrentBaseTransform().GetLocation();
		CameraRig->SetActorTransform(GetCurrentBaseTransform());
	}
	bHasAppliedInitialView = true;
	SyncRigCameraSettings();
	// The rig itself interpolates; blending to the same ViewTarget does not move it.
	PC->SetViewTarget(CameraRig);
	OnCameraViewChanged(bUsingNearCamera);
}

void ACharacterSelectCameraSwitcher::CacheBaseCameraTransforms()
{
	if (FarCamera)
	{
		FarCameraBaseTransform = FarCamera->GetCameraComponent()->GetComponentTransform();
	}
	if (NearCamera)
	{
		NearCameraBaseTransform = NearCamera->GetCameraComponent()->GetComponentTransform();
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

	// The placed camera owns framing and grading. The rig only adds movement.
	RigComponent->ProjectionMode = SourceComponent->ProjectionMode;
	RigComponent->bConstrainAspectRatio = SourceComponent->bConstrainAspectRatio;
	RigComponent->bOverrideAspectRatioAxisConstraint = SourceComponent->bOverrideAspectRatioAxisConstraint;
	RigComponent->AspectRatioAxisConstraint = SourceComponent->AspectRatioAxisConstraint;
	RigComponent->bUseFieldOfViewForLOD = SourceComponent->bUseFieldOfViewForLOD;
	RigComponent->OrthoWidth = SourceComponent->OrthoWidth;
	RigComponent->OrthoNearClipPlane = SourceComponent->OrthoNearClipPlane;
	RigComponent->OrthoFarClipPlane = SourceComponent->OrthoFarClipPlane;
	RigComponent->bAutoCalculateOrthoPlanes = SourceComponent->bAutoCalculateOrthoPlanes;
	RigComponent->bUpdateOrthoPlanes = SourceComponent->bUpdateOrthoPlanes;
	RigComponent->Overscan = SourceComponent->Overscan;
	RigComponent->AsymmetricOverscan = SourceComponent->AsymmetricOverscan;
	RigComponent->bScaleResolutionWithOverscan = SourceComponent->bScaleResolutionWithOverscan;
	RigComponent->bCropOverscan = SourceComponent->bCropOverscan;
	RigComponent->PostProcessSettings = SourceComponent->PostProcessSettings;
	RigComponent->PostProcessBlendWeight = SourceComponent->PostProcessBlendWeight;
	RigComponent->SetFilmback(SourceComponent->Filmback);
	RigComponent->SetLensSettings(SourceComponent->LensSettings);
	FCameraFocusSettings Focus = SourceComponent->FocusSettings;
	// Focus on the displayed subject, not the scene camera's old fixed distance.
	// A focal plane uses forward depth, rather than distance to an off-centre subject.
	if (auto* PC = Cast<ACharacterSelectPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (const ALobbyCharacterBase* Character = PC->GetDisplayCharacter())
		{
			const UPrimitiveComponent* Subject = Character->DisplayMesh;
			bool bWeaponSubject = false;
			if (bUsingNearCamera)
			{
				if (Character->DisplayWeapon && Character->DisplayWeapon->GetStaticMesh() && Character->DisplayWeapon->IsVisible())
					Subject = Character->DisplayWeapon;
				else if (Character->DisplaySkeletalWeapon && Character->DisplaySkeletalWeapon->GetSkeletalMeshAsset() && Character->DisplaySkeletalWeapon->IsVisible())
					Subject = Character->DisplaySkeletalWeapon;
				bWeaponSubject = Subject && Subject != Character->DisplayMesh;
			}
			if (Subject)
			{
				const FBoxSphereBounds Bounds = Subject->CalcBounds(Subject->GetComponentTransform());
				FVector FocusPoint = Bounds.Origin;
				// Upper torso keeps the face and body near the character page's focal plane.
				if (!bWeaponSubject) FocusPoint.Z += Bounds.BoxExtent.Z * 0.35f;
				const float Depth = FVector::DotProduct(FocusPoint - RigComponent->GetComponentLocation(), RigComponent->GetForwardVector());
				if (Depth > 0.f)
				{
					Focus.FocusMethod = ECameraFocusMethod::Manual;
					Focus.FocusOffset = 0.f;
					Focus.ManualFocusDistance = Depth;
				}
			}
		}
	}
	Focus.ManualFocusDistance = FMath::Lerp(StartFocusDistance, Focus.ManualFocusDistance, TransitionAlpha);
	Focus.bSmoothFocusChanges = false;
	RigComponent->SetFocusSettings(Focus);
	RigComponent->SetCropSettings(SourceComponent->CropSettings);
	RigComponent->SetCurrentFocalLength(FMath::Lerp(StartFocalLength, SourceComponent->CurrentFocalLength, TransitionAlpha));
	RigComponent->SetCurrentAperture(FMath::Lerp(StartAperture, SourceComponent->CurrentAperture, TransitionAlpha));
	RigComponent->ExposureMethod = SourceComponent->ExposureMethod;
	RigComponent->bOverride_CustomNearClippingPlane = SourceComponent->bOverride_CustomNearClippingPlane;
	RigComponent->SetCustomNearClippingPlane(SourceComponent->CustomNearClippingPlane);
}

void ACharacterSelectCameraSwitcher::UpdateMouseParallax(float DeltaSeconds)
{
	if (!bEnableMouseParallax || TransitionAlpha < 1.f)
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
		CurrentParallaxOffset = FMath::VInterpTo(CurrentParallaxOffset, FVector::ZeroVector, DeltaSeconds, MouseParallaxInterpSpeed);
		return;
	}

	const float NormalizedX = FMath::Clamp((MouseX / static_cast<float>(ViewportSizeX) - 0.5f) * 2.0f, -1.0f, 1.0f);
	const float NormalizedY = FMath::Clamp((0.5f - MouseY / static_cast<float>(ViewportSizeY)) * 2.0f, -1.0f, 1.0f);
	const float DirectionScale = bInvertMouseParallax ? -1.0f : 1.0f;
	const float FocalLengthScale = GetCurrentFocalLengthScale() * (bUsingNearCamera ? NearParallaxScale : 1.f);

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
	TransitionElapsed += DeltaSeconds;
	const float T = TransitionDuration > 0.f ? FMath::Clamp(TransitionElapsed / TransitionDuration, 0.f, 1.f) : 1.f;
	TransitionAlpha = T * T * (3.f - 2.f * T);
	RigBaseLocation = FMath::Lerp(TransitionStart.GetLocation(), BaseTransform.GetLocation(), TransitionAlpha);
	const FQuat Rotation = FQuat::Slerp(TransitionStart.GetRotation(), BaseTransform.GetRotation(), TransitionAlpha);
	CameraRig->SetActorLocationAndRotation(RigBaseLocation + CurrentParallaxOffset, Rotation);
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
