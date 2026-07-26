// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "TheManPlayerState.generated.h"

class UAbilitySystemComponent;
class UTheManAttributeSetBase;

/**
 * ATheManPlayerState
 * 单机游戏架构下，负责跨越肉体生死来保存 GAS 状态的核心容器
 */
UCLASS()
class THEMANTEST_API ATheManPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATheManPlayerState();

	// 实现 IAbilitySystemInterface 接口的核心函数
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 对外暴露获取属性集的接口
	FORCEINLINE UTheManAttributeSetBase* GetAttributeSet() const { return AttributeSet; }

protected:
	// GAS 技能系统组件 (ASC)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UAbilitySystemComponent* AbilitySystemComponent;

	// 全局通用基础属性集
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	UTheManAttributeSetBase* AttributeSet;
};