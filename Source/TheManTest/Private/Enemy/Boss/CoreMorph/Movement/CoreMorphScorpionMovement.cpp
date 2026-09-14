#include "Enemy/Boss/CoreMorph/Movement/CoreMorphScorpionMovement.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

namespace
{
constexpr float ContactOffset=31;
FVector Radial(const FVector& V) {return FVector(V.X,V.Y,0).GetSafeNormal();}
int32 Tetrapod(int32 Leg) {return (Leg%4+Leg/4)%2;}
float Radians(float D) {return FMath::DegreesToRadians(D);}
float LimitAngle(int32 Joint,float Value,float Rest)
{
    // Coxa is fixed to the body. These are trochanter through telotarsus.
    const float Lo[]={0,-48,-48,-140,3,-85,0};
    const float Hi[]={0,55,65,-15,105,45,110};
    if(Joint<=2) return FMath::Clamp(Value,Rest+Radians(Lo[Joint]),Rest+Radians(Hi[Joint]));
    return FMath::Clamp(Value,Radians(Lo[Joint]),Radians(Hi[Joint]));
}
}

bool UCoreMorphScorpionMovement::SampleAnatomicalGround(const FVector& At,FVector& Contact,FVector& Normal) const
{
    FHitResult H;FCollisionQueryParams Q(SCENE_QUERY_STAT(ScorpionAnatomicalGround),false,GetOwner());
    FCollisionObjectQueryParams Objects(ECC_WorldStatic);
    if(GetWorld()->LineTraceSingleByObjectType(H,At+FVector(0,0,1800),At-FVector(0,0,2200),Objects,Q) && H.ImpactNormal.Z>.78f)
    {Contact=H.ImpactPoint+FVector(0,0,ContactOffset);Normal=H.ImpactNormal;return true;}
    if(true)
    {
        // A toe landing on the steep side of a small desert rock can plant beside it.
        // Keep the original lab's strict single-contact sampler unchanged.
        for(float Radius:{100.f,200.f})for(int32 I=0;I<8;++I)
        {
            const float A=I*PI/4;const FVector Probe=At+FVector(FMath::Cos(A),FMath::Sin(A),0)*Radius;
            if(GetWorld()->LineTraceSingleByObjectType(H,Probe+FVector(0,0,1800),Probe-FVector(0,0,2200),Objects,Q) && H.ImpactNormal.Z>.78f && FMath::Abs(H.ImpactPoint.Z-At.Z)<400)
            {Contact=H.ImpactPoint+FVector(0,0,ContactOffset);Normal=H.ImpactNormal;return true;}
        }
    }
    return false;
}
void UCoreMorphScorpionMovement::InitializeAnatomicalFeet()
{
    Feet.SetNum(8);SolvedNodes.SetNum(64);JointAngles.SetNumZeroed(56);RestAngles.SetNumZeroed(56);
    LastSwingGroup=1;GroupRest=0;TerrainPitch=TerrainRoll=0;BodyPose=GroundFrame;
    if(Layout->LegNodes.Num()!=64) return;
    for(int32 I=0;I<8;++I)
    {
        const FVector* R=&Layout->LegNodes[I*8];auto& F=Feet[I];F=FCoreMorphScorpionFoot();F.StanceAge=1;
        F.Position=BodyPose.TransformPosition(R[7]);SampleAnatomicalGround(F.Position,F.Position,F.GroundNormal);
        F.From=F.To=F.Position;F.GroundZ=F.Position.Z-ContactOffset;
        const FVector D=Radial(R[7]-R[1]);float Previous=0;
        for(int32 J=1;J<7;++J)
        {
            const FVector Bone=R[J+1]-R[J];const float A=FMath::Atan2(Bone.Z,FVector::DotProduct(Bone,D));
            RestAngles[I*7+J]=JointAngles[I*7+J]=A-Previous;Previous=A;
        }
    }
    PlantedFeet=8;UpdateAnatomicalPose(0);
}

void UCoreMorphScorpionMovement::AdvanceWalk(float Dt)
{
    if(!Layout || Layout->LegNodes.Num()!=64 || Feet.Num()!=8 || !FMath::IsFinite(Dt) || Dt<=0)return;
    if(Dt>1.f/60+.0001f){const int32 N=FMath::CeilToInt(Dt*60);for(int32 I=0;I<N;++I)AdvanceWalk(Dt/N);return;}
    Clock+=Dt;const FVector Goal=ExternalGoal;
    FVector To=Goal-GetGroundLocation();To.Z=0;const float Distance=To.Size(),Stop=0;
    bChasing=bDriveTurn || bDriveTranslation;
    const float OldYaw=GetGroundRotation().Yaw,Error=bChasing?FMath::FindDeltaAngleDegrees(OldYaw,To.Rotation().Yaw):0;
    YawRate=FMath::FInterpTo(YawRate,FMath::Clamp(Error*1.5f,-TurnSpeed,TurnSpeed),Dt,3.2f);
    const float Facing=FMath::Clamp(1.f-FMath::Abs(Error)/105.f,0.f,1.f);
    CurrentSpeed=FMath::FInterpConstantTo(CurrentSpeed,bChasing && bDriveTranslation?WalkSpeed*Facing*FMath::Clamp((Distance-Stop)/1000.f,0.f,1.f):0,Dt,220);
    float NewYaw=OldYaw+YawRate*Dt;const FVector Old=GetGroundLocation();
    FVector Next=Old+FRotator(0,NewYaw,0).Vector()*CurrentSpeed*Dt,Ground,Normal;
    FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(GiantScorpionBody),false,GetOwner());FCollisionObjectQueryParams Objects(ECC_WorldStatic);
    // The desert motor uses the actual raised belly volume so small ground rocks do not
    // overlap an oversized sphere extending into the space between planted legs.
    const FVector BodyLift(0,0,800);
    const FCollisionShape BodyShape=FCollisionShape::MakeBox(FVector(900,650,230));
    bBlocked=GetWorld()->SweepSingleByObjectType(Hit,Old+BodyLift,Next+BodyLift,FRotator(0,NewYaw,0).Quaternion(),Objects,BodyShape,Q);
    bool GroundSafe=SampleAnatomicalGround(Next,Ground,Normal);
    if(GroundSafe && !bBlocked)
    {
        const FTransform Candidate(FRotator(0,NewYaw,0),Next);
        for(int32 I=0;I<8;++I)
        {
            FVector C,N;const bool Valid=SampleAnatomicalGround(Candidate.TransformPosition(Layout->LegNodes[I*8+7]),C,N);
            if(!Valid || FMath::Abs(C.Z-Ground.Z)>280) {GroundSafe=false;break;}
        }
    }
    if(!GroundSafe || bBlocked) {Next=Old;CurrentSpeed=0;bBlocked=true;}
    else {BodyGround=FMath::FInterpTo(BodyGround,Ground.Z-30,Dt,3);Next.Z=BodyGround;}
    // Contact feedback limits body travel before an accelerating body can leave
    // a planted (or just lifting) leg beyond its finite reach.
    float TravelFraction=1;
    const FTransform BeforeBody(FRotator(0,OldYaw,0),Old),AfterBody(FRotator(0,NewYaw,0),Next);
    for(int32 I=0;I<8;++I)
    {
        const FVector* R=&Layout->LegNodes[I*8];float Reach=0;for(int32 J=1;J<7;++J) Reach+=(R[J+1]-R[J]).Size();Reach*=.965f;
        const float A=FVector::Dist(BeforeBody.TransformPosition(R[1]),Feet[I].Position),B=FVector::Dist(AfterBody.TransformPosition(R[1]),Feet[I].Position);
        if(B>Reach && B>A+.0001f) TravelFraction=FMath::Min(TravelFraction,FMath::Clamp((Reach-A)/(B-A),0.f,1.f));
    }
    if(TravelFraction<1) {Next=FMath::Lerp(Old,Next,TravelFraction);NewYaw=OldYaw+YawRate*Dt*TravelFraction;CurrentSpeed*=TravelFraction;YawRate*=TravelFraction;}
    GroundFrame=FTransform(FRotator(0,NewYaw,0),Next);Velocity=(Next-Old)/FMath::Max(Dt,SMALL_NUMBER);Velocity.Z=0;DistanceTravelled+=FVector::Dist2D(Old,Next);
    const float Drive=FMath::Max(CurrentSpeed,FMath::Abs(Radians(YawRate))*2500);
    GaitPhase+=Dt*Drive/FMath::Max(StrideLength,300.f);
    FVector MeanNormal=FVector::ZeroVector;int32 Supports=0;
    for(const auto& F:Feet) if(!F.bSwing) {MeanNormal+=F.GroundNormal;++Supports;}
    if(Supports>0) MeanNormal.Normalize();else MeanNormal=FVector::UpVector;
    const FVector LocalNormal=FRotator(0,NewYaw,0).UnrotateVector(MeanNormal);
    TerrainPitch=FMath::FInterpTo(TerrainPitch,FMath::Clamp(-FMath::RadiansToDegrees(FMath::Atan2(LocalNormal.X,LocalNormal.Z)),-10.f,10.f),Dt,3);
    TerrainRoll=FMath::FInterpTo(TerrainRoll,FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(LocalNormal.Y,LocalNormal.Z)),-10.f,10.f),Dt,3);
    const float Motion=FMath::Min(Drive/400,1.f);
    BodyPose=FTransform(FRotator(TerrainPitch,NewYaw,TerrainRoll-YawRate*.025f),Next+FVector(0,0,6*FMath::Sin(GaitPhase*4*PI)*Motion));
    bool Swinging=Feet.ContainsByPredicate([](const FCoreMorphScorpionFoot& F){return F.bSwing;});
    GroupRest=FMath::Max(0.f,GroupRest-Dt);
    if(!Swinging && GroupRest<=0)
    {
        const int32 Group=1-LastSwingGroup;bool NeedsStep=false;
        for(int32 I=0;I<8;++I) if(Tetrapod(I)==Group)
        {
            const FVector Rest=BodyPose.TransformPosition(Layout->LegNodes[I*8+7]);
            NeedsStep |= FVector::Dist2D(Rest,Feet[I].Position)>(Drive>10?65:95);
        }
        if(NeedsStep)
        {
            const float Duration=FMath::Clamp(.32f*StrideLength/FMath::Max(Drive,230.f),.38f,.78f);
            TArray<FVector> Targets,Normals;Targets.SetNum(8);Normals.SetNum(8);bool Valid=true;
            for(int32 I=0;I<8;++I) if(Tetrapod(I)==Group)
            {
                const FVector Rest=BodyPose.TransformPosition(Layout->LegNodes[I*8+7]),Relative=Rest-Next;
                // Predict through this swing and half of the following stance.
                // A swing-only lead leaves rear feet behind the body each cycle.
                const float Lead=Duration+.5f*(Duration+.28f);
                const FVector Turning=FVector(-Relative.Y,Relative.X,0)*Radians(YawRate);
                const float AccelerationLead=bChasing?FMath::Min(220.f*(Duration+.14f),FMath::Max(0.f,WalkSpeed*Facing-CurrentSpeed)):0;
                Targets[I]=Rest+((Velocity+GroundFrame.GetUnitAxis(EAxis::X)*AccelerationLead+Turning)*Lead).GetClampedToMaxSize(560);
                const FVector* R=&Layout->LegNodes[I*8];float Reach=0;for(int32 J=1;J<7;++J) Reach+=(R[J+1]-R[J]).Size();
                const FVector Hip=BodyPose.TransformPosition(R[1]);Targets[I]=Hip+(Targets[I]-Hip).GetClampedToMaxSize(Reach*.94f);
                Valid &= SampleAnatomicalGround(Targets[I],Targets[I],Normals[I]);
                Valid &= FMath::Abs(Targets[I].Z-Feet[I].Position.Z)<280;
            }
            if(Valid)
            {
                for(int32 I=0;I<8;++I) if(Tetrapod(I)==Group)
                {
                    auto& F=Feet[I];F.bSwing=true;F.Progress=-.025f*(3-I%4);F.From=F.Position;F.To=Targets[I];F.GroundNormal=Normals[I];F.SwingDuration=Duration;F.StanceAge=0;
                }
                LastSwingGroup=Group;Swinging=true;
            }
        }
        else
        {
            bool OtherNeedsStep=Drive>10;
            for(int32 I=0;I<8;++I) OtherNeedsStep |= FVector::Dist2D(BodyPose.TransformPosition(Layout->LegNodes[I*8+7]),Feet[I].Position)>95;
            if(OtherNeedsStep) LastSwingGroup=Group;
        }
    }
    PlantedFeet=0;bool Landed=false;
    for(int32 I=0;I<8;++I)
    {
        auto& F=Feet[I];
        if(F.bSwing)
        {
            F.Progress=FMath::Min(1.f,F.Progress+Dt/F.SwingDuration);const float T=FMath::Max(0.f,F.Progress);
            const float Ease=T*T*T*(10+T*(-15+6*T));
            const float Clearance=StepHeight*(1+.06f*(I%4));
            F.Position=FMath::Lerp(F.From,F.To,Ease)+FVector(0,0,Clearance*FMath::Square(FMath::Sin(PI*T)));
            if(T>=1) {F.Position=F.To;F.GroundZ=F.To.Z-ContactOffset;F.bSwing=false;F.StanceAge=0;++F.Steps;Landed=true;}
        }
        else F.StanceAge+=Dt;
        if(!F.bSwing) ++PlantedFeet;
    }
    if(Landed && PlantedFeet==8) GroupRest=.14f;
    TailLag=FMath::FInterpTo(TailLag,-YawRate*.22f,Dt,1.8f);UpdateAnatomicalPose(Dt);
}

void UCoreMorphScorpionMovement::SolveAnatomicalLeg(int32 Leg,float Dt)
{
    const FVector* R=&Layout->LegNodes[Leg*8];FVector* N=&SolvedNodes[Leg*8];
    float* Angle=&JointAngles[Leg*7];const float* Rest=&RestAngles[Leg*7];
    N[0]=BodyPose.TransformPosition(R[0]);N[1]=BodyPose.TransformPosition(R[1]);
    const FVector D=Radial(Feet[Leg].Position-N[1]);
    const FVector Goal3=Feet[Leg].Position-N[1];const FVector2D Goal(FVector::DotProduct(Goal3,D),Goal3.Z);
    float Length[7]={},Start[7]={};for(int32 J=1;J<7;++J) {Length[J]=(R[J+1]-R[J]).Size();Start[J]=Angle[J];}
    for(int32 J=1;J<7;++J) Angle[J]=FMath::FInterpTo(Angle[J],Rest[J],Dt,.7f);
    FVector2D Points[8];Points[1]=FVector2D::ZeroVector;
    auto Forward=[&]()
    {
        float A=0;for(int32 J=1;J<7;++J) {A+=Angle[J];Points[J+1]=Points[J]+FVector2D(FMath::Cos(A),FMath::Sin(A))*Length[J];}
    };
    // Warm-started planar CCD. All distal joints share one coherent leg plane;
    // anatomical bend signs and limits exclude knee reversal and arbitrary twist.
    Forward();
    for(int32 Iter=0;Iter<64;++Iter)
    {
        if(FVector2D::Distance(Points[7],Goal)<.015f) break;
        for(int32 J=6;J>=1;--J)
        {
            const FVector2D A=Points[7]-Points[J],B=Goal-Points[J];
            const float Change=FMath::Atan2(A.X*B.Y-A.Y*B.X,FVector2D::DotProduct(A,B));
            const float Weight=J==1?.35f:J==6?.55f:.85f;
            Angle[J]=LimitAngle(J,Angle[J]+FMath::Clamp(Change*Weight,-.12f,.12f),Rest[J]);
            if(Dt>0) Angle[J]=FMath::Clamp(Angle[J],Start[J]-Dt*4,Start[J]+Dt*4);
            Forward();
        }
    }
    for(int32 J=2;J<8;++J) N[J]=N[1]+D*Points[J].X+FVector::UpVector*Points[J].Y;
}

void UCoreMorphScorpionMovement::UpdateAnatomicalPose(float Dt)
{
    if(!Layout || Layout->LegNodes.Num()!=64 || Feet.Num()!=8) return;
    for(int32 I=0;I<8;++I) SolveAnatomicalLeg(I,Dt);
    const FQuat BodyQ=BodyPose.GetRotation();
    FQuat TailQ=BodyQ;FVector TailPivot=FVector::ZeroVector,PreviousRest=FVector::ZeroVector;bool Started=false;
    TArray<FTransform> TailFrames;TailFrames.SetNum(16);
    for(int32 I=0;I<16;++I)
    {
        const auto* P=Layout->Segments.FindByPredicate([I](const FCoreMorphScorpionSegment& S){return I<15?(S.Group==TEXT("Tail") && S.Limb==I):S.Group==TEXT("Stinger");});
        if(!P) continue;
        if(!Started) {TailPivot=BodyPose.TransformPosition(P->Pivot);Started=true;}
        else TailPivot+=TailQ.RotateVector(P->Pivot-PreviousRest);
        const float Delay=Clock-I*.15f;
        TailQ=(TailQ*FRotator(.22f*FMath::Sin(Delay*.9f),TailLag/15+.16f*FMath::Sin(Delay*.72f),0).Quaternion()).GetNormalized();
        TailFrames[I]=FTransform(TailQ,TailPivot);PreviousRest=P->Pivot;
    }
    SegmentTransforms.SetNum(Layout->Segments.Num());
    for(int32 I=0;I<Layout->Segments.Num();++I)
    {
        const auto& P=Layout->Segments[I];FTransform T(BodyQ,BodyPose.TransformPosition(P.Pivot));
        if(P.Group==TEXT("Leg"))
        {
            const FVector* R=&Layout->LegNodes[P.Limb*8];const FVector* N=&SolvedNodes[P.Limb*8];const int32 J=P.Segment;
            if(J==0) T=FTransform(BodyQ,N[0]);
            else
            {
                const FVector RestAxis=FVector::CrossProduct(FVector::UpVector,Radial(R[7]-R[1]));
                const FVector LiveAxis=FVector::CrossProduct(FVector::UpVector,Radial(Feet[P.Limb].Position-N[1]));
                const FQuat RestFrame=FRotationMatrix::MakeFromXY((R[J+1]-R[J]).GetSafeNormal(),RestAxis).ToQuat();
                const FQuat LiveFrame=FRotationMatrix::MakeFromXY((N[J+1]-N[J]).GetSafeNormal(),LiveAxis).ToQuat();
                T=FTransform((LiveFrame*RestFrame.Inverse()).GetNormalized(),N[J]);
            }
        }
        else if(P.Group==TEXT("Tail")) T=TailFrames[P.Limb];
        else if(P.Group==TEXT("Stinger")) T=TailFrames[15];
        else if(P.Group==TEXT("Claw")) T.SetRotation(BodyQ*FRotator(.35f*FMath::Sin(Clock*.95f+P.Limb*PI),(P.Limb?1:-1)*(.6f+.45f*FMath::Sin(Clock*.65f)),0).Quaternion());
        SegmentTransforms[I]=T;

    }
}

UCoreMorphScorpionMovement::UCoreMorphScorpionMovement(){PrimaryComponentTick.bCanEverTick=false;}
bool UCoreMorphScorpionMovement::Initialize(const FTransform& Frame)
{
 Reset();if(!Layout || Layout->LegNodes.Num()!=64 || Layout->Segments.IsEmpty())return false;
 GroundFrame=Frame;BodyGround=Frame.GetLocation().Z;ExternalGoal=Frame.GetLocation();InitializeAnatomicalFeet();return true;
}
void UCoreMorphScorpionMovement::Reset()
{
 Feet.Reset();SolvedNodes.Reset();JointAngles.Reset();RestAngles.Reset();SegmentTransforms.Reset();
 Clock=CurrentSpeed=DistanceTravelled=YawRate=TailLag=0;GaitPhase=.40f;Velocity=FVector::ZeroVector;
 bDriveTranslation=bDriveTurn=bChasing=bBlocked=false;PlantedFeet=8;
}
void UCoreMorphScorpionMovement::SetExternalDrive(FVector Goal,bool Translate,bool Turn)
{
 ExternalGoal=Goal;bDriveTranslation=Translate;bDriveTurn=Turn;
 if(!Translate)CurrentSpeed=0;if(!Turn)YawRate=0;
}
