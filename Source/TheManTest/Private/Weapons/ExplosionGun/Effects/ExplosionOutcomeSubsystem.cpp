#include "Weapons/ExplosionGun/Effects/ExplosionOutcomeSubsystem.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Engine/World.h"

void UExplosionOutcomeSubsystem::Watch(UGeometryCollectionComponent* Collection,float Radius,const TSharedRef<FExplosionOutcome>& Outcome)
{
 if(!IsValid(Collection)||Outcome->bResolved||!Outcome->Settings.bEnabled)return;
 if(!OriginalNotify.Contains(Collection))
 {
  OriginalNotify.Add(Collection,Collection->bNotifyBreaks);
  Collection->OnChaosBreakEvent.AddUniqueDynamic(this,&UExplosionOutcomeSubsystem::OnBreak);
  Collection->SetNotifyBreaks(true);
 }
 Pending.Add({Collection,Outcome,Radius,GetWorld()->GetTimeSeconds()+.2f});
}
void UExplosionOutcomeSubsystem::OnBreak(const FChaosBreakEvent& Event)
{
 // A new break is required. Merely applying strain or kicking loose debris never qualifies.
 for(int32 I=Pending.Num()-1;I>=0;--I)
 {
  auto& P=Pending[I];
  if(P.Collection.Get()!=Event.Component||P.Outcome->bResolved||GetWorld()->GetTimeSeconds()>P.Expires)continue;
  if(FVector::DistSquared(Event.Location,P.Outcome->Origin)>FMath::Square(P.Radius))continue;
  P.Outcome->bResolved=true;
  if(auto* Feedback=GetWorld()->GetSubsystem<UBulletTimeSubsystem>())
   Feedback->RequestBulletTimeAtLocation(P.Outcome->Origin,P.Outcome->Settings);
 }
 // Unbind in Tick, outside the multicast's broadcast.
}
void UExplosionOutcomeSubsystem::Prune()
{
 Pending.RemoveAll([this](const FPending& P){return !P.Collection.IsValid()||P.Outcome->bResolved||GetWorld()->GetTimeSeconds()>P.Expires;});
 for(auto It=OriginalNotify.CreateIterator();It;++It)
 {
  const auto Key=It.Key();
  if(Pending.ContainsByPredicate([Key](const FPending& P){return P.Collection==Key;}))continue;
  if(auto* Collection=Key.Get())
  {
   Collection->OnChaosBreakEvent.RemoveDynamic(this,&UExplosionOutcomeSubsystem::OnBreak);
   Collection->SetNotifyBreaks(It.Value());
  }
  It.RemoveCurrent();
 }
}
void UExplosionOutcomeSubsystem::Tick(float DeltaTime){Prune();}
void UExplosionOutcomeSubsystem::Clear(){Pending.Reset();Prune();}
void UExplosionOutcomeSubsystem::OnWorldEndPlay(UWorld& World){Clear();Super::OnWorldEndPlay(World);}
void UExplosionOutcomeSubsystem::Deinitialize(){Clear();Super::Deinitialize();}
