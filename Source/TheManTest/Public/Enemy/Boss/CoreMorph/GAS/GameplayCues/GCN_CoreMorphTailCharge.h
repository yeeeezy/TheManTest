#pragma once
#include "GameplayCueNotify_Static.h"
#include "GCN_CoreMorphTailCharge.generated.h"
UCLASS()
class THEMANTEST_API UGCN_CoreMorphTailCharge : public UGameplayCueNotify_Static
{
 GENERATED_BODY()
public:
 UGCN_CoreMorphTailCharge();
 virtual bool OnActive_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
 virtual bool OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
};
