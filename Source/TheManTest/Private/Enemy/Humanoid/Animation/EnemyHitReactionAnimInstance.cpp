#include "Enemy/Humanoid/Animation/EnemyHitReactionAnimInstance.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
void UEnemyHitReactionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
 Super::NativeUpdateAnimation(DeltaSeconds);
 ReactionAnimation=nullptr;ReactionTime=0;ReactionAlpha=0;bUseFullBodyReaction=true;
 if(auto* Owner=GetOwningActor())
  if(auto* Reaction=Owner->FindComponentByClass<UEnemyHitReactionComponent>())
  {
   UAnimSequence* Sequence=nullptr;Reaction->SampleAnimation(Sequence,ReactionTime,ReactionAlpha);ReactionAnimation=Sequence;
   bUseFullBodyReaction=Owner->GetVelocity().SizeSquared2D()<100.f;
   if(const auto* Character=Cast<ACharacter>(Owner))
    if(Character->GetCharacterMovement()->IsFalling())bUseFullBodyReaction=false;
  }
}
