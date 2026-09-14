#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphMissileDamage.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
UGE_CoreMorphMissileDamage::UGE_CoreMorphMissileDamage()
{
 DurationPolicy=EGameplayEffectDurationType::Instant;
 FSetByCallerFloat Damage;Damage.DataTag=TAG_Data_Damage;
 auto& M=Modifiers.AddDefaulted_GetRef();M.Attribute=UTheManAttributeSetBase::GetHealthAttribute();
 M.ModifierOp=EGameplayModOp::Additive;M.ModifierMagnitude=FGameplayEffectModifierMagnitude(Damage);
}

