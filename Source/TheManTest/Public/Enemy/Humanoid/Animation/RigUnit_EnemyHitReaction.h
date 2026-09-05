#pragma once
#include "Units/RigUnit.h"
#include "Enemy/Humanoid/Animation/HumanoidReactionFrame.h"
#include "RigUnit_EnemyHitReaction.generated.h"

/** Shared additive body response with delayed follow and leg compression; capsule/root stay fixed. */
USTRUCT(meta=(DisplayName="Enemy Directional Hit Reaction", Category="Enemy", NodeColor="0.8 0.15 0.1"))
struct THEMANTEST_API FRigUnit_EnemyHitReaction : public FRigUnitMutable
{
 GENERATED_BODY()
 RIGVM_METHOD() virtual void Execute() override;
 UPROPERTY(meta=(Input)) FVector ReactionRotation=FVector::ZeroVector;
 UPROPERTY(meta=(Input)) FName ReactionBone;
 UPROPERTY(meta=(Input)) FHumanoidReactionFrame ReactionFrame;
};
