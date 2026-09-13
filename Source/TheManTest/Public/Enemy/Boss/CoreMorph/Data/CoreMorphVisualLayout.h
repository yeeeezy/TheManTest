#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CoreMorphVisualLayout.generated.h"

USTRUCT(BlueprintType)
struct FCoreMorphVisualPiece
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<UStaticMesh> Mesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Position = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector Anchor = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Form = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Limb = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Order = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Kind;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector RevealAxis = FVector::ForwardVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float RevealMin = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float RevealSpan = 1;
};

UCLASS(BlueprintType)
class THEMANTEST_API UCoreMorphVisualLayout : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FCoreMorphVisualPiece> Pieces;
};
