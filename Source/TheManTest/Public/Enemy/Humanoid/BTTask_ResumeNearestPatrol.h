#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ResumeNearestPatrol.generated.h"

UCLASS()
class THEMANTEST_API UBTTask_ResumeNearestPatrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ResumeNearestPatrol();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
