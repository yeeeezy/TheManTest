#pragma once
#include "Engine/DataAsset.h"
#include "CoreMorphScorpionLayout.generated.h"
USTRUCT(BlueprintType)
struct FCoreMorphScorpionSegment
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Group;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Limb=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) int32 Segment=0;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector Pivot=FVector::ZeroVector;
};
// Numerical articulation data only. Final armor remains owned by the visual layout.
UCLASS(BlueprintType)
class THEMANTEST_API UCoreMorphScorpionLayout : public UDataAsset
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FCoreMorphScorpionSegment> Segments;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FVector> LegNodes;
};
