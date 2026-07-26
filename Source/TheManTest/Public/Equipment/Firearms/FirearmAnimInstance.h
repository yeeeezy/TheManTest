// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Animation/EquipmentAnimInstance.h"
#include "FirearmAnimInstance.generated.h"

UCLASS()
class THEMANTEST_API UFirearmAnimInstance : public UEquipmentAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void SetAimSourceLocalTransform(const FTransform& InTransform) { AimSourceLocalTransform = InTransform; }

protected:
	UPROPERTY(BlueprintReadWrite, Category = "AimIK")
	FTransform AimSourceLocalTransform;

	UPROPERTY(BlueprintReadWrite, Category = "AimIK")
	FVector AimTargetComponentSpace;

	UPROPERTY(BlueprintReadWrite, Category = "AimIK")
	bool bHasValidAimTarget;

	UPROPERTY(BlueprintReadWrite, Category = "AimIK")
	bool bIsAiming;
};
