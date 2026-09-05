#pragma once
#include "Animation/AnimInstance.h"
#include "EnemyHitReactionAnimInstance.generated.h"
class UAnimSequence;
UCLASS()
class THEMANTEST_API UEnemyHitReactionAnimInstance : public UAnimInstance
{
 GENERATED_BODY()
public:
 UPROPERTY(BlueprintReadOnly,Category="Reaction") TObjectPtr<UAnimSequence> ReactionAnimation;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") float ReactionTime=0.f;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") float ReactionAlpha=0.f;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") bool bUseFullBodyReaction=true;
 virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
