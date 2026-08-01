#include "Enemy/BTTask_UseCombatSkill.h"
#include "Enemy/EnemyBase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"

UBTTask_UseCombatSkill::UBTTask_UseCombatSkill()
{
	NodeName = TEXT("Use Combat Skill + 3s Recovery");
	bCreateNodeInstance = true;
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_UseCombatSkill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	AEnemyBase* Enemy = AIC ? Cast<AEnemyBase>(AIC->GetPawn()) : nullptr;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!Enemy || !BB) { return EBTNodeResult::Failed; }

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey));
	if (!Target) { return EBTNodeResult::Failed; }

	if (!Enemy->UseRandomSkill(Target, Range)) return EBTNodeResult::Failed;

	ActiveEnemy = Enemy;
	bWaitingForAbilityEnd = true;
	RemainingPostSkillDelay = FMath::Max(3.f, PostSkillDelay);
	return EBTNodeResult::InProgress;
}

void UBTTask_UseCombatSkill::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AEnemyBase* Enemy = ActiveEnemy.Get();
	UAbilitySystemComponent* ASC = Enemy ? Enemy->GetAbilitySystemComponent() : nullptr;
	if (!Enemy || !ASC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (bWaitingForAbilityEnd)
	{
		for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
		{
			if (Spec.IsActive()) return;
		}
		bWaitingForAbilityEnd = false;
		return; // 后摇从技能结束后的下一帧开始，保证完整 3 秒间隔。
	}

	RemainingPostSkillDelay -= DeltaSeconds;
	if (RemainingPostSkillDelay <= 0.f)
	{
		ActiveEnemy.Reset();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_UseCombatSkill::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ActiveEnemy.Reset();
	bWaitingForAbilityEnd = false;
	RemainingPostSkillDelay = 0.f;
	return Super::AbortTask(OwnerComp, NodeMemory);
}
