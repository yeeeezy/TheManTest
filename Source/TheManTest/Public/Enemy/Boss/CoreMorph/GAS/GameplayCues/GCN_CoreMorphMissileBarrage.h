#pragma once
#include "GameplayCueNotify_Static.h"
#include "GCN_CoreMorphMissileBarrage.generated.h"
UCLASS()
class THEMANTEST_API UGCN_CoreMorphMissileBarrage : public UGameplayCueNotify_Static
{
 GENERATED_BODY()
public:
 UGCN_CoreMorphMissileBarrage();
 virtual bool OnActive_Implementation(AActor* Target,const FGameplayCueParameters& P) const override;
 virtual bool OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& P) const override;
};
