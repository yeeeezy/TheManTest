#pragma once
#include "GameplayCueNotify_Static.h"
#include "GCN_ExplosionGunExplosion.generated.h"
class UNiagaraSystem;
class USoundBase;
UCLASS(Blueprintable)
class THEMANTEST_API UGCN_ExplosionGunExplosion : public UGameplayCueNotify_Static
{
 GENERATED_BODY()
public:
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.1",Units="s"))
 float EffectLifeSpan=8.f;
 UGCN_ExplosionGunExplosion();
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion") TObjectPtr<UNiagaraSystem> ExplosionEffect;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion") TObjectPtr<USoundBase> ExplosionSound;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion",meta=(ClampMin="0.01")) float EffectScale=1.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion",meta=(ClampMin="0.0")) float VolumeMultiplier=1.f;
 virtual bool OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
};
