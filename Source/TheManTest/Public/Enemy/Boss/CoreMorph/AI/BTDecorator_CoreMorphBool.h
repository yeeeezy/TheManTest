#pragma once
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTDecorator_CoreMorphBool.generated.h"
UCLASS()
class THEMANTEST_API UBTDecorator_CoreMorphBool : public UBTDecorator_Blackboard
{GENERATED_BODY() public: void Configure(FName Key,EBTFlowAbortMode::Type AbortMode);};
