#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TheManAnimationAssetLibrary.generated.h"

class UAnimationAsset;
class USkeleton;

// Small editor-facing repair hook used by destination-only asset migration workflows.
UCLASS()
class THEMANTEST_API UTheManAnimationAssetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Animation")
	static bool AssignAnimationSkeleton(UAnimationAsset* AnimationAsset, USkeleton* Skeleton);
};
