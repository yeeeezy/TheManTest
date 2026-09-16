#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExecutiveDroneComponent.generated.h"
class AExecutiveDrone;
UCLASS(ClassGroup=Executive,meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UExecutiveDroneComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UExecutiveDroneComponent();
 virtual void TickComponent(float DeltaSeconds,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Drone") TSubclassOf<AExecutiveDrone> DroneClass;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Drone") bool bLobbyPresentation=false;
 UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="Drone") TObjectPtr<AExecutiveDrone> Drone;
 void ReleaseDrone();
};
