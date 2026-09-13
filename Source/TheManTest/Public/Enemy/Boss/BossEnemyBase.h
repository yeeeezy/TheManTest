#pragma once

#include "Enemy/EnemyBase.h"
#include "BossEnemyBase.generated.h"

// Semantic boss boundary. Health, ASC, phase skill sets and death lifecycle remain in EnemyBase.
// Creature anatomy and transformations belong to concrete bosses.
UCLASS(Abstract)
class THEMANTEST_API ABossEnemyBase : public AEnemyBase
{
	GENERATED_BODY()
};
