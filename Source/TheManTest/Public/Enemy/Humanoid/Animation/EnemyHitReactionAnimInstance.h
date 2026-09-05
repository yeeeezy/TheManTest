#pragma once
#include "Animation/AnimInstance.h"
#include "EnemyHitReactionAnimInstance.generated.h"
UCLASS()
class THEMANTEST_API UEnemyHitReactionAnimInstance : public UAnimInstance
{
 GENERATED_BODY()
public:
 UPROPERTY(BlueprintReadOnly,Category="Reaction") FVector ReactionRotation=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") FName ReactionBone;
 virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
