#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "EnvironmentHazardInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UEnvironmentHazard : public UInterface
{
	GENERATED_BODY()
};

/**
 * 环境危险物接口：所有可对角色造成持续伤害的危险区（火焰、毒气等）实现此接口。
 * InflictDamage 留空，由具体子类（AFlameHazard 等）在功能开发时实现。
 */
class THEMANTEST_API IEnvironmentHazard
{
	GENERATED_BODY()
public:
	// 对目标施加伤害（留空：待后续功能实现 GAS Effect 逻辑）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hazard")
	void InflictDamage(AActor* Target);

	// 危险区当前是否处于激活状态（未被压制）
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Hazard")
	bool IsHazardActive();
};
