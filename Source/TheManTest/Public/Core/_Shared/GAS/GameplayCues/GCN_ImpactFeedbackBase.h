#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_ImpactFeedbackBase.generated.h"

class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;

UCLASS(Abstract, Blueprintable)
class THEMANTEST_API UGCN_ImpactFeedbackBase : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	// Optional character-specific hit flash. When configured, character hits use
	// this effect while walls and props continue to use ImpactEffect.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<UNiagaraSystem> CharacterImpactEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float PitchMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact", meta = (ClampMin = "0.0"))
	float EffectScale = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Decal")
	TObjectPtr<UMaterialInterface> ImpactDecalMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Decal", meta = (ClampMin = "0.0"))
	float DecalSizeMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Decal", meta = (ClampMin = "0.0"))
	float DecalLifeSpan = 10.f;

	// The source VFX setup leaves decals on the environment only. Keep that as
	// the default while allowing other cues to opt in for character decals.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact|Decal")
	bool bSpawnDecalOnCharacters = false;

	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
};
