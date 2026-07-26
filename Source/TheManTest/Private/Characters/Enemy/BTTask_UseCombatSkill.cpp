#include "Characters/Enemy/BTTask_UseCombatSkill.h"
#include "Characters/Enemy/EnemyBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_UseCombatSkill::UBTTask_UseCombatSkill()
{
	NodeName = TEXT("Use Combat Skill");
}

EBTNodeResult::Type UBTTask_UseCombatSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AIC ? Cast<AEnemyBase>(AIC->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !BB) { return EBTNodeResult::Failed; }

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey));
	if (!Target) { return EBTNodeResult::Failed; }

	return Enemy->UseRandomSkill(Target, Range) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
