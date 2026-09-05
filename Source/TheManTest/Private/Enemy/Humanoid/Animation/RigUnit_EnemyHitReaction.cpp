#include "Enemy/Humanoid/Animation/RigUnit_EnemyHitReaction.h"
#include "Rigs/RigHierarchy.h"

FRigUnit_EnemyHitReaction_Execute()
{
 auto* Hierarchy=ExecuteContext.Hierarchy;
 if(!Hierarchy||ReactionRotation.ContainsNaN()||ReactionRotation.IsNearlyZero())return;
 const float Angle=FMath::Min(float(ReactionRotation.Size()),FMath::DegreesToRadians(40.f));
 const FVector Axis=ReactionRotation.GetSafeNormal();
 const auto Bend=[&](FName Name,float Weight)
 {
  const FRigElementKey Key(Name,ERigElementType::Bone);
  if(!Hierarchy->Contains(Key))return;
  FTransform T=Hierarchy->GetGlobalTransform(Key);
  T.SetRotation((FQuat(Axis,Angle*Weight)*T.GetRotation()).GetNormalized());
  Hierarchy->SetGlobalTransform(Key,T,false,true);
 };
 Bend(TEXT("spine_01"),.2f);Bend(TEXT("spine_02"),.35f);Bend(TEXT("spine_03"),.45f);
 const FString Name=ReactionBone.ToString();
 if(Name.Contains(TEXT("arm"))||Name.Contains(TEXT("hand"))||Name.Contains(TEXT("clavicle")))
  Bend(Name.EndsWith(TEXT("_l"))?TEXT("upperarm_l"):TEXT("upperarm_r"),.45f);
 if(Name.Contains(TEXT("head"))||Name.Contains(TEXT("neck")))Bend(TEXT("neck_01"),.3f);
}
