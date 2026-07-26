#pragma once

#include "CoreMinimal.h"
#include "Environment/Hazards/EnvironmentHazardBase.h"
#include "FlameHazard.generated.h"

/**
 * 火焰危险区。
 * 在编辑器中创建 BP_FlameHazard，配置：
 *   - HazardEffect → 指定火焰 Niagara System 资产
 *   - HazardZone → 调整球体半径覆盖火焰范围
 *   - DamageInterval → 伤害 tick 频率
 *
 * 后续实现伤害时重写 InflictDamage_Implementation，应用 GE_FireDamage。
 */
UCLASS(Blueprintable)
class THEMANTEST_API AFlameHazard : public AEnvironmentHazardBase
{
	GENERATED_BODY()

public:
	AFlameHazard();

	// 伤害接口（TODO：后续功能点实现 GAS Effect 扣血）
	virtual void InflictDamage_Implementation(AActor* Target) override;
};
