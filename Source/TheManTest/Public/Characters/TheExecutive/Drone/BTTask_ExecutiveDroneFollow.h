#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ExecutiveDroneFollow.generated.h"
UCLASS()
class THEMANTEST_API UBTTask_ExecutiveDroneFollow : public UBTTaskNode
{
 GENERATED_BODY()
public:
 UBTTask_ExecutiveDroneFollow();
 virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory) override;
 virtual void TickTask(UBehaviorTreeComponent& OwnerComp,uint8* NodeMemory,float DeltaSeconds) override;
};
