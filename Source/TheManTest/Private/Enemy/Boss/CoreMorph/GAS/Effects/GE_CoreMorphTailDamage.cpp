#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailDamage.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
UGE_CoreMorphTailDamage::UGE_CoreMorphTailDamage()
{
 DurationPolicy=EGameplayEffectDurationType::Instant;
 FSetByCallerFloat Damage;Damage.DataTag=TAG_Data_Damage;
 auto& M=Modifiers.AddDefaulted_GetRef();M.Attribute=UTheManAttributeSetBase::GetHealthAttribute();
 M.ModifierOp=EGameplayModOp::Additive;M.ModifierMagnitude=FGameplayEffectModifierMagnitude(Damage);
}

