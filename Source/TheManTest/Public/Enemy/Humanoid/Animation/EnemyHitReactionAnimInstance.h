#pragma once
#include "Animation/AnimInstance.h"
#include "Enemy/Humanoid/Animation/HumanoidReactionFrame.h"
#include "EnemyHitReactionAnimInstance.generated.h"
class UAnimSequence;
UCLASS()
class THEMANTEST_API UEnemyHitReactionAnimInstance : public UAnimInstance
{
 GENERATED_BODY()
public:
 UPROPERTY(BlueprintReadOnly,Category="Reaction") FVector ReactionRotation=FVector::ZeroVector;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") FName ReactionBone;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") FHumanoidReactionFrame ReactionFrame;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") TObjectPtr<UAnimSequence> ReactionAnimation;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") float ReactionTime=0.f;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") float ReactionAlpha=0.f;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") bool bUseAnimationReaction=true;
 UPROPERTY(BlueprintReadOnly,Category="Reaction") bool bUseFullBodyReaction=true;
 virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
