#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "HumanoidAIController.generated.h"

class UBehaviorTree;
class UAIPerceptionComponent;

UCLASS()
class THEMANTEST_API AHumanoidAIController : public AAIController
{
	GENERATED_BODY()

public:
	AHumanoidAIController();

	// 黑板 key 名称，供 C++ 和 BT Task 共用
	static const FName BB_TargetActor;
	static const FName BB_LastKnownPlayerLocation;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;

	// 在蓝图子类（BP_HumanoidAIController）里指定 BT 资产
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

private:
	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
};
