#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/Humanoid/Animation/HumanoidReactionBones.h"
#include "EnemyHitReactionComponent.generated.h"

class UAnimSequence;
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

/** Authored explosion animation with optional swept, grounded capsule root motion. */
UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEnemyHitReactionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UEnemyHitReactionComponent();
 virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction) override;
 virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") bool bEnabled=true;
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
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction|Animation") bool bApplyAnimationRootMotion=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") FHumanoidReactionBones BoneMapping;
 UFUNCTION(BlueprintCallable,Category="Reaction") void ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone=NAME_None,FVector HitLocalDirection=FVector::ZeroVector);
 UFUNCTION(BlueprintPure,Category="Reaction") EEnemyHitRegion ClassifyHitBone(FName Bone) const;
 void SampleAnimation(UAnimSequence*& OutAnimation,float& OutTime,float& OutAlpha) const;
private:
 UPROPERTY(Transient) TObjectPtr<UAnimSequence> ActiveAnimation;
 double AnimationStartTime=-1000;
 float ActivePlayRate=1.f;
 float AnimationStrength=0.f;
 float LastRootTime=0.f;
 FQuat RootMotionOrientation=FQuat::Identity;
 FVector RootMotionScale=FVector::OneVector;
 uint8 PreviousMovementMode=0;
 uint8 PreviousCustomMode=0;
 bool bOwnsMovement=false;
 void ReleaseMovement();
 FName HitBone;
};
