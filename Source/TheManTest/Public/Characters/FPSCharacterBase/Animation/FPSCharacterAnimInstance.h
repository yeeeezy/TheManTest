#pragma once

#include "CoreMinimal.h"
#include "Characters/Animation/BaseLocomotionAnimInstance.h"
#include "FPSCharacterAnimInstance.generated.h"

// 玩家角色（AFPSCharacterBase）的动画实例。挂在 GetMesh() 上，驱动身体/影子/腿三件套共享的全身姿势。
// 当前 locomotion 采用模板式基础方案：Idle 与 Walk/Run BlendSpace 直接按 Speed/Direction 混合，
// 不再做专门停步动画、脚相位或 StopAnimIndex 选择。
UCLASS()
class THEMANTEST_API UFPSCharacterAnimInstance : public UBaseLocomotionAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	// 加速度（移动意图）方向角，-180~180，与 Direction 同坐标系。普通 Locomotion/Lean 可选使用。
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float AccelDirection = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	bool bHasAcceleration = false;

};
