#include "Enemy/Boss/CoreMorph/AI/BTTask_CoreMorphAction.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphReassemble.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
namespace {ACoreMorphBoss* Owner(UBehaviorTreeComponent& BT){return BT.GetAIOwner()?Cast<ACoreMorphBoss>(BT.GetAIOwner()->GetPawn()):nullptr;}}
UBTTask_CoreMorphAction::UBTTask_CoreMorphAction(){NodeName=TEXT("CoreMorph action");bNotifyTick=true;bCreateNodeInstance=true;}
EBTNodeResult::Type UBTTask_CoreMorphAction::ExecuteTask(UBehaviorTreeComponent& BT,uint8*)
{
 auto* B=Owner(BT);if(!B || B->IsDead())return EBTNodeResult::Failed;AbilityHandle=FGameplayAbilitySpecHandle();Elapsed=0;
 auto* C=B->ScorpionCombat.Get();auto* ASC=B->GetAbilitySystemComponent();
 if(Action==ECoreMorphTreeAction::Idle)return EBTNodeResult::InProgress;
 if(Action==ECoreMorphTreeAction::Approach || Action==ECoreMorphTreeAction::Face)return C->StartAction(Action==ECoreMorphTreeAction::Approach?ECoreMorphScorpionAction::Approach:ECoreMorphScorpionAction::Face)?EBTNodeResult::InProgress:EBTNodeResult::Failed;
 if(Action==ECoreMorphTreeAction::PhaseSkill)
 {
  if(!B->UseRandomSkill(C->Target,EEnemySkillRange::Near))return EBTNodeResult::Failed;
  for(auto& Spec:ASC->GetActivatableAbilities())if(Spec.IsActive()){AbilityHandle=Spec.Handle;break;}
 }
 else
 {
  const auto Class=Action==ECoreMorphTreeAction::Flight?UGA_CoreMorphFlight::StaticClass():UGA_CoreMorphReassemble::StaticClass();
  auto* Spec=ASC->FindAbilitySpecFromClass(Class);if(!Spec)return EBTNodeResult::Failed;
  AbilityHandle=Spec->Handle;if(!Spec->IsActive() && !ASC->TryActivateAbility(AbilityHandle))return EBTNodeResult::Failed;
 }
 return AbilityHandle.IsValid()?EBTNodeResult::InProgress:EBTNodeResult::Failed;
}
EBTNodeResult::Type UBTTask_CoreMorphAction::AbortTask(UBehaviorTreeComponent& BT,uint8*)
{
 if(auto* B=Owner(BT))
 {
  if(AbilityHandle.IsValid())B->GetAbilitySystemComponent()->CancelAbilityHandle(AbilityHandle);
  else if(B->ScorpionCombat->IsDrivingPose())B->ScorpionCombat->AbortAction();
 }
 AbilityHandle=FGameplayAbilitySpecHandle();return EBTNodeResult::Aborted;
}
void UBTTask_CoreMorphAction::TickTask(UBehaviorTreeComponent& BT,uint8*,float Dt)
{
 auto* B=Owner(BT);if(!B || B->IsDead()){FinishLatentTask(BT,EBTNodeResult::Failed);return;}
 auto* C=B->ScorpionCombat.Get();if(C->IsPaused())return;
 bool Done=false;
 if(Action==ECoreMorphTreeAction::Idle){Elapsed+=Dt;Done=Elapsed>.15f;}
 else if(Action==ECoreMorphTreeAction::Approach || Action==ECoreMorphTreeAction::Face)
 {if(!C->IsReady()){C->AbortAction();FinishLatentTask(BT,EBTNodeResult::Failed);return;}Done=C->TickMovement(Action==ECoreMorphTreeAction::Approach?ECoreMorphScorpionAction::Approach:ECoreMorphScorpionAction::Face,Dt);}
 else {auto* Spec=B->GetAbilitySystemComponent()->FindAbilitySpecFromHandle(AbilityHandle);Done=!Spec || !Spec->IsActive();}
 if(Done)FinishLatentTask(BT,EBTNodeResult::Succeeded);
}
