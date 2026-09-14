#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphMissileCooldown.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGE_CoreMorphMissileCooldown::UGE_CoreMorphMissileCooldown()
{
 DurationPolicy=EGameplayEffectDurationType::HasDuration;
 DurationMagnitude=FScalableFloat(6.f);
 FInheritedTagContainer Tags;Tags.AddTag(TAG_State_CoreMorph_MissileCooldown);
 auto* C=CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("MissileCooldownTags"));GEComponents.Add(C);C->SetAndApplyTargetTagChanges(Tags);
}
