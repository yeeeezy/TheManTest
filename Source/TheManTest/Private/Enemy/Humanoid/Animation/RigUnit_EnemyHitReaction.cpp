#include "Enemy/Humanoid/Animation/RigUnit_EnemyHitReaction.h"
#include "Rigs/RigHierarchy.h"

FRigUnit_EnemyHitReaction_Execute()
{
 auto* Hierarchy=ExecuteContext.Hierarchy;
 if(!Hierarchy||ReactionRotation.ContainsNaN())return;
 const FVector Torso=ReactionFrame.Torso.IsNearlyZero()?ReactionRotation:ReactionFrame.Torso;
 const auto& Bones=ReactionFrame.Bones;
 if(Torso.IsNearlyZero()&&ReactionFrame.Follow.IsNearlyZero()&&FMath::IsNearlyZero(ReactionFrame.Compression))return;
 const auto Bend=[&](FName Name,const FVector& Rotation,float Weight)
 {
  const FRigElementKey Key(Name,ERigElementType::Bone);
  if(!Hierarchy->Contains(Key))return;
  FTransform T=Hierarchy->GetGlobalTransform(Key);
  const float Angle=FMath::Min(float(Rotation.Size()),FMath::DegreesToRadians(55.f));
  T.SetRotation((FQuat(Rotation.GetSafeNormal(),Angle*Weight)*T.GetRotation()).GetNormalized());
  Hierarchy->SetGlobalTransform(Key,T,false,true);
 };
 // Preserve each incoming animation foot pose, not a fixed world-space standing pose.
 const FRigElementKey Pelvis(Bones.Pelvis,ERigElementType::Bone);
 const FRigElementKey LeftFoot(Bones.LeftFoot,ERigElementType::Bone),RightFoot(Bones.RightFoot,ERigElementType::Bone);
 const bool Legs=Hierarchy->Contains(Pelvis)&&Hierarchy->Contains(LeftFoot)&&Hierarchy->Contains(RightFoot);
 FTransform FootL,FootR;
 if(Legs)
 {
  FootL=Hierarchy->GetGlobalTransform(LeftFoot);FootR=Hierarchy->GetGlobalTransform(RightFoot);
  FTransform P=Hierarchy->GetGlobalTransform(Pelvis);
  P.AddToTranslation(FVector(0,0,-FMath::Clamp(ReactionFrame.Compression,0.f,15.f)));
  Hierarchy->SetGlobalTransform(Pelvis,P,false,true);
 }
 float Sum=0;for(float W:Bones.SpineWeights)Sum+=FMath::Max(0.f,W);
 for(int32 I=0;I<Bones.Spine.Num();++I)
  Bend(Bones.Spine[I],Torso,Bones.SpineWeights.IsValidIndex(I)?FMath::Max(0.f,Bones.SpineWeights[I])/FMath::Max(.01f,Sum):0.f);
 const FVector Lag=ReactionFrame.Follow-Torso;
 Bend(Bones.Neck,Lag,.35f);
 Bend(Bones.LeftArm,Lag,.35f);Bend(Bones.RightArm,Lag,.35f);
 const FString Name=ReactionBone.ToString();
 if(Name.Contains(TEXT("arm"))||Name.Contains(TEXT("hand"))||Name.Contains(TEXT("clavicle")))
  Bend(Name.EndsWith(TEXT("_l"))?Bones.LeftArm:Bones.RightArm,ReactionFrame.Follow,.45f);
 if(Name.Contains(TEXT("head"))||Name.Contains(TEXT("neck")))Bend(Bones.Neck,ReactionFrame.Follow,.3f);
 const auto SolveLeg=[&](FName Thigh,FName Calf,FName Foot,const FTransform& Goal)
 {
  const FRigElementKey A(Thigh,ERigElementType::Bone),B(Calf,ERigElementType::Bone),C(Foot,ERigElementType::Bone);
  if(!Hierarchy->Contains(A)||!Hierarchy->Contains(B)||!Hierarchy->Contains(C))return;
  FTransform TA=Hierarchy->GetGlobalTransform(A),TB=Hierarchy->GetGlobalTransform(B),TC=Hierarchy->GetGlobalTransform(C);
  const FVector Hip=TA.GetLocation(),Knee=TB.GetLocation(),End=TC.GetLocation(),Target=Goal.GetLocation();
  const float L1=FVector::Distance(Hip,Knee),L2=FVector::Distance(Knee,End);
  if(L1<.1f||L2<.1f)return;
  const FVector Axis=(Target-Hip).GetSafeNormal();
  const float Distance=FMath::Clamp(float(FVector::Distance(Hip,Target)),FMath::Abs(L1-L2)+.001f,L1+L2-.001f);
  const float Along=(L1*L1+Distance*Distance-L2*L2)/(2*Distance);
  FVector Pole=(Knee-Hip)-Axis*FVector::DotProduct(Knee-Hip,Axis);
  if(!Pole.Normalize())Pole=FVector::CrossProduct(Axis,FVector::RightVector).GetSafeNormal();
  const FVector NewKnee=Hip+Axis*Along+Pole*FMath::Sqrt(FMath::Max(0.f,L1*L1-Along*Along));
  TA.SetRotation((FQuat::FindBetweenNormals((Knee-Hip).GetSafeNormal(),(NewKnee-Hip).GetSafeNormal())*TA.GetRotation()).GetNormalized());
  Hierarchy->SetGlobalTransform(A,TA,false,true);
  TB.SetLocation(NewKnee);TB.SetRotation((FQuat::FindBetweenNormals((End-Knee).GetSafeNormal(),(Target-NewKnee).GetSafeNormal())*TB.GetRotation()).GetNormalized());
  Hierarchy->SetGlobalTransform(B,TB,false,true);Hierarchy->SetGlobalTransform(C,Goal,false,true);
 };
 if(Legs && ReactionFrame.Compression>0)
 {
  SolveLeg(Bones.LeftThigh,Bones.LeftCalf,Bones.LeftFoot,FootL);
  SolveLeg(Bones.RightThigh,Bones.RightCalf,Bones.RightFoot,FootR);
 }
}
