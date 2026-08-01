#include "Enemy/Humanoid/BTTask_ResumeNearestPatrol.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "AIController.h"

UBTTask_ResumeNearestPatrol::UBTTask_ResumeNearestPatrol()
{
	NodeName = TEXT("Resume Nearest Patrol");
}

EBTNodeResult::Type UBTTask_ResumeNearestPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = Cast<AAIController>(OwnerComp.GetAIOwner());
	if (!AIC) return EBTNodeResult::Failed;

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(AIC->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->ResumeNearestPatrol();
	return EBTNodeResult::Succeeded;
}
