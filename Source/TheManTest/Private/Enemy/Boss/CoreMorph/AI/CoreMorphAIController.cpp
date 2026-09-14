#include "Enemy/Boss/CoreMorph/AI/CoreMorphAIController.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "BrainComponent.h"
void ACoreMorphAIController::EnsureTree()
{
 auto* B=Cast<ACoreMorphBoss>(GetPawn());
 if(!bStarted && B && !B->IsDead() && B->HasActorBegunPlay() && B->ScorpionCombat->bEnabled && B->ScorpionCombat->BehaviorTree)bStarted=RunBehaviorTree(B->ScorpionCombat->BehaviorTree);
}
void ACoreMorphAIController::StopCombat(){if(GetBrainComponent())GetBrainComponent()->StopLogic(TEXT("CoreMorph stopped"));StopMovement();ClearFocus(EAIFocusPriority::Gameplay);bStarted=false;}
void ACoreMorphAIController::OnUnPossess(){StopCombat();Super::OnUnPossess();}
