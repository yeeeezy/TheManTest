#pragma once

#include "CoreMinimal.h"
#include "HumanoidEnemyTypes.generated.h"

UENUM(BlueprintType)
enum class EHumanoidEnemyAIState : uint8
{
	Patrol		UMETA(DisplayName = "Patrol"),		// 巡逻（未发现玩家）
	Aim			UMETA(DisplayName = "Aim"),			// 锁定玩家瞄准追击（有目标）
	SearchRush	UMETA(DisplayName = "Search Rush"),	// 丢失玩家，冲向消失点
	SearchScan	UMETA(DisplayName = "Search Scan"),	// 到达消失点，左右晃头
	Dead		UMETA(DisplayName = "Dead")
};
