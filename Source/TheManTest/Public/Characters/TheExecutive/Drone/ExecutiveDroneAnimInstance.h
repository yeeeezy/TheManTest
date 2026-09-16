#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ExecutiveDroneAnimInstance.generated.h"
UCLASS(Transient,Blueprintable)
class THEMANTEST_API UExecutiveDroneAnimInstance : public UAnimInstance
{
 GENERATED_BODY()
public:
 virtual void NativeUpdateAnimation(float DeltaSeconds) override;
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Drone") int32 FlightPose = 0;
};
