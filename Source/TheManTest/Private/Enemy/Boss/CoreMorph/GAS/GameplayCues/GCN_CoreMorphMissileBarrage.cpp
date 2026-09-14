#include "Enemy/Boss/CoreMorph/GAS/GameplayCues/GCN_CoreMorphMissileBarrage.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphMissileEffects.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
UGCN_CoreMorphMissileBarrage::UGCN_CoreMorphMissileBarrage(){GameplayCueTag=TAG_GameplayCue_CoreMorph_MissileBarrage;}
bool UGCN_CoreMorphMissileBarrage::OnActive_Implementation(AActor* Target,const FGameplayCueParameters& P) const {if(auto* B=Cast<ACoreMorphBoss>(Target)){B->MissileEffects->BeginCue();return true;}return false;}
bool UGCN_CoreMorphMissileBarrage::OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& P) const {if(auto* B=Cast<ACoreMorphBoss>(Target)){B->MissileEffects->EndCue();return true;}return false;}
