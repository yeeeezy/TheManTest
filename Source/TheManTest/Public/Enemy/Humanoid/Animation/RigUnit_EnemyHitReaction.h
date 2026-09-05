#pragma once
#include "Units/RigUnit.h"
#include "RigUnit_EnemyHitReaction.generated.h"

/** Additive component-space bend distributed over the spine; root and legs are untouched. */
USTRUCT(meta=(DisplayName="Enemy Directional Hit Reaction", Category="Enemy", NodeColor="0.8 0.15 0.1"))
struct THEMANTEST_API FRigUnit_EnemyHitReaction : public FRigUnitMutable
{
 GENERATED_BODY()
 RIGVM_METHOD() virtual void Execute() override;
 UPROPERTY(meta=(Input)) FVector ReactionRotation=FVector::ZeroVector;
 UPROPERTY(meta=(Input)) FName ReactionBone;
};
