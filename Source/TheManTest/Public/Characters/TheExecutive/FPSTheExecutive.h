#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "FPSTheExecutive.generated.h"

UCLASS()
class THEMANTEST_API AFPSTheExecutive : public AFPSCharacterBase
{
	GENERATED_BODY()
public:
 AFPSTheExecutive();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Executive|Drone")
 TObjectPtr<class UExecutiveDroneComponent> DroneCompanion;
};
