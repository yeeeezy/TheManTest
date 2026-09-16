#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "ExecutiveDroneMovementComponent.generated.h"
UCLASS(ClassGroup=Movement,meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UExecutiveDroneMovementComponent : public UPawnMovementComponent
{
 GENERATED_BODY()
public:
 UExecutiveDroneMovementComponent();
 virtual void TickComponent(float DeltaSeconds,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Flight") float MaxSpeed=950.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Flight") float Acceleration=1600.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Drone|Flight") float PositionGain=3.f;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone|Flight") bool bBlocked=false;
};
