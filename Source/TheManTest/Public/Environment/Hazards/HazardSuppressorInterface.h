#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HazardSuppressorInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UHazardSuppressor : public UInterface
{
	GENERATED_BODY()
};

/**
 * 危险区压制者接口（标记接口）。
 * 实现此接口的 Actor（如 ARepairGunBullet）进入危险区时，
 * 危险区会暂停特效和伤害；Actor 销毁后自动恢复。
 * 后续扩展其他可压制危险区的工具时，只需让该 Actor 实现此接口即可。
 */
class THEMANTEST_API IHazardSuppressor
{
	GENERATED_BODY()
};
