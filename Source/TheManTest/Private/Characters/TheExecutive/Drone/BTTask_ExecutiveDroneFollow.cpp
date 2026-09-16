#include "Characters/TheExecutive/Drone/BTTask_ExecutiveDroneFollow.h"
#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
UBTTask_ExecutiveDroneFollow::UBTTask_ExecutiveDroneFollow()
{NodeName=TEXT("Follow Executive with swept flight");bNotifyTick=true;}
EBTNodeResult::Type UBTTask_ExecutiveDroneFollow::ExecuteTask(UBehaviorTreeComponent& C,uint8* Memory)
{
 return C.GetAIOwner() && Cast<AExecutiveDrone>(C.GetAIOwner()->GetPawn())?EBTNodeResult::InProgress:EBTNodeResult::Failed;
}
void UBTTask_ExecutiveDroneFollow::TickTask(UBehaviorTreeComponent& C,uint8* Memory,float Dt)
{
 auto* Drone=C.GetAIOwner()?Cast<AExecutiveDrone>(C.GetAIOwner()->GetPawn()):nullptr;
 if(!Drone || !Drone->GetLeader()) {FinishLatentTask(C,EBTNodeResult::Failed);return;}
 Drone->UpdateFollowGoal();
 if(auto* BB=C.GetBlackboardComponent()) {BB->SetValueAsObject(TEXT("Leader"),Drone->GetLeader());BB->SetValueAsVector(TEXT("FollowLocation"),Drone->FlightGoal);}
}
