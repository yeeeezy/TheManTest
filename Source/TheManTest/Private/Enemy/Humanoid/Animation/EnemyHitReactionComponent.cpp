#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"

float UEnemyHitReactionComponent::EvaluateEnvelope(float Age,float Attack,float Recovery)
{
 if(Age<0)return 0;
 if(Age<Attack){const float T=Age/FMath::Max(.01f,Attack);return T*T*(3-2*T);}
 const float T=FMath::Clamp((Age-Attack)/FMath::Max(.05f,Recovery),0.f,1.f);
 return FMath::Square(1-T)*FMath::Cos(T*PI*1.5f);
}
EEnemyHitRegion UEnemyHitReactionComponent::ClassifyHitBone(FName Bone) const
{
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 const auto* Mesh=Enemy?Enemy->GetMesh():nullptr;
 if(!Mesh || Mesh->GetBoneIndex(Bone)==INDEX_NONE)return EEnemyHitRegion::Torso;
 auto Under=[&](FName Root){return !Root.IsNone() && (Bone==Root || Mesh->BoneIsChildOf(Bone,Root));};
 if(Under(BoneMapping.LeftArm))return EEnemyHitRegion::LeftArm;
 if(Under(BoneMapping.RightArm))return EEnemyHitRegion::RightArm;
 if(Under(BoneMapping.LeftThigh))return EEnemyHitRegion::LeftLeg;
 if(Under(BoneMapping.RightThigh))return EEnemyHitRegion::RightLeg;
 if(Under(BoneMapping.Neck))return EEnemyHitRegion::Head;
 return EEnemyHitRegion::Torso;
}
void UEnemyHitReactionComponent::ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone,FVector HitLocalDirection)
{
 auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||!Enemy||Enemy->IsDead()||!GetWorld()||Origin.ContainsNaN()||FallbackDirection.ContainsNaN()||!FMath::IsFinite(Strength)||Strength<=0)return;
 auto* Mesh=Enemy->GetMesh();
 if(!Mesh)return;
 HitBone=Mesh->GetBoneIndex(Bone)!=INDEX_NONE?Bone:Mesh->FindClosestBone(Origin);
 const FVector Point=Mesh->GetBoneIndex(HitBone)!=INDEX_NONE?Mesh->GetSocketLocation(HitBone):Enemy->GetActorLocation();
 FVector Away=Point-Origin;
 if(Away.SizeSquared()<25)Away=FallbackDirection;
 const FVector FullDirection=Away.GetSafeNormal();
 VerticalStrength=FullDirection.Z;
 Away.Z=0;
 if(!Away.Normalize())Away=Enemy->GetActorForwardVector();
 if(ReactionMode==EEnemyHitReactionMode::Animation)
 {
  // Let the current reaction finish, rather than repeatedly snapping back to its first frame.
  if(ActiveAnimation && GetWorld()->GetTimeSeconds()-AnimationStartTime<ActiveAnimation->GetPlayLength()/ActivePlayRate)return;
  if(HitLocalDirection.ContainsNaN())return;
  const FVector SourceDirection=HitLocalDirection.IsNearlyZero()?Enemy->GetActorQuat().UnrotateVector(-Away):-HitLocalDirection;
  const EEnemyHitRegion Region=ClassifyHitBone(HitBone);
  UAnimSequence* Selected=nullptr;
  for(const auto& Set:BodyAnimations)
   if(Set.Region==Region)
   {
    if(FMath::Abs(SourceDirection.X)>=FMath::Abs(SourceDirection.Y))Selected=SourceDirection.X>=0?Set.Front.Get():Set.Back.Get();
    else Selected=SourceDirection.Y>=0?Set.Right.Get():Set.Left.Get();
    break;
   }
  if(!Selected)
  {
  if(FMath::Abs(SourceDirection.X)>=FMath::Abs(SourceDirection.Y))
   Selected=SourceDirection.X>=0 ? (HeavyFrontAnimation && Strength>=HeavyFrontMinStrength ? HeavyFrontAnimation.Get():FrontAnimation.Get()) : BackAnimation.Get();
  else Selected=SourceDirection.Y>=0 ? RightAnimation.Get():LeftAnimation.Get();
  }
  // Animation assets are configured by the concrete enemy; never load a Phantom asset here.
  if(!Selected||!Mesh->GetSkeletalMeshAsset()||Selected->GetSkeleton()!=Mesh->GetSkeletalMeshAsset()->GetSkeleton()||Selected->IsValidAdditive())return;
  ActiveRegion=Region;ActiveAnimation=Selected;AnimationStartTime=GetWorld()->GetTimeSeconds();
  ActivePlayRate=FMath::Clamp(AnimationPlayRate,.1f,3.f);AnimationStrength=FMath::Clamp(Strength,0.f,1.f);
  return;
 }
 ActiveAnimation=nullptr;
 AxisWS=FVector::CrossProduct(FVector::UpVector,Away).GetSafeNormal();
 Amplitude=FMath::DegreesToRadians(FMath::Clamp(MaxAngleDegrees,0.f,55.f))*FMath::Clamp(Strength,0.f,1.f)*(1.f-.25f*FMath::Abs(VerticalStrength));
 StartTime=GetWorld()->GetTimeSeconds();
}

FHumanoidReactionFrame UEnemyHitReactionComponent::SampleFrame() const
{
 FHumanoidReactionFrame Frame;Frame.Bones=BoneMapping;
 FName Bone;Sample(Frame.Torso,Bone);
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||ReactionMode!=EEnemyHitReactionMode::ControlRig||!Enemy||Enemy->IsDead()||!GetWorld()||!Enemy->GetMesh())return Frame;
 const float Age=float(GetWorld()->GetTimeSeconds()-StartTime);
 Frame.Follow=Enemy->GetMesh()->GetComponentTransform().InverseTransformVectorNoScale(AxisWS)*Amplitude
  *EvaluateEnvelope(Age-FollowDelay,AttackDuration*1.25f,RecoveryDuration);
 Frame.Compression=LegCompression*FMath::Clamp(Amplitude/FMath::DegreesToRadians(38.f),0.f,1.5f)
  *FMath::Max(0.f,EvaluateEnvelope(Age-.015f,AttackDuration*1.6f,RecoveryDuration))*(1.f-.4f*VerticalStrength);
 return Frame;
}
void UEnemyHitReactionComponent::Sample(FVector& Out,FName& OutBone) const
{
 Out=FVector::ZeroVector;OutBone=HitBone;
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||ReactionMode!=EEnemyHitReactionMode::ControlRig||!Enemy||Enemy->IsDead()||!GetWorld()||!Enemy->GetMesh())return;
 const float Alpha=EvaluateEnvelope(float(GetWorld()->GetTimeSeconds()-StartTime),AttackDuration,RecoveryDuration);
 Out=Enemy->GetMesh()->GetComponentTransform().InverseTransformVectorNoScale(AxisWS)*Amplitude*Alpha;
}

void UEnemyHitReactionComponent::SampleAnimation(UAnimSequence*& OutAnimation,float& OutTime,float& OutAlpha) const
{
 OutAnimation=nullptr;OutTime=0;OutAlpha=0;
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||ReactionMode!=EEnemyHitReactionMode::Animation||!ActiveAnimation||!Enemy||Enemy->IsDead()||!GetWorld())return;
 const float Age=float(GetWorld()->GetTimeSeconds()-AnimationStartTime);
 const float Duration=ActiveAnimation->GetPlayLength()/ActivePlayRate;
 if(Age<0||Age>=Duration)return;
 auto Smooth=[](float T){T=FMath::Clamp(T,0.f,1.f);return T*T*(3.f-2.f*T);};
 OutAnimation=ActiveAnimation;OutTime=Age*ActivePlayRate;
 OutAlpha=AnimationStrength*FMath::Min(Smooth(Age/FMath::Max(.01f,AnimationBlendIn)),Smooth((Duration-Age)/FMath::Max(.01f,AnimationBlendOut)));
}
