#include "Enemy/Boss/CoreMorph/GAS/GameplayCues/GCN_CoreMorphTailCharge.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphTailEffects.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGCN_CoreMorphTailCharge::UGCN_CoreMorphTailCharge(){GameplayCueTag=TAG_GameplayCue_CoreMorph_TailCharge;}
bool UGCN_CoreMorphTailCharge::OnActive_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{if(auto* B=Cast<ACoreMorphBoss>(Target)){if(!B->IsDead())B->TailEffects->BeginCharge(Parameters);return true;}return false;}
bool UGCN_CoreMorphTailCharge::OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const
{if(auto* B=Cast<ACoreMorphBoss>(Target)){B->TailEffects->EndCharge();return true;}return false;}
