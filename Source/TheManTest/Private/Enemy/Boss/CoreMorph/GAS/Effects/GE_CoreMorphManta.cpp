#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphManta.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

UGE_CoreMorphManta::UGE_CoreMorphManta()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	FInheritedTagContainer Tags;
	Tags.AddTag(TAG_State_CoreMorph_Form_Manta);
	auto* TargetTags = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("MantaFormTags"));
	GEComponents.Add(TargetTags);
	TargetTags->SetAndApplyTargetTagChanges(Tags);
}
