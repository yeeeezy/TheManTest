#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/AI/CoreMorphAIController.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphScorpionMovement.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BrainComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
constexpr double CoreHeight=785.72;
const FVector RestTip(1845,0,1955);
float Smooth(float T){T=FMath::Clamp(T,0.f,1.f);return T*T*T*(10+T*(-15+6*T));}
}
UCoreMorphScorpionCombat::UCoreMorphScorpionCombat()
{PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickGroup=TG_PostPhysics;}
void UCoreMorphScorpionCombat::BeginPlay()
{Super::BeginPlay();Boss=Cast<ACoreMorphBoss>(GetOwner());Motor=Boss?Boss->ScorpionMovement.Get():nullptr;}
void UCoreMorphScorpionCombat::EndPlay(const EEndPlayReason::Type Reason)
{ResetCombat();Super::EndPlay(Reason);}
bool UCoreMorphScorpionCombat::IsPaused() const {return bPaused;}
bool UCoreMorphScorpionCombat::IsDrivingPose() const {return bInitialized;}
bool UCoreMorphScorpionCombat::IsReady() const
{return bEnabled && Boss && !Boss->IsDead() && Boss->CurrentForm==ECoreMorphForm::Scorpion && !Boss->Reassembly->IsMorphing() && !bPaused && bInitialized && IsValid(Target);}
FVector UCoreMorphScorpionCombat::BodyPosition() const
{return Motor?Motor->GetGroundLocation()+FVector(0,0,CoreHeight):GetOwner()->GetActorLocation();}
bool UCoreMorphScorpionCombat::NeedsApproach() const
{
    if(!IsReady())return false;
    const float D=FVector::Dist2D(BodyPosition(),Target->GetActorLocation());
    return Action==ECoreMorphScorpionAction::Approach?(D>3550 || D<2800):(D>3900 || D<2400);
}
bool UCoreMorphScorpionCombat::CanStrike() const
{
    if(!IsReady() || Boss->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_CoreMorph_TailCooldown))return false;
    const float D=FVector::Dist2D(BodyPosition(),Target->GetActorLocation());
    return D>=2300 && D<=3950 && Action!=ECoreMorphScorpionAction::Windup && Action!=ECoreMorphScorpionAction::Thrust && Action!=ECoreMorphScorpionAction::Recover;
}
bool UCoreMorphScorpionCombat::InitializeCombat()
{
    if(!Boss || Boss->IsDead() || !Boss->Reassembly->Layout || !Motor || !Motor->Layout || Motor->Layout->LegNodes.Num()!=64)return false;
    const FVector Origin=Boss->GetActorLocation()-FVector(0,0,CoreHeight);
    const FTransform Pose(Boss->GetActorQuat(),Origin);
    if(!Motor->Initialize(Pose))return false;
    TailRest.Reset();TailFrames.SetNum(16);
    for(int32 J=0;J<16;++J)
    {
        const auto* S=Motor->Layout->Segments.FindByPredicate([J](const FCoreMorphScorpionSegment& P){return J==15?P.Group==TEXT("Stinger"):P.Group==TEXT("Tail") && P.Limb==J;});
        if(!S){ResetCombat();return false;}TailRest.Add(S->Pivot);
    }
    TailRest.Add(RestTip);TailLengths.Reset();TailNodes.Reset();
    for(int32 J=0;J<TailRest.Num();++J)
    {TailNodes.Add(Pose.TransformPosition(TailRest[J]));if(J>0)TailLengths.Add(FVector::Dist(TailRest[J-1],TailRest[J]));}
    TipPosition=PreviousTip=TailNodes.Last();
    Bindings.Init(INDEX_NONE,Boss->Reassembly->Layout->Pieces.Num());
    for(int32 I=0;I<Bindings.Num();++I)
    {
        const auto& P=Boss->Reassembly->Layout->Pieces[I];if(P.Form!=1)continue;
        Bindings[I]=Motor->Layout->Segments.IndexOfByPredicate([&](const FCoreMorphScorpionSegment& S)
        {
            if(P.Kind==TEXT("Leg"))return S.Group==TEXT("Leg") && S.Limb==P.Limb && S.Segment==FMath::RoundToInt((P.Order-.15f)*6/.85f);
            if(P.Kind==TEXT("Claw"))return S.Group==TEXT("Claw") && S.Limb==P.Limb-8;
            if(P.Kind==TEXT("Tail"))return P.Order>.97f?S.Group==TEXT("Stinger"):S.Group==TEXT("Tail") && S.Limb==FMath::RoundToInt((P.Order-.18f)*14/.75f);
            return S.Group==TEXT("Body");
        });
    }
    bInitialized=true;AdvanceTail(0);ApplyPose();return true;
}
void UCoreMorphScorpionCombat::ResetCombat()
{
    if(Boss)if(auto* AI=Cast<ACoreMorphAIController>(Boss->GetController()))AI->StopCombat();
    if(Motor)Motor->Reset();
    Target=nullptr;bInitialized=false;bPaused=false;
    Bindings.Reset();TailRest.Reset();TailNodes.Reset();TailLengths.Reset();TailFrames.Reset();
    WakeTime=CombatClock=ActionTime=BlockedTime=0;StrikeCount=HitCount=BlockedStrikes=0;
    Action=ECoreMorphScorpionAction::Idle;LockedTarget=TipPosition=PreviousTip=FVector::ZeroVector;bStrikeResolved=bStageNotified=false;
}
void UCoreMorphScorpionCombat::SetPaused(bool Value)
{
    bPaused=Value;
    if(Boss)if(auto* AI=Cast<ACoreMorphAIController>(Boss->GetController()))if(auto* Brain=AI->GetBrainComponent())
    {if(Value)Brain->PauseLogic(TEXT("Review pause"));else Brain->ResumeLogic(TEXT("Review resume"));}
}
bool UCoreMorphScorpionCombat::IsAttacking() const
{return Boss && Boss->GetAbilitySystemComponent()->HasMatchingGameplayTag(TAG_State_CoreMorph_Attacking);}
void UCoreMorphScorpionCombat::ResetStrike()
{
    bStrikeResolved=true;bStageNotified=false;
    if(Motor)Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);
    ActionStartTip=TipPosition;Action=ECoreMorphScorpionAction::Recover;ActionTime=0;
}
void UCoreMorphScorpionCombat::TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt,Tick,Function);
    if(!Boss || Boss->IsDead() || !bEnabled){if(bInitialized)ResetCombat();return;}
    if(bPaused || !FMath::IsFinite(Dt) || Dt<=0)return;
    if(auto* AI=Cast<ACoreMorphAIController>(Boss->GetController()))AI->EnsureTree();
    if(Boss->CurrentForm!=ECoreMorphForm::Scorpion || Boss->Reassembly->IsMorphing())return;
    if(!bInitialized){WakeTime+=Dt;if(WakeTime>=WakeDelay)InitializeCombat();return;}
    if(IsAttacking() && !IsValid(Target)){OnStrikeInvalidated.Broadcast();}
    const int32 Steps=FMath::Max(1,FMath::CeilToInt(FMath::Min(Dt,.25f)*120));const float Step=FMath::Min(Dt,.25f)/Steps;
    for(int32 I=0;I<Steps;++I)
    {
        CombatClock+=Step;ActionTime+=Step;Motor->AdvanceWalk(Step);AdvanceTail(Step);
        if(IsAttacking() && ActionFinished() && !bStageNotified){bStageNotified=true;OnStrikeStageFinished.Broadcast();}
        else if(!IsAttacking() && Action==ECoreMorphScorpionAction::Recover && ActionFinished())Action=ECoreMorphScorpionAction::Idle;
    }
    ApplyPose();
}
bool UCoreMorphScorpionCombat::StartAction(ECoreMorphScorpionAction Next)
{
    if(!Motor || (Next!=ECoreMorphScorpionAction::Idle && !IsReady()))return false;
    if(Next==ECoreMorphScorpionAction::Windup)
    {
        FCollisionQueryParams Q(SCENE_QUERY_STAT(CoreMorphStrikeGround),false,Boss);Q.AddIgnoredActor(Target);
        const FVector At=Target->GetActorLocation();
        if(!GetWorld()->LineTraceSingleByObjectType(LockedGroundHit,At+FVector(0,0,300),At-FVector(0,0,4000),FCollisionObjectQueryParams(ECC_WorldStatic),Q) || LockedGroundHit.ImpactNormal.Z<.55f)return false;
        LockedBlastRadius=FMath::Max(100.f,BlastRadius);
    }
    Action=Next;ActionTime=BlockedTime=0;bStageNotified=false;ActionStartTip=TipPosition;
    Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);
    if(Next==ECoreMorphScorpionAction::Windup)
    {
        if(FVector::Dist2D(BodyPosition(),Target->GetActorLocation())>4100)return false;
        LockedTarget=LockedGroundHit.ImpactPoint+LockedGroundHit.ImpactNormal*FMath::Max(0.f,TipRadius-1);
        WindupTip=Motor->GetBodyPose().TransformPosition(RestTip+FVector(-750,0,500));
        bStrikeResolved=false;
    }
    if(Next==ECoreMorphScorpionAction::Thrust){++StrikeCount;PreviousTip=TipPosition;}
    return true;
}
bool UCoreMorphScorpionCombat::TickMovement(ECoreMorphScorpionAction Which,float Dt)
{
    if(!IsReady())return false;
    const FVector Delta=Target->GetActorLocation()-Motor->GetGroundLocation();const float Distance=Delta.Size2D();
    const float Error=FMath::Abs(FMath::FindDeltaAngleDegrees(Motor->GetGroundRotation().Yaw,Delta.Rotation().Yaw));
    if(Which==ECoreMorphScorpionAction::Face)
    {
        if(Error<4.f){Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);return Motor->PlantedFeet==8;}
        Motor->SetExternalDrive(Target->GetActorLocation(),false,true);return false;
    }
    if(Distance<=3550 && Distance>=2800){Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);return true;}
    const FVector Direction=FVector(Delta.X,Delta.Y,0).GetSafeNormal();
    FVector Goal=Target->GetActorLocation()-Direction*3300;
    // A short fan of body-clearance probes routes the giant local motor around nearby blocks.
    const FVector Start=Motor->GetGroundLocation()+FVector(0,0,800);
    const FVector Desired=(Goal-Motor->GetGroundLocation()).GetSafeNormal2D();
    FCollisionQueryParams Q(SCENE_QUERY_STAT(ScorpionSteering),false,Boss);FCollisionObjectQueryParams Objects(ECC_WorldStatic);
    float Best=-MAX_flt;FVector BestDirection=Desired;
    for(float Angle:{0.f,35.f,-35.f,70.f,-70.f,105.f,-105.f})
    {
        const FVector D=Desired.RotateAngleAxis(Angle,FVector::UpVector);FHitResult Hit;
        const bool Block=GetWorld()->SweepSingleByObjectType(Hit,Start,Start+D*1800,D.Rotation().Quaternion(),Objects,FCollisionShape::MakeBox(FVector(920,670,230)),Q);
        const float Score=(Block?Hit.Time:1.f)*2.f+FVector::DotProduct(D,Desired);
        if(Score>Best){Best=Score;BestDirection=D;}
    }
    if(!BestDirection.Equals(Desired,.01f))Goal=Motor->GetGroundLocation()+BestDirection*2200;
    Motor->SetExternalDrive(Goal,true,true);
    BlockedTime=Motor->bBlocked?BlockedTime+Dt:0;
    // Yield to the selector if terrain cannot support the next step; the idle branch retries.
    if(BlockedTime>1.2f){Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);return true;}
    return false;
}
bool UCoreMorphScorpionCombat::ActionFinished() const
{
    const float Duration=Action==ECoreMorphScorpionAction::Windup?WindupDuration:Action==ECoreMorphScorpionAction::Thrust?ThrustDuration:Action==ECoreMorphScorpionAction::Recover?RecoverDuration:.15f;
    return ActionTime>=Duration;
}
void UCoreMorphScorpionCombat::AbortAction()
{
    if(Motor)Motor->SetExternalDrive(Motor->GetGroundLocation(),false,false);
    bStrikeResolved=true;ActionStartTip=TipPosition;Action=ECoreMorphScorpionAction::Recover;ActionTime=0;
}
void UCoreMorphScorpionCombat::AdvanceTail(float Dt)
{
    if(!Motor || TailNodes.Num()!=17)return;
    const FTransform Body=Motor->GetBodyPose();const FVector Root=Body.TransformPosition(TailRest[0]);
    const FVector RestGoal=Body.TransformPosition(RestTip);
    FVector Goal=RestGoal;
    if(Action==ECoreMorphScorpionAction::Windup)Goal=FMath::Lerp(ActionStartTip,WindupTip,Smooth(ActionTime/WindupDuration));
    else if(Action==ECoreMorphScorpionAction::Thrust)Goal=bStrikeResolved?ContactTip:FMath::Lerp(ActionStartTip,LockedTarget,Smooth(ActionTime/ThrustDuration));
    else if(Action==ECoreMorphScorpionAction::Recover)Goal=FMath::Lerp(ActionStartTip,RestGoal,Smooth(ActionTime/RecoverDuration));
    else Goal=FMath::VInterpTo(TipPosition,RestGoal,Dt,4.f);
    // A single curvature scale distributes the requested bend over the anatomical arc.
    // Unlike an endpoint-only solve, no individual joint can consume the whole correction.
    // Every link (including the rigid stinger) keeps its original length.
    const FVector Up=Body.GetRotation().GetUpVector();
    const FVector Delta=Goal-Root;
    FVector Forward=FVector::VectorPlaneProject(Delta,Up).GetSafeNormal();
    if(Forward.IsNearlyZero())Forward=Body.GetRotation().GetForwardVector();
    double Angles[16];Angles[0]=0;
    double MaxScale=1.5;
    FVector PreviousRest=(TailRest[1]-TailRest[0]).GetSafeNormal();
    for(int32 J=1;J<16;++J)
    {
        const FVector RestDirection=(TailRest[J+1]-TailRest[J]).GetSafeNormal();
        const double Bend=FMath::Atan2(PreviousRest.X*RestDirection.Z-PreviousRest.Z*RestDirection.X,
            FVector::DotProduct(PreviousRest,RestDirection));
        Angles[J]=Angles[J-1]+Bend;
        const double Limit=FMath::DegreesToRadians(J==15?16.0:20.0);
        if(FMath::Abs(Bend)>UE_DOUBLE_SMALL_NUMBER)MaxScale=FMath::Min(MaxScale,Limit/FMath::Abs(Bend));
        PreviousRest=RestDirection;
    }
    auto ArcEnd=[&](double Scale)
    {
        FVector2D End=FVector2D::ZeroVector;
        for(int32 J=0;J<16;++J)End+=FVector2D(FMath::Cos(Angles[J]*Scale),FMath::Sin(Angles[J]*Scale))*TailLengths[J];
        return End;
    };
    // The admissible arc has monotonically decreasing reach. Unreachable requests keep
    // the nearest admissible shape instead of breaking a joint or stretching the stinger.
    double Low=0,High=MaxScale;
    for(int32 Pass=0;Pass<32;++Pass)
    {
        const double Scale=(Low+High)*.5;
        if(ArcEnd(Scale).Size()>Delta.Size())Low=Scale;else High=Scale;
    }
    const double Scale=(Low+High)*.5;
    const FVector2D End=ArcEnd(Scale);
    const double Heading=FMath::Atan2(FVector::DotProduct(Delta,Up),FVector::DotProduct(Delta,Forward))-
        FMath::Atan2(End.Y,End.X);
    TailNodes[0]=Root;
    for(int32 J=0;J<16;++J)
    {
        const double Angle=Heading+Angles[J]*Scale;
        TailNodes[J+1]=TailNodes[J]+(Forward*FMath::Cos(Angle)+Up*FMath::Sin(Angle))*TailLengths[J];
    }
    TipPosition=TailNodes.Last();
    if(Action==ECoreMorphScorpionAction::Thrust && !bStrikeResolved)SweepTip(PreviousTip,TipPosition);
    PreviousTip=TipPosition;
    const FVector Side=FVector::CrossProduct(Up,Forward).GetSafeNormal();
    for(int32 J=0;J<16;++J)
    {
        const FQuat RestFrame=FRotationMatrix::MakeFromXY(TailRest[J+1]-TailRest[J],FVector::RightVector).ToQuat();
        const FQuat Frame=FRotationMatrix::MakeFromXY(TailNodes[J+1]-TailNodes[J],Side).ToQuat();
        TailFrames[J]=FTransform((Frame*RestFrame.Inverse()).GetNormalized(),TailNodes[J]);
    }
}
void UCoreMorphScorpionCombat::SweepTip(const FVector& From,const FVector& To)
{
    FCollisionQueryParams Q(SCENE_QUERY_STAT(ScorpionTailStrike),false,Boss);Q.AddIgnoredActor(Target);
    FCollisionObjectQueryParams Objects;Objects.AddObjectTypesToQuery(ECC_WorldStatic);Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    FHitResult Hit;
    if(GetWorld()->SweepSingleByObjectType(Hit,From,To,FQuat::Identity,Objects,FCollisionShape::MakeSphere(TipRadius),Q))
    {
        bStrikeResolved=true;
        if(Hit.ImpactNormal.Z>.55f && FVector::Dist(Hit.ImpactPoint,LockedGroundHit.ImpactPoint)<TipRadius*2)
        {OnStrikeContact.Broadcast(Hit);++HitCount;}
        else ++BlockedStrikes;
        // Contact halts the strike at the actual obstacle, including a wall in front of the player.
        ContactTip=Hit.Location;
    }
}
void UCoreMorphScorpionCombat::ApplyPose()
{
    Boss->SetActorLocationAndRotation(BodyPosition(),Motor->GetGroundRotation(),false,nullptr,ETeleportType::TeleportPhysics);
    const auto& Poses=Motor->GetSegmentTransforms();const auto& Pieces=Boss->Reassembly->GetPieces();
    for(int32 I=0;I<Bindings.Num() && I<Pieces.Num();++I)
    {
        const auto& P=Boss->Reassembly->Layout->Pieces[I];if(P.Form!=1)continue;
        const int32 Binding=Bindings[I];FTransform Frame=Motor->GetBodyPose();FVector Pivot=FVector::ZeroVector;
        if(Motor->Layout->Segments.IsValidIndex(Binding) && Poses.IsValidIndex(Binding))
        {
            const auto& S=Motor->Layout->Segments[Binding];Frame=Poses[Binding];Pivot=S.Pivot;
            if(S.Group==TEXT("Tail"))Frame=TailFrames[S.Limb];else if(S.Group==TEXT("Stinger"))Frame=TailFrames[15];
        }
        const FVector Position=P.Position+FVector(0,0,CoreHeight);
        Pieces[I]->SetWorldTransform(FTransform(Frame.GetRotation(),Frame.TransformPosition(Position-Pivot)));
    }
}
FString UCoreMorphScorpionCombat::StatusLabel() const
{
    switch(Action)
    {
        case ECoreMorphScorpionAction::Approach:return TEXT("SCORPION | PURSUING");
        case ECoreMorphScorpionAction::Face:return TEXT("SCORPION | TURNING");
        case ECoreMorphScorpionAction::Windup:return TEXT("TAIL DRAWBACK | MOVE TO DODGE");
        case ECoreMorphScorpionAction::Thrust:return TEXT("TAIL STRIKE");
        case ECoreMorphScorpionAction::Recover:return TEXT("TAIL RECOVERY");
        default:return TEXT("SCORPION | WATCHING");
    }
}
