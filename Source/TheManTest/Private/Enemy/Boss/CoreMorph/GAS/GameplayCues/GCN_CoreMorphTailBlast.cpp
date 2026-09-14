#include "Enemy/Boss/CoreMorph/GAS/GameplayCues/GCN_CoreMorphTailBlast.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphTailEffects.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGCN_CoreMorphTailBlast::UGCN_CoreMorphTailBlast(){GameplayCueTag=TAG_GameplayCue_CoreMorph_TailBlast;}
bool UGCN_CoreMorphTailBlast::OnActive_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{if(auto* B=Cast<ACoreMorphBoss>(Target)){if(!B->IsDead())B->TailEffects->BeginBlast(Parameters);return true;}return false;}
bool UGCN_CoreMorphTailBlast::OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{if(auto* B=Cast<ACoreMorphBoss>(Target)){B->TailEffects->EndBlast();return true;}return false;}
