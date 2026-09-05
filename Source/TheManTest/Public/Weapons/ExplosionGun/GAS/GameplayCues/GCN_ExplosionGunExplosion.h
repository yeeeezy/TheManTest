#pragma once
#include "GameplayCueNotify_Static.h"
#include "GCN_ExplosionGunExplosion.generated.h"
class UNiagaraSystem;
class USoundBase;
class UCameraShakeBase;
UCLASS(Blueprintable)
class THEMANTEST_API UGCN_ExplosionGunExplosion : public UGameplayCueNotify_Static
{
 GENERATED_BODY()
public:
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Explosion", meta=(ClampMin="0.1",Units="s"))
 float EffectLifeSpan=8.f;
 UGCN_ExplosionGunExplosion();
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion") TObjectPtr<UNiagaraSystem> ExplosionEffect;
 // Empty enemy VFX means sound/shake only, never fall back to the ground decal.
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion") TObjectPtr<UNiagaraSystem> EnemyExplosionEffect;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion") TObjectPtr<USoundBase> ExplosionSound;
 // Independent enemy presentation. Empty is intentional; no environment fallback.
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Enemy") TObjectPtr<USoundBase> EnemyExplosionSound;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Enemy",meta=(ClampMin="0")) float EnemyVolumeMultiplier=3.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Enemy",meta=(ClampMin="0.01")) float EnemyEffectScale=1.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion",meta=(ClampMin="0.01")) float EffectScale=1.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion",meta=(ClampMin="0.0")) float VolumeMultiplier=1.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera") TSubclassOf<UCameraShakeBase> CameraShakeClass;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="0.0")) float CameraShakeScale=3.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="0.1",ClampMax="2",Units="s")) float ShakeDuration=.75f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="0",ClampMax="3")) float ShakeRotationDegrees=1.5f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="1",ClampMax="30")) float ShakeFrequency=12.f;
 USoundBase* GetExplosionSound(bool bEnemy) const { return bEnemy?EnemyExplosionSound.Get():ExplosionSound.Get(); }
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="0.0",Units="cm")) float ShakeInnerRadius=200.f;
 UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Explosion|Camera",meta=(ClampMin="1.0",Units="cm")) float ShakeOuterRadius=1800.f;
 float GetShakeScaleAtDistance(float Distance) const;
 virtual bool OnExecute_Implementation(AActor* Target,const FGameplayCueParameters& Parameters) const override;
};
