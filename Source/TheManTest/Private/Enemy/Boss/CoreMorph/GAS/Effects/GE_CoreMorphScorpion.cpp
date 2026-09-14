#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphScorpion.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

UGE_CoreMorphScorpion::UGE_CoreMorphScorpion()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	FInheritedTagContainer Tags;
	Tags.AddTag(TAG_State_CoreMorph_Form_Scorpion);
	auto* TargetTags = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("ScorpionFormTags"));
	GEComponents.Add(TargetTags);
	TargetTags->SetAndApplyTargetTagChanges(Tags);
}
