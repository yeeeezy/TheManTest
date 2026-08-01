// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TheManCharacterDataAssetBase.generated.h"

/**
 * UTheManCharacterDataAssetBase
 * 基础角色数据资产基类
 * 负责在虚幻引擎原生资产中配置角色的共有基础初始数据（当前仅包含血量）
 */
UCLASS()
class THEMANTEST_API UTheManCharacterDataAssetBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* ==========================================
	 * 🩸 基础通用属性 (Base Attributes)
	 * ========================================== */
    
	// 角色出生时的初始最大血量上限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Base", meta = (ClampMin = "0.0"))
	float InitialMaxHealth = 100.0f;

	// 角色出生时的初始当前血量
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes|Base", meta = (ClampMin = "0.0"))
	float InitialHealth = 100.0f;
};