#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "GCN_ImpactFeedbackBase.generated.h"

class UNiagaraSystem;
class USoundBase;
class UMaterialInterface;
class USoundAttenuation;
class USoundConcurrency;
class UAudioComponent;
class UDecalComponent;

UCLASS(Abstract, Blueprintable)
class THEMANTEST_API UGCN_ImpactFeedbackBase : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UGCN_ImpactFeedbackBase();
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Impact|Audio")
	TObjectPtr<USoundAttenuation> ImpactAttenuation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Impact|Audio")
	TObjectPtr<USoundConcurrency> ImpactConcurrency;
	// Random pitch/volume belong to the Sound Cue, never to the gameplay caller.
	virtual bool ShouldPlayImpactSound(bool bCharacterImpact) const { return !bCharacterImpact; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Impact|Decal", meta=(ClampMin="0",ClampMax="0.8"))
	float DecalSizeVariation=.3f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Impact|Decal")
	bool bRandomizeDecals=true;
	UAudioComponent* SpawnImpactSound(UWorld* World,const FVector& Location,bool bCharacterImpact) const;
	static FRotator MakeDecalRotation(const FVector& Normal,bool bRandomize);
	static void RandomizeDecalMaterial(UDecalComponent* Decal);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

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

	virtual bool OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const override;
};
