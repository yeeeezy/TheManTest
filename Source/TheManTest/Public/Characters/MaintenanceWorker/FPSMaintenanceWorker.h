#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "FPSMaintenanceWorker.generated.h"

UCLASS()
class THEMANTEST_API AFPSMaintenanceWorker : public AFPSCharacterBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
