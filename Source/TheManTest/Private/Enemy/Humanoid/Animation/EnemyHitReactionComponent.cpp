#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

float UEnemyHitReactionComponent::EvaluateEnvelope(float Age,float Attack,float Recovery)
{
 if(Age<0)return 0;
 if(Age<Attack){const float T=Age/FMath::Max(.01f,Attack);return T*T*(3-2*T);}
 const float T=FMath::Clamp((Age-Attack)/FMath::Max(.05f,Recovery),0.f,1.f);
 return FMath::Square(1-T)*FMath::Cos(T*PI*1.5f);
}
void UEnemyHitReactionComponent::ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone)
{
 auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||!Enemy||Enemy->IsDead()||!GetWorld()||Origin.ContainsNaN()||!FMath::IsFinite(Strength))return;
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
 AxisWS=FVector::CrossProduct(FVector::UpVector,Away).GetSafeNormal();
 Amplitude=FMath::DegreesToRadians(FMath::Clamp(MaxAngleDegrees,0.f,55.f))*FMath::Clamp(Strength,0.f,1.f)*(1.f-.25f*FMath::Abs(VerticalStrength));
 StartTime=GetWorld()->GetTimeSeconds();
}

FHumanoidReactionFrame UEnemyHitReactionComponent::SampleFrame() const
{
 FHumanoidReactionFrame Frame;Frame.Bones=BoneMapping;
 FName Bone;Sample(Frame.Torso,Bone);
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||!Enemy||Enemy->IsDead()||!GetWorld()||!Enemy->GetMesh())return Frame;
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
 if(!bEnabled||!Enemy||Enemy->IsDead()||!GetWorld()||!Enemy->GetMesh())return;
 const float Alpha=EvaluateEnvelope(float(GetWorld()->GetTimeSeconds()-StartTime),AttackDuration,RecoveryDuration);
 Out=Enemy->GetMesh()->GetComponentTransform().InverseTransformVectorNoScale(AxisWS)*Amplitude*Alpha;
}
