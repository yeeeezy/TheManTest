#pragma once
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"
#include "BTTask_CoreMorphAction.generated.h"
UENUM()
enum class ECoreMorphTreeAction : uint8 {Idle,Approach,Face,Flight,Reassemble,PhaseSkill,FarPhaseSkill};
UCLASS()
class THEMANTEST_API UBTTask_CoreMorphAction : public UBTTaskNode
{
 GENERATED_BODY()
public:
 UBTTask_CoreMorphAction();
 UPROPERTY(EditAnywhere,Category="CoreMorph") ECoreMorphTreeAction Action=ECoreMorphTreeAction::Idle;
 virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& BT,uint8* Memory) override;
 virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& BT,uint8* Memory) override;
 virtual void TickTask(UBehaviorTreeComponent& BT,uint8* Memory,float Dt) override;
private:
 FGameplayAbilitySpecHandle AbilityHandle;
 float Elapsed=0;
};
