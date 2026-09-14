#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTailCooldown.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGE_CoreMorphTailCooldown::UGE_CoreMorphTailCooldown()
{
 DurationPolicy=EGameplayEffectDurationType::HasDuration;
 DurationMagnitude=FScalableFloat(2.1f);
 FInheritedTagContainer Tags;Tags.AddTag(TAG_State_CoreMorph_TailCooldown);
 auto* C=CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TailCooldownTags"));GEComponents.Add(C);C->SetAndApplyTargetTagChanges(Tags);
}
