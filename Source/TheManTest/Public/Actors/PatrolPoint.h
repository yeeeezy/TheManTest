#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPoint.generated.h"

// 场景中放置的巡逻路点，HumanoidEnemy 按数组顺序循环到达
UCLASS()
class THEMANTEST_API APatrolPoint : public AActor
{
	GENERATED_BODY()

public:
	APatrolPoint();

	// 到达后原地等待的秒数，0 表示立即前往下一点
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Patrol")
	float WaitTime = 0.f;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> Sphere;
};
