// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h" // 必须包含此头文件以使用宏
#include "TheManAttributeSetBase.generated.h"

// 🌟 3A 级标准：定义 GAS 属性访问器宏
// 这段宏是官方标配，它会自动帮我们生成 Value 的 Getter、Setter 以及 Init 函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class THEMANTEST_API UTheManAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UTheManAttributeSetBase();

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// --- 核心血量属性 ---
	
	// 当前血量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UTheManAttributeSetBase, Health); // 使用上面定义的宏

	// 最大血量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UTheManAttributeSetBase, MaxHealth);
};