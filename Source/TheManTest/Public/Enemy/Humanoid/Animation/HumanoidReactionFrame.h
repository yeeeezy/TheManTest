#pragma once
#include "CoreMinimal.h"
#include "HumanoidReactionFrame.generated.h"

USTRUCT(BlueprintType)
struct FHumanoidReactionBones
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Pelvis=TEXT("pelvis");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<FName> Spine={TEXT("spine_01"),TEXT("spine_02"),TEXT("spine_03")};
 UPROPERTY(EditAnywhere,BlueprintReadWrite) TArray<float> SpineWeights={.2f,.35f,.45f};
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName Neck=TEXT("neck_01");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftArm=TEXT("upperarm_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightArm=TEXT("upperarm_r");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftThigh=TEXT("thigh_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftCalf=TEXT("calf_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName LeftFoot=TEXT("foot_l");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightThigh=TEXT("thigh_r");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightCalf=TEXT("calf_r");
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FName RightFoot=TEXT("foot_r");
};

USTRUCT(BlueprintType)
struct FHumanoidReactionFrame
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector Torso=FVector::ZeroVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FVector Follow=FVector::ZeroVector;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) float Compression=0.f;
 UPROPERTY(EditAnywhere,BlueprintReadWrite) FHumanoidReactionBones Bones;
};
