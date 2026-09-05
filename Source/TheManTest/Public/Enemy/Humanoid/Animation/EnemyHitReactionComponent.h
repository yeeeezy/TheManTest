#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHitReactionComponent.generated.h"

/** Enemy-owned, additive explosion reaction. Never moves the character capsule. */
UCLASS(ClassGroup=(Enemy), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEnemyHitReactionComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction") bool bEnabled=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin="0",ClampMax="40")) float MaxAngleDegrees=22.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".01",ClampMax=".5")) float AttackDuration=.055f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Reaction",meta=(ClampMin=".05",ClampMax="2")) float RecoveryDuration=.55f;
 UFUNCTION(BlueprintCallable,Category="Reaction") void ReactToExplosion(FVector Origin,FVector FallbackDirection,float Strength,FName Bone=NAME_None);
 void Sample(FVector& OutRotationVectorCS,FName& OutBone) const;
 static float EvaluateEnvelope(float Age,float Attack,float Recovery);
private:
 FVector AxisWS=FVector::RightVector;
 FName HitBone;
 float Amplitude=0.f;
 double StartTime=-1000;
};
