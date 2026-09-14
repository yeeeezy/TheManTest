#include "Enemy/Boss/CoreMorph/GAS/Effects/GE_CoreMorphAttacking.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGE_CoreMorphAttacking::UGE_CoreMorphAttacking()
{
 DurationPolicy=EGameplayEffectDurationType::Infinite;
 FInheritedTagContainer Tags;Tags.AddTag(TAG_State_CoreMorph_Attacking);
 auto* C=CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>(TEXT("AttackingTags"));GEComponents.Add(C);C->SetAndApplyTargetTagChanges(Tags);
}
