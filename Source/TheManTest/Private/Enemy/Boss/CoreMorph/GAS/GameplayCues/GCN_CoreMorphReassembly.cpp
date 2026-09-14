#include "Enemy/Boss/CoreMorph/GAS/GameplayCues/GCN_CoreMorphReassembly.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGCN_CoreMorphReassembly::UGCN_CoreMorphReassembly(){GameplayCueTag=TAG_GameplayCue_CoreMorph_Reassembly;}
bool UGCN_CoreMorphReassembly::OnActive_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{
    if(auto* Boss=Cast<ACoreMorphBoss>(Target)){if(!Boss->IsDead())Boss->Reassembly->BeginCue();return true;}
    return false;
}
bool UGCN_CoreMorphReassembly::OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{
    if(auto* Boss=Cast<ACoreMorphBoss>(Target)){Boss->Reassembly->EndCue();return true;}
    return false;
}
