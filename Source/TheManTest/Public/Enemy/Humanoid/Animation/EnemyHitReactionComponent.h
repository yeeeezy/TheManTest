#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/Humanoid/Animation/HumanoidReactionFrame.h"
#include "EnemyHitReactionComponent.generated.h"

/** Enemy-owned, additive explosion reaction. Never moves the character capsule. */
UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEnemyHitReactionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") bool bEnabled=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax="55")) float MaxAngleDegrees=38.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".01",ClampMax=".5")) float AttackDuration=.055f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".05",ClampMax="2")) float RecoveryDuration=.85f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") FHumanoidReactionBones BoneMapping;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax=".2")) float FollowDelay=.045f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax="15",Units="cm")) float LegCompression=7.f;
 UFUNCTION(BlueprintCallable,Category="Reaction") void ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone=NAME_None);
 void Sample(FVector& OutRotationVectorCS,FName& OutBone) const;
 FHumanoidReactionFrame SampleFrame() const;
 static float EvaluateEnvelope(float Age,float Attack,float Recovery);
private:
 FVector AxisWS=FVector::RightVector;
 FName HitBone;
 float Amplitude=0.f;
 float VerticalStrength=0.f;
 double StartTime=-1000;
};
