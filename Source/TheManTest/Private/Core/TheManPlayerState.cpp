// Fill out your copyright notice in the Description page of Project Settings.

#include "Core/TheManPlayerState.h"
#include "AbilitySystemComponent.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"

ATheManPlayerState::ATheManPlayerState()
{
	// 1. 实例化核心大脑组件 ASC (纯单机模式，无需任何网络同步设置)
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 2. 挂载通用的基础属性集
	AttributeSet = CreateDefaultSubobject<UTheManAttributeSetBase>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ATheManPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}