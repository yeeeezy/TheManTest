#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "ExecutiveDroneAIController.generated.h"
UCLASS()
class THEMANTEST_API AExecutiveDroneAIController : public AAIController
{
 GENERATED_BODY()
public:
 AExecutiveDroneAIController();
 virtual void OnPossess(APawn* InPawn) override;
 virtual void OnUnPossess() override;
};
