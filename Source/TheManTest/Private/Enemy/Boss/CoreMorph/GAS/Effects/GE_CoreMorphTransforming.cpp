#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphTransforming.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

UGE_CoreMorphTransforming::UGE_CoreMorphTransforming()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	FInheritedTagContainer Tags;
	Tags.AddTag(TAG_State_CoreMorph_Transforming);
	auto* TargetTags = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("TransformingFormTags"));
	GEComponents.Add(TargetTags);
	TargetTags->SetAndApplyTargetTagChanges(Tags);
}
