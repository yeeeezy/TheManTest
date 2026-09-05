#pragma once
#include "CoreMinimal.h"
#include "HumanoidReactionBones.generated.h"

USTRUCT(BlueprintType)
struct FHumanoidReactionBones
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Neck=TEXT("neck_01");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftArm=TEXT("upperarm_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightArm=TEXT("upperarm_r");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftThigh=TEXT("thigh_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightThigh=TEXT("thigh_r");
};
