#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "HumanoidAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;
class AHumanoidEnemy;

UCLASS()
class THEMANTEST_API AHumanoidAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHumanoidAIController();

	// 黑板 key 名称，供 C++ 和 BT Task 共用
	static const FName BB_TargetActor;
	static const FName BB_LastKnownPlayerLocation;

	// 纯几何战术落点计算：保持目标距离环带，并叠加切向移动以避免直线追击。
	UFUNCTION(BlueprintPure, Category = "AI|Combat Movement")
	FVector CalculateCombatMoveDestination(const FVector& EnemyLocation,
		const FVector& TargetLocation, float StrafeSign) const;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual FPathFollowingRequestResult MoveTo(const FAIMoveRequest& MoveRequest,
		FNavPathSharedPtr* OutPath = nullptr) override;

	// 在蓝图子类（BP_HumanoidAIController）里指定 BT 资产
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.0"))
	float PreferredCombatDistance = 700.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.0"))
	float CombatDistanceTolerance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.0"))
	float CombatStrafeDistance = 350.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.05"))
	float CombatMoveDecisionMinInterval = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.05"))
	float CombatMoveDecisionMaxInterval = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Combat Movement", meta = (ClampMin = "0.0"))
	float CombatMoveAcceptanceRadius = 65.f;

private:
	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void UpdateCombatMovement(AHumanoidEnemy& Enemy, AActor& Target);

	float NextCombatMoveDecisionTime = 0.f;
	float CurrentStrafeSign = 1.f;
	FVector CurrentCombatDestination = FVector::ZeroVector;
	bool bUseDirectCombatMovement = false;
};
