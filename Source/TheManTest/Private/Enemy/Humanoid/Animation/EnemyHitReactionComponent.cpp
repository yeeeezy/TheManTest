#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/EnemyBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"

UEnemyHitReactionComponent::UEnemyHitReactionComponent()
{
 PrimaryComponentTick.bCanEverTick=true;
}
void UEnemyHitReactionComponent::ReleaseMovement()
{
 if(!bOwnsMovement)return;
 bOwnsMovement=false;
 if(auto* Enemy=Cast<AEnemyBase>(GetOwner()))
  if(!Enemy->IsDead())
   if(auto* Move=Enemy->GetCharacterMovement())
    if(Move->MovementMode==MOVE_None)Move->SetMovementMode(EMovementMode(PreviousMovementMode),PreviousCustomMode);
}
void UEnemyHitReactionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
 ReleaseMovement();Super::EndPlay(Reason);
}
void UEnemyHitReactionComponent::TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
 Super::TickComponent(DeltaTime,TickType,ThisTickFunction);
 if(!bOwnsMovement)return;
 auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||!bApplyAnimationRootMotion||!Enemy||Enemy->IsDead()||!ActiveAnimation||!GetWorld()){ReleaseMovement();return;}
 const float Time=FMath::Clamp(float(GetWorld()->GetTimeSeconds()-AnimationStartTime)*ActivePlayRate,0.f,ActiveAnimation->GetPlayLength());
 if(Time>LastRootTime)
 {
  const FAnimExtractContext Context(double(LastRootTime),true);
  const FVector Delta=ActiveAnimation->ExtractRootMotionFromRange(double(LastRootTime),double(Time),Context).GetTranslation();
  FVector WorldDelta=RootMotionOrientation.RotateVector(Delta*RootMotionScale);WorldDelta.Z=0;
  FHitResult Hit;
  Enemy->GetCharacterMovement()->SafeMoveUpdatedComponent(WorldDelta,Enemy->GetActorQuat(),true,Hit);
  LastRootTime=Time;
 }
 if(Time>=ActiveAnimation->GetPlayLength())ReleaseMovement();
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
 Away.Z=0;
 if(!Away.Normalize())Away=Enemy->GetActorForwardVector();
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
  ReleaseMovement();LastRootTime=0;
  if(auto* Move=Enemy->GetCharacterMovement())
   if(bApplyAnimationRootMotion&&Selected->HasRootMotion()&&!Move->IsFalling())
   {
    PreviousMovementMode=uint8(Move->MovementMode);PreviousCustomMode=Move->CustomMovementMode;
    RootMotionOrientation=Mesh->GetComponentQuat();RootMotionScale=Mesh->GetComponentScale();
    bOwnsMovement=true;Move->StopMovementImmediately();Move->SetMovementMode(MOVE_None);
   }
}

void UEnemyHitReactionComponent::SampleAnimation(UAnimSequence*& OutAnimation,float& OutTime,float& OutAlpha) const
{
 OutAnimation=nullptr;OutTime=0;OutAlpha=0;
 const auto* Enemy=Cast<AEnemyBase>(GetOwner());
 if(!bEnabled||!ActiveAnimation||!Enemy||Enemy->IsDead()||!GetWorld())return;
 const float Age=float(GetWorld()->GetTimeSeconds()-AnimationStartTime);
 const float Duration=ActiveAnimation->GetPlayLength()/ActivePlayRate;
 if(Age<0||Age>=Duration)return;
 auto Smooth=[](float T){T=FMath::Clamp(T,0.f,1.f);return T*T*(3.f-2.f*T);};
 OutAnimation=ActiveAnimation;OutTime=Age*ActivePlayRate;
 OutAlpha=AnimationStrength*FMath::Min(Smooth(Age/FMath::Max(.01f,AnimationBlendIn)),Smooth((Duration-Age)/FMath::Max(.01f,AnimationBlendOut)));
}
