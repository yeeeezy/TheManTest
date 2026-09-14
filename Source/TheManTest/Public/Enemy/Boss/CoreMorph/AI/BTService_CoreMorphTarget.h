#pragma once
#include "BehaviorTree/BTService.h"
#include "BTService_CoreMorphTarget.generated.h"
UCLASS()
class THEMANTEST_API UBTService_CoreMorphTarget : public UBTService
{
 GENERATED_BODY()
public:
 UBTService_CoreMorphTarget();
 virtual void TickNode(UBehaviorTreeComponent& BT,uint8* Memory,float Dt) override;
};
