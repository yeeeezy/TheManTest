#include "Enemy/Humanoid/Animation/EnemyHitReactionAnimInstance.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "GameFramework/Actor.h"
void UEnemyHitReactionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
 Super::NativeUpdateAnimation(DeltaSeconds);
 ReactionRotation=FVector::ZeroVector;
 if(auto* Owner=GetOwningActor())
  if(auto* Reaction=Owner->FindComponentByClass<UEnemyHitReactionComponent>())Reaction->Sample(ReactionRotation,ReactionBone);
}
