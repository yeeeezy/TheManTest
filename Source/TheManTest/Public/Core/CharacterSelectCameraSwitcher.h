#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Actor.h"
#include "CharacterSelectCameraSwitcher.generated.h"

class ACameraActor;
class ACineCameraActor;

UCLASS(Blueprintable)
class THEMANTEST_API ACharacterSelectCameraSwitcher : public AActor
{
	GENERATED_BODY()

public:
	ACharacterSelectCameraSwitcher();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Character Select|Camera")
	void ToggleCameraView();

	UFUNCTION(BlueprintCallable, Category = "Character Select|Camera")
	void SetNearCameraView();

	UFUNCTION(BlueprintCallable, Category = "Character Select|Camera")
	void SetFarCameraView();

	UFUNCTION(BlueprintPure, Category = "Character Select|Camera")
	bool IsUsingNearCamera() const { return bUsingNearCamera; }

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character Select|Camera")
	ACameraActor* FarCamera = nullptr;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character Select|Camera")
	ACameraActor* NearCamera = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Camera", meta = (ClampMin = "0.0"))
	float BlendTime = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Camera")
	bool bStartInNearCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax")
	bool bEnableMouseParallax = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxHorizontalStrength = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxVerticalStrength = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxInterpSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax")
	bool bInvertMouseParallax = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax")
	bool bScaleParallaxByFocalLength = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.1"))
	float ReferenceFocalLength = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MinFocalLengthScale = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MaxFocalLengthScale = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float NearParallaxScale = 0.5f;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character Select|Camera")
	void OnCameraViewChanged(bool bNearCamera);

private:
	UPROPERTY(VisibleInstanceOnly, Category = "Character Select|Camera")
	bool bUsingNearCamera = false;

	bool bHasAppliedInitialView = false;

	FTransform FarCameraBaseTransform;
	FTransform NearCameraBaseTransform;
	FVector CurrentParallaxOffset = FVector::ZeroVector;
	FVector RigBaseLocation = FVector::ZeroVector;
	FTransform TransitionStart;
	float TransitionElapsed = 0.f;
	float TransitionDuration = 0.f;
	float TransitionAlpha = 1.f;
	float StartFocalLength = 35.f;
	float StartFocusDistance = 100.f;
	float StartAperture = 2.8f;

	UPROPERTY(Transient)
	ACineCameraActor* CameraRig = nullptr;

	void ApplyCameraView(bool bNearCamera, float OverrideBlendTime);
	void CacheBaseCameraTransforms();
	void CreateCameraRig();
	void SyncRigCameraSettings() const;
	void UpdateMouseParallax(float DeltaSeconds);
	void UpdateRigTransform(float DeltaSeconds);

	ACameraActor* GetCurrentCamera() const;
	const FTransform& GetCurrentBaseTransform() const;
	float GetCurrentFocalLengthScale() const;
};
