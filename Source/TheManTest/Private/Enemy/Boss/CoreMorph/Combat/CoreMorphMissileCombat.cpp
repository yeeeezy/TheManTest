#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
UCoreMorphMissileCombat::UCoreMorphMissileCombat()
{PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.bStartWithTickEnabled=false;PrimaryComponentTick.TickGroup=TG_PostPhysics;}
bool UCoreMorphMissileCombat::IsPaused() const
{auto* B=Cast<ACoreMorphBoss>(GetOwner());return B && (B->Flight->IsPaused() || B->ScorpionCombat->IsPaused());}
bool UCoreMorphMissileCombat::CanFire() const
{
 auto* B=Cast<ACoreMorphBoss>(GetOwner());
 return B && !B->IsDead() && B->CurrentForm==ECoreMorphForm::Manta && !B->Reassembly->IsMorphing() && B->Flight->IsFlying() && !IsPaused() && !bActive && IsValid(Target) && FVector::Dist(B->GetActorLocation(),Target->GetActorLocation())<=MaxRange;
}
FVector UCoreMorphMissileCombat::GetCoreLocation() const {return Core.IsValid()?Core->Bounds.Origin:GetOwner()->GetActorLocation();}
bool UCoreMorphMissileCombat::StartSalvo()
{
 if(!CanFire())return false;auto* B=CastChecked<ACoreMorphBoss>(GetOwner());
 Core.Reset();for(const auto& P:B->Flight->GetPieces())if(P && P->GetStaticMesh() && P->GetStaticMesh()->GetFName()==TEXT("SM_CoreMorph_Manta_OriginalCore_0")){Core=P;break;}
 if(!Core.IsValid())return false;
 Missiles.Reset();Clock=0;LockedRadius=FMath::Max(100.f,ImpactRadius);
 FRandomStream Random(RandomSeed?RandomSeed+SalvoNumber*7919:FMath::Rand());++SalvoNumber;
 const FVector Center=Target->GetActorLocation();const float Spread=FMath::Max(ScatterRadius,LockedRadius*3.5f);
 FCollisionQueryParams Q(SCENE_QUERY_STAT(CoreMorphMissileGround),false,B);Q.AddIgnoredActor(Target);
 for(int32 I=0;I<FMath::Clamp(MissileCount,1,8);++I)
 {
  for(int32 Try=0;Try<64;++Try)
  {
   const float A=Random.FRandRange(0,UE_TWO_PI),R=I==0?Random.FRandRange(0,LockedRadius*.15f):Random.FRandRange(LockedRadius*2.2f,Spread);
   const FVector At=Center+FVector(FMath::Cos(A)*R,FMath::Sin(A)*R,0);FHitResult Ground;
   if(!GetWorld()->LineTraceSingleByObjectType(Ground,At+FVector(0,0,15000),At-FVector(0,0,30000),FCollisionObjectQueryParams(ECC_WorldStatic),Q) || Ground.ImpactNormal.Z<.6f)continue;
   bool Clear=true;for(const auto& M:Missiles)if(FVector::Dist2D(M.Ground,Ground.ImpactPoint)<LockedRadius*2.1f){Clear=false;break;}
   if(!Clear)continue;
   auto& M=Missiles.AddDefaulted_GetRef();M.Ground=Ground.ImpactPoint;M.Normal=Ground.ImpactNormal;M.LaunchAt=FMath::Max(.3f,WarningLead)+I*FMath::Max(.05f,LaunchInterval);M.FlightTime=FMath::Max(.5f,TravelDuration)+Random.FRandRange(0,.3f);break;
  }
 }
 if(Missiles.IsEmpty())return false;bActive=true;SetComponentTickEnabled(true);return true;
}
void UCoreMorphMissileCombat::StopSalvo(){bActive=false;SetComponentTickEnabled(false);Missiles.Reset();Clock=0;}
void UCoreMorphMissileCombat::TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function)
{
 Super::TickComponent(Dt,Tick,Function);if(!bActive || IsPaused() || Dt<=0)return;
 auto* B=Cast<ACoreMorphBoss>(GetOwner());
 if(!B || B->IsDead() || B->CurrentForm!=ECoreMorphForm::Manta || B->Reassembly->IsMorphing() || !IsValid(Target)){OnInvalidated.Broadcast();return;}
 Clock+=FMath::Min(Dt,.25f);bool Finished=true;
 for(int32 I=0;I<Missiles.Num();++I)
 {
  auto& M=Missiles[I];
  if(!M.bLaunched && Clock>=M.LaunchAt)
  {
   M.bLaunched=true;M.Launch=GetCoreLocation();M.Position=M.Launch;
   const FVector Side=B->Flight->GetMotionState().GetBody().GetRotation().GetRightVector();
   M.Control=M.Launch+FVector(0,0,2200)+Side*((I%2?1:-1)*(700+I*200));
   M.Damage=FMath::Max(0.f,BaseDamage)*FMath::Max(0.f,B->GetDamageMultiplier());M.Trail.Add(M.Launch);
  }
  if(M.bLaunched && !M.bResolved)
  {
   const float T=FMath::Clamp((Clock-M.LaunchAt)/M.FlightTime,0.f,1.f),U=1-T;
   const FVector P2=M.Ground+M.Normal*4500,End=M.Ground+M.Normal*34;
   const FVector Next=U*U*U*M.Launch+3*U*U*T*M.Control+3*U*T*T*P2+T*T*T*End;
   M.Direction=(Next-M.Position).GetSafeNormal();if(M.Direction.IsNearlyZero())M.Direction=FVector::UpVector;
   FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(CoreMorphMissileFlight),false,B);
   const bool Contact=GetWorld()->SweepSingleByObjectType(Hit,M.Position,Next,FQuat::Identity,FCollisionObjectQueryParams(ECC_WorldStatic),FCollisionShape::MakeSphere(35),Q);
   M.Position=Contact?Hit.Location:Next;
   if(M.Trail.IsEmpty() || FVector::DistSquared(M.Trail.Last(),M.Position)>10000){M.Trail.Add(M.Position);if(M.Trail.Num()>24)M.Trail.RemoveAt(0);}
   if(Contact || T>=1)
   {
    M.bResolved=true;M.ImpactAt=Clock;M.bImpact=Contact && FVector::Dist(Hit.ImpactPoint,M.Ground)<100 && Hit.ImpactNormal.Z>.5f;
    if(M.bImpact){OnImpact.Broadcast(I);if(!bActive)return;}
   }
  }
  Finished &= M.bResolved && Clock-M.ImpactAt>=1.1f;
 }
 if(Finished){OnFinished.Broadcast();}
}
