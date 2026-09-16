#include "Characters/TheExecutive/Drone/ExecutiveDroneAIController.h"
#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
AExecutiveDroneAIController::AExecutiveDroneAIController() {bAttachToPawn=true;}
void AExecutiveDroneAIController::OnPossess(APawn* InPawn)
{
 Super::OnPossess(InPawn);
 if(auto* Drone=Cast<AExecutiveDrone>(InPawn))
  if(!Drone->bLobbyPresentation && Drone->FollowTree) RunBehaviorTree(Drone->FollowTree);
}
void AExecutiveDroneAIController::OnUnPossess()
{
 if(BrainComponent) BrainComponent->StopLogic(TEXT("Companion released"));
 Super::OnUnPossess();
}
