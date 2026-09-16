#include "Characters/TheExecutive/Drone/ExecutiveDrone.h"
#include "Characters/TheExecutive/Drone/ExecutiveDroneMovementComponent.h"
#include "Characters/TheExecutive/Drone/ExecutiveDroneAIController.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

AExecutiveDrone::AExecutiveDrone()
{
 PrimaryActorTick.bCanEverTick=true;
 Collision=CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
 SetRootComponent(Collision); Collision->InitSphereRadius(32.f);
 Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 Collision->SetCollisionObjectType(ECC_WorldDynamic);
 Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
 Collision->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
 Collision->SetCanEverAffectNavigation(false);
 DroneMesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DroneMesh"));
 DroneMesh->SetupAttachment(Collision); DroneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 DroneMesh->SetCanEverAffectNavigation(false);
 DroneMesh->SetLightingChannels(true,true,false);
 DroneMesh->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
 Flight=CreateDefaultSubobject<UExecutiveDroneMovementComponent>(TEXT("Flight"));
 Flight->SetUpdatedComponent(Collision);
 AIControllerClass=AExecutiveDroneAIController::StaticClass();
 AutoPossessAI=EAutoPossessAI::Disabled;
}
UPawnMovementComponent* AExecutiveDrone::GetMovementComponent() const { return Flight; }
void AExecutiveDrone::InitializeCompanion(AActor* InLeader,bool bPresentation)
{
 Leader=InLeader; bLobbyPresentation=bPresentation;
}
void AExecutiveDrone::BeginPlay()
{
 Super::BeginPlay();
 Collision->IgnoreActorWhenMoving(Leader.Get(),true);
 if(bLobbyPresentation) Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 FlightGoal=GetAnchor(); LastYaw=GetActorRotation().Yaw;
 AddTickPrerequisiteComponent(Flight); DroneMesh->AddTickPrerequisiteActor(this);
 if(!bLobbyPresentation) SpawnDefaultController();
}
FVector AExecutiveDrone::GetAnchor() const
{
 if(!Leader.IsValid()) return GetActorLocation();
 const FRotator Heading(0,Leader->GetActorRotation().Yaw,0);
 return Leader->GetActorLocation()+Heading.RotateVector(bLobbyPresentation?LobbyOffset:FollowOffset);
}
bool AExecutiveDrone::HasClearPath(const FVector& Target) const
{
 FCollisionQueryParams Params(SCENE_QUERY_STAT(DroneFollow),false,this); Params.AddIgnoredActor(Leader.Get());
 return !GetWorld()->SweepTestByChannel(GetActorLocation(),Target,FQuat::Identity,ECC_WorldStatic,FCollisionShape::MakeSphere(Collision->GetScaledSphereRadius()),Params);
}
void AExecutiveDrone::UpdateFollowGoal()
{
 if(!Leader.IsValid()) return;
 const FVector Anchor=GetAnchor();
 const FVector Trail=Leader->GetActorLocation()+FVector(0,0,FollowOffset.Z);
 if(Breadcrumbs.IsEmpty() || FVector::DistSquared(Trail,Breadcrumbs.Last())>FMath::Square(60.f))
 { Breadcrumbs.Add(Trail); if(Breadcrumbs.Num()>96) Breadcrumbs.RemoveAt(0); }
 FlightGoal=Anchor;
 if(HasClearPath(Anchor)) { if(Breadcrumbs.Num()>1) Breadcrumbs.RemoveAt(0,Breadcrumbs.Num()-1); return; }
 for(int32 I=Breadcrumbs.Num()-1;I>=0;--I)
  if(FVector::DistSquared(GetActorLocation(),Breadcrumbs[I])>FMath::Square(45.f) && HasClearPath(Breadcrumbs[I]))
  { FlightGoal=Breadcrumbs[I]; return; }
 // Local detour candidates remain collision checked; never teleport through an obstacle.
 const FVector Direction=(Anchor-GetActorLocation()).GetSafeNormal();
 float BestScore=FVector::Dist(GetActorLocation(),Anchor)+160.f;
 FlightGoal=GetActorLocation();
 for(float Angle : {-90.f,-55.f,55.f,90.f})
  for(float Height : {0.f,90.f,-60.f})
  {
   const FVector Candidate=GetActorLocation()+Direction.RotateAngleAxis(Angle,FVector::UpVector)*180.f+FVector(0,0,Height);
   const float Score=FVector::Dist(Candidate,Anchor)+(HasClearPath(Candidate)?0.f:100000.f);
   if(Score<BestScore) {BestScore=Score;FlightGoal=Candidate;}
  }
}
void AExecutiveDrone::Tick(float Dt)
{
 Super::Tick(Dt);
 if(!Leader.IsValid()) {Destroy();return;}
 Age+=Dt;
 float TargetYaw=Leader->GetActorRotation().Yaw;
 if(bLobbyPresentation)
 {
  TargetYaw+=LobbyYawOffset;
  FlightGoal=GetAnchor()+FVector(4.f*FMath::Sin(Age*.7f),3.f*FMath::Cos(Age*.6f),4.f*FMath::Sin(Age*1.2f));
  const float Interval=FMath::Max(TurnInterval,TurnDuration+1.f);
  const float Phase=FMath::Fmod(Age,Interval);
  const float T=FMath::Clamp((Phase-(Interval-TurnDuration))/FMath::Max(TurnDuration,.1f),0.f,1.f);
  TargetYaw+=360.f*T*T*(3.f-2.f*T);
  CompletedLobbyTurns=FMath::FloorToInt(Age/Interval);
 }
 else if(Flight->Velocity.SizeSquared2D()>FMath::Square(40.f)) TargetYaw=Flight->Velocity.Rotation().Yaw;
 const float NewYaw=bLobbyPresentation?TargetYaw:FMath::FixedTurn(GetActorRotation().Yaw,TargetYaw,TurnSpeed*Dt);
 SetActorRotation(FRotator(0,NewYaw,0));
 YawRate=Dt>SMALL_NUMBER?FMath::FindDeltaAngleDegrees(LastYaw,NewYaw)/Dt:0.f; LastYaw=NewYaw;
}
void AExecutiveDrone::EndPlay(const EEndPlayReason::Type Reason)
{
 if(AController* AI=GetController()) {AI->UnPossess();AI->Destroy();}
 Super::EndPlay(Reason);
}
