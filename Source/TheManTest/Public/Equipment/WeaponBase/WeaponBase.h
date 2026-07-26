// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquipmentBase/EquipmentBase.h" 
#include "WeaponBase.generated.h" 

UCLASS()
// 3. 将 public AActor 修改为 public AEquipmentBase
class THEMANTEST_API AWeaponBase : public AEquipmentBase
{
	GENERATED_BODY()
    
public: 
	// Sets default values for this actor's properties
	AWeaponBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public: 
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};