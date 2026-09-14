#pragma once

#include "Enemy/Boss/BossEnemyBase.h"
#include "GameplayEffectTypes.h"
#include "CoreMorphBoss.generated.h"

class UCoreMorphTailEffects;
class UCoreMorphVisualLayout;
class UCoreMorphFlightComponent;
class UCoreMorphReassemblyComponent;
class UCoreMorphScorpionMovement;
class UCoreMorphScorpionCombat;

UENUM(BlueprintType)
enum class ECoreMorphForm : uint8 { Manta, Scorpion };

UCLASS()
class THEMANTEST_API ACoreMorphBoss : public ABossEnemyBase
{
	GENERATED_BODY()
public:
	ACoreMorphBoss();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void OnDeath() override;
	virtual void ReactToProjectileHit(AActor* HitInstigator) override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CoreMorph") TObjectPtr<UCoreMorphVisualLayout> VisualLayout;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CoreMorph") TObjectPtr<UCoreMorphFlightComponent> Flight;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CoreMorph") TObjectPtr<UCoreMorphReassemblyComponent> Reassembly;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="CoreMorph") TObjectPtr<UCoreMorphScorpionMovement> ScorpionMovement;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="CoreMorph") TObjectPtr<UCoreMorphScorpionCombat> ScorpionCombat;
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="CoreMorph") TObjectPtr<UCoreMorphTailEffects> TailEffects;
	virtual void AimAtTarget(AActor* Target) override;
	UFUNCTION(BlueprintCallable, Category="CoreMorph") bool StartReassembly();
	void SetForm(ECoreMorphForm Form);
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CoreMorph") ECoreMorphForm CurrentForm = ECoreMorphForm::Manta;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CoreMorph") TObjectPtr<AActor> LastThreat;
	UFUNCTION(BlueprintCallable, Category="CoreMorph|Review") bool StartFlightPreview();
	UFUNCTION(BlueprintCallable, Category="CoreMorph|Review") void ResetFlightPreview();
private:
	FActiveGameplayEffectHandle FormEffect;
};
