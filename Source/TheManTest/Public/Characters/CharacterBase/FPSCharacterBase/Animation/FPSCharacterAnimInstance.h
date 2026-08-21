#pragma once

#include "CoreMinimal.h"
#include "Characters/_Shared/Animation/BaseLocomotionAnimInstance.h"
#include "FPSCharacterAnimInstance.generated.h"

class UAnimMontage;
class USkeletalMeshComponent;

// 玩家角色（AFPSCharacterBase）的动画实例。挂在 GetMesh() 上，驱动身体/影子/腿三件套共享的全身姿势。
// 当前 locomotion 采用模板式基础方案：Idle 与 Walk/Run BlendSpace 直接按 Speed/Direction 混合，
// 不再做专门停步动画、脚相位或 StopAnimIndex 选择。
UCLASS()
class THEMANTEST_API UFPSCharacterAnimInstance : public UBaseLocomotionAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// CharacterMesh0's AnimGraph reads this as the source of the upper-body Copy Pose.
	// It is assigned from the owning AFPSCharacterBase and intentionally excludes the
	// first-person component transform: Copy Pose transfers local bone transforms only.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "First Person Pose")
	TObjectPtr<USkeletalMeshComponent> FirstPersonPoseSource;

	// 保存当前最终姿势，供武器 Linked Layer 切换时做短时连续过渡。
	void CaptureWeaponTransitionPose();
	void StartWeaponTransition(UAnimMontage* MontageToPlay);
	void CompleteWeaponTransition();

protected:
	// 加速度（移动意图）方向角，-180~180，与 Direction 同坐标系。普通 Locomotion/Lean 可选使用。
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float AccelDirection = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHasAcceleration = false;

	// Camera-relative first-person sway is authored once at the end of the shared
	// body graph, after the weapon linked layers. Both CharacterMesh0 and
	// ArmsViewMesh therefore receive the same final skeletal correction.
	UPROPERTY(BlueprintReadWrite, Category = "Viewmodel Sway")
	double Lean_Sides_Amount = 0.0;

	UPROPERTY(BlueprintReadWrite, Category = "Viewmodel Sway")
	double Look_Up_Amount = 0.0;

	// AnimGraph 在输出端用它从命名 Pose Snapshot 混合到当前实时姿势。
	UPROPERTY(BlueprintReadOnly, Category = "Weapon Transition")
	float WeaponTransitionAlpha = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Transition", meta = (ClampMin = "0.01"))
	float WeaponTransitionDuration = 0.08f;

private:
	bool bWeaponTransitionActive = false;
	bool bHoldWeaponTransitionFirstUpdate = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> PendingWeaponTransitionMontage;

};
