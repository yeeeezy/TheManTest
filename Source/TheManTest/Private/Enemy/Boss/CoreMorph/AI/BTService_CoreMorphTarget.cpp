#include "Enemy/Boss/CoreMorph/AI/BTService_CoreMorphTarget.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "AbilitySystemComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
UBTService_CoreMorphTarget::UBTService_CoreMorphTarget(){NodeName=TEXT("CoreMorph target / form / range");Interval=.10f;RandomDeviation=0;bNotifyTick=true;bCallTickOnSearchStart=true;}
void UBTService_CoreMorphTarget::TickNode(UBehaviorTreeComponent& BT,uint8* M,float Dt)
{
 Super::TickNode(BT,M,Dt);auto* B=BT.GetAIOwner()?Cast<ACoreMorphBoss>(BT.GetAIOwner()->GetPawn()):nullptr;auto* BB=BT.GetBlackboardComponent();if(!B || !BB)return;
 auto* C=B->ScorpionCombat.Get();if(C->IsPaused())return;
 C->Target=IsValid(B->LastThreat)?B->LastThreat.Get():UGameplayStatics::GetPlayerPawn(B,0);
 if(auto* Enemy=Cast<AEnemyBase>(C->Target))if(Enemy->IsDead())C->Target=nullptr;
 B->MissileCombat->Target=C->Target;
 BB->SetValueAsBool(TEXT("CanBombard"),B->MissileCombat->CanFire() && !B->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_CoreMorph_MissileCooldown) && !B->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_CoreMorph_Attacking));
 BB->SetValueAsObject(TEXT("TargetActor"),C->Target);
 BB->SetValueAsBool(TEXT("Manta"),!B->IsDead() && B->CurrentForm==ECoreMorphForm::Manta);
 BB->SetValueAsBool(TEXT("CombatReady"),C->IsReady());BB->SetValueAsBool(TEXT("CanStrike"),C->CanStrike());BB->SetValueAsBool(TEXT("NeedsApproach"),C->NeedsApproach());
 BB->SetValueAsVector(TEXT("LockedStrikeLocation"),C->LockedTarget);
}
