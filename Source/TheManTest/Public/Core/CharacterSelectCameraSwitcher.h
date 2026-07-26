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
	float BlendTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Camera")
	TEnumAsByte<EViewTargetBlendFunction> BlendFunction = VTBlend_Cubic;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Camera", meta = (ClampMin = "0.0"))
	float BlendExp = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Camera")
	bool bStartInNearCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax")
	bool bEnableMouseParallax = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxHorizontalStrength = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxVerticalStrength = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Mouse Parallax", meta = (ClampMin = "0.0"))
	float MouseParallaxInterpSpeed = 6.0f;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot")
	bool bEnableSwitchOvershoot = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchOvershootDistance = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchOvershootDistanceRatio = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float MaxSwitchOvershootDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchOvershootReturnSpeed = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchSpringStrength = 28.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchSpringDamping = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Switch Overshoot", meta = (ClampMin = "0.0"))
	float SwitchRotationInterpSpeed = 4.0f;

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
	FVector RigVelocity = FVector::ZeroVector;

	UPROPERTY(Transient)
	ACineCameraActor* CameraRig = nullptr;

	void ApplyCameraView(bool bNearCamera, float OverrideBlendTime);
	void CacheBaseCameraTransforms();
	void CreateCameraRig();
	void SyncRigCameraSettings() const;
	void UpdateMouseParallax(float DeltaSeconds);
	void UpdateRigTransform(float DeltaSeconds);
	void StartSwitchSpring(bool bNearCamera);
	ACameraActor* GetCurrentCamera() const;
	const FTransform& GetCurrentBaseTransform() const;
	float GetCurrentFocalLengthScale() const;
};
