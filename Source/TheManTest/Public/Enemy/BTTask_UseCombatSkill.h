#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Enemy/EnemyBase.h"   // EEnemySkillRange
#include "BTTask_UseCombatSkill.generated.h"

/**
 * UBTTask_UseCombatSkill
 * 通用敌人战斗技能节点：读黑板目标 → 调 AEnemyBase::UseRandomSkill，
 * 从「当前阶段」技能集里、本节点配置的距离档(近/中/远)随机放一个技能。
 *
 * 节点本身不绑定具体技能——技能由敌人的 PhaseSkillSets 数据决定。
 * BT 里同一个节点类用三次（Range 各填近/中/远），配距离 Decorator 分支即可。
 */
UCLASS()
class THEMANTEST_API UBTTask_UseCombatSkill : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_UseCombatSkill();
	UFUNCTION(BlueprintPure, Category = "CombatSkill|Cadence")
	float GetPostSkillDelay() const { return FMath::Max(3.f, PostSkillDelay); }

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	// 本节点对应的交战距离档（从当前阶段技能集的对应档随机放招）
	UPROPERTY(EditAnywhere, Category = "CombatSkill")
	EEnemySkillRange Range = EEnemySkillRange::Near;

	// 目标黑板键名（默认 TargetActor，与 AIController 写入的键一致）
	UPROPERTY(EditAnywhere, Category = "CombatSkill")
	FName TargetActorKey = TEXT("TargetActor");

	// 技能完全结束后仍需等待的行为树后摇。所有人形敌人的技能节奏统一由 BT 节点控制。
	UPROPERTY(EditAnywhere, Category = "CombatSkill|Cadence", meta = (ClampMin = "3.0"))
	float PostSkillDelay = 3.f;

private:
	TWeakObjectPtr<AEnemyBase> ActiveEnemy;
	float RemainingPostSkillDelay = 0.f;
	bool bWaitingForAbilityEnd = false;
};
