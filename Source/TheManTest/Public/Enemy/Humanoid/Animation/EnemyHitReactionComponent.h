#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/Humanoid/Animation/HumanoidReactionFrame.h"
#include "EnemyHitReactionComponent.generated.h"

class UAnimSequence;
UENUM(BlueprintType)
enum class EEnemyHitReactionMode : uint8
{
 Animation,
 ControlRig
};

UENUM(BlueprintType)
enum class EEnemyHitRegion : uint8 { Torso, Head, LeftArm, RightArm, LeftLeg, RightLeg };

USTRUCT(BlueprintType)
struct FEnemyBodyReactionAnimations
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) EEnemyHitRegion Region=EEnemyHitRegion::Torso;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UAnimSequence> Front;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UAnimSequence> Back;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UAnimSequence> Left;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TObjectPtr<UAnimSequence> Right;
};

/** Enemy-owned, additive explosion reaction. Never moves the character capsule. */
UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEnemyHitReactionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") bool bEnabled=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") EEnemyHitReactionMode ReactionMode=EEnemyHitReactionMode::Animation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TArray<FEnemyBodyReactionAnimations> BodyAnimations;
 UPROPERTY(BlueprintReadOnly, Transient, Category="Reaction") EEnemyHitRegion ActiveRegion=EEnemyHitRegion::Torso;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TObjectPtr<UAnimSequence> FrontAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TObjectPtr<UAnimSequence> BackAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TObjectPtr<UAnimSequence> LeftAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TObjectPtr<UAnimSequence> RightAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") TObjectPtr<UAnimSequence> HeavyFrontAnimation;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation",meta=(ClampMin="0",ClampMax="1")) float HeavyFrontMinStrength=.9f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation",meta=(ClampMin=".01",ClampMax=".5")) float AnimationBlendIn=.06f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation",meta=(ClampMin=".01",ClampMax=".5")) float AnimationBlendOut=.18f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation",meta=(ClampMin=".1",ClampMax="3")) float AnimationPlayRate=1.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax="55")) float MaxAngleDegrees=38.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".01",ClampMax=".5")) float AttackDuration=.055f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".05",ClampMax="2")) float RecoveryDuration=.85f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") FHumanoidReactionBones BoneMapping;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax=".2")) float FollowDelay=.045f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax="15",Units="cm")) float LegCompression=7.f;
 UFUNCTION(BlueprintCallable,Category="Reaction") void ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone=NAME_None,FVector HitLocalDirection=FVector::ZeroVector);
 UFUNCTION(BlueprintPure,Category="Reaction") EEnemyHitRegion ClassifyHitBone(FName Bone) const;
 void Sample(FVector& OutRotationVectorCS,FName& OutBone) const;
 FHumanoidReactionFrame SampleFrame() const;
 void SampleAnimation(UAnimSequence*& OutAnimation,float& OutTime,float& OutAlpha) const;
 static float EvaluateEnvelope(float Age,float Attack,float Recovery);
private:
 UPROPERTY(Transient) TObjectPtr<UAnimSequence> ActiveAnimation;
 double AnimationStartTime=-1000;
 float ActivePlayRate=1.f;
 float AnimationStrength=0.f;
 FVector AxisWS=FVector::RightVector;
 FName HitBone;
 float Amplitude=0.f;
 float VerticalStrength=0.f;
 double StartTime=-1000;
};
