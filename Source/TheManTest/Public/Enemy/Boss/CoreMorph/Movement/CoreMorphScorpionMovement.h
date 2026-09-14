#pragma once
#include "Components/ActorComponent.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphScorpionLayout.h"
#include "CoreMorphScorpionMovement.generated.h"
struct FCoreMorphScorpionFoot
{
    FVector Position=FVector::ZeroVector, From=FVector::ZeroVector, To=FVector::ZeroVector;
    bool bSwing=false;
    float Progress=0, GroundZ=0;
    int32 Steps=0;
    float SwingDuration=.5f, StanceAge=0;
    FVector GroundNormal=FVector::UpVector;
};


UCLASS(ClassGroup=(CoreMorph))
class THEMANTEST_API UCoreMorphScorpionMovement : public UActorComponent
{
 GENERATED_BODY()
public:
 UCoreMorphScorpionMovement();
 UPROPERTY(EditAnywhere,Category="CoreMorph") TObjectPtr<UCoreMorphScorpionLayout> Layout;
 UPROPERTY(EditAnywhere,Category="Movement",meta=(ClampMin="0")) float WalkSpeed=650;
 UPROPERTY(EditAnywhere,Category="Movement",meta=(ClampMin="0")) float TurnSpeed=24;
 UPROPERTY(EditAnywhere,Category="Movement",meta=(ClampMin="300")) float StrideLength=560;
 UPROPERTY(EditAnywhere,Category="Movement",meta=(ClampMin="0")) float StepHeight=125;
 float CurrentSpeed=0,DistanceTravelled=0,YawRate=0;
 int32 PlantedFeet=8;
 bool bBlocked=false;
 bool Initialize(const FTransform& GroundFrame);
 void Reset();
 void AdvanceWalk(float Dt);
 void SetExternalDrive(FVector Goal,bool bTranslate,bool bTurn);
 const TArray<FTransform>& GetSegmentTransforms() const {return SegmentTransforms;}
 const FTransform& GetBodyPose() const {return BodyPose;}
 const TArray<FCoreMorphScorpionFoot>& GetFeet() const {return Feet;}
 const TArray<FVector>& GetSolvedNodes() const {return SolvedNodes;}
 FVector GetGroundLocation() const {return GroundFrame.GetLocation();}
 FRotator GetGroundRotation() const {return GroundFrame.Rotator();}
private:
 TArray<FTransform> SegmentTransforms;
 TArray<FCoreMorphScorpionFoot> Feet;
 TArray<FVector> SolvedNodes;
 TArray<float> JointAngles,RestAngles;
 FTransform GroundFrame,BodyPose;
 FVector ExternalGoal=FVector::ZeroVector,Velocity=FVector::ZeroVector;
 float Clock=0,GaitPhase=.40f,BodyGround=0,TailLag=0;
 bool bDriveTranslation=false,bDriveTurn=false,bChasing=false;
 int32 LastSwingGroup=1;
 float GroupRest=0,TerrainPitch=0,TerrainRoll=0;
 bool SampleAnatomicalGround(const FVector& At,FVector& Contact,FVector& Normal) const;
 void InitializeAnatomicalFeet();
 void UpdateAnatomicalPose(float Dt);
 void SolveAnatomicalLeg(int32 Leg,float Dt);
};
