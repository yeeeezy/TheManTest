#pragma once
#include "GameplayCueNotify_Static.h"
#include "GCN_CoreMorphReassembly.generated.h"

UCLASS()
class THEMANTEST_API UGCN_CoreMorphReassembly : public UGameplayCueNotify_Static
{
    GENERATED_BODY()
public:
    UGCN_CoreMorphReassembly();
    virtual bool OnActive_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
    virtual bool OnRemove_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
};
