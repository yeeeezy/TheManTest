#pragma once
#include "AIController.h"
#include "CoreMorphAIController.generated.h"
UCLASS()
class THEMANTEST_API ACoreMorphAIController : public AAIController
{
 GENERATED_BODY()
public:
 void EnsureTree();
 void StopCombat();
 virtual void OnUnPossess() override;
private:
 bool bStarted=false;
};
