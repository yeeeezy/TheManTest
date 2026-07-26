#pragma once

#include "NativeGameplayTags.h"

// 武器输入事件 Tag
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Weapon_PrimaryFire)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Weapon_SecondaryFire)

// 通用交互 Tag：E 键触发，各角色授予不同技能监听此 Tag 实现差异化交互
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Character_Interact)

// SetByCaller 数据 Tag：子弹命中时把 Damage 数值传入伤害 GE，由 GE 的 Health 修改器读取
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_Damage)
