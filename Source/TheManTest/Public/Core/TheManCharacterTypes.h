// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" 
#include "TheManCharacterTypes.generated.h"

// 前向声明当前活跃的 FPS 角色基类
class AFPSCharacterBase;

/**
 * FCharacterType
 * 角色类型数据结构
 * 记录角色的 UI 显示信息以及对应的物理蓝图类，作为 DataTable 的数据行使用
 */
USTRUCT(BlueprintType)
struct THEMANTEST_API FCharacterType : public FTableRowBase
{
	GENERATED_BODY()



	// UI 上展示的角色名字
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData")
	FText CharacterDisplayName;

	// 🌟 新增：角色描述 (带 MultiLine 魔法，在编辑器里会变成一个可以换行的大输入框！)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData", meta = (MultiLine = "true"))
	FText CharacterDescription;

	// UI 选人界面的角色头像
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData")
	class UTexture2D* CharacterIcon = nullptr;

	// 核心：这个角色对应的物理肉体蓝图类（当前指向 AFPSCharacterBase 子类）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterData")
	TSubclassOf<AFPSCharacterBase> CharacterClass;
};
