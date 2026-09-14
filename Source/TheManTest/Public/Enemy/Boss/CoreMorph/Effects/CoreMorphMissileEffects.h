#pragma once
#include "Components/ActorComponent.h"
#include "CoreMorphMissileEffects.generated.h"
class UInstancedStaticMeshComponent;class UDecalComponent;class UPointLightComponent;class UMaterialInterface;class UMaterialInstanceDynamic;
UCLASS(ClassGroup=(Effects),meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphMissileEffects : public UActorComponent
{
 GENERATED_BODY()
public:
 UCoreMorphMissileEffects();
 virtual void TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditDefaultsOnly,Category="Missile Effects") TObjectPtr<UMaterialInterface> HullMaterial;
 UPROPERTY(EditDefaultsOnly,Category="Missile Effects") TObjectPtr<UMaterialInterface> EnergyMaterial;
 UPROPERTY(EditDefaultsOnly,Category="Missile Effects") TObjectPtr<UMaterialInterface> BlastMaterial;
 UPROPERTY(EditDefaultsOnly,Category="Missile Effects") TObjectPtr<UMaterialInterface> WarningMaterial;
 void BeginCue();void EndCue();
 bool HasCue() const {return bActive;}
 int32 GetWarningCount() const;
private:
 UPROPERTY(Transient) TArray<TObjectPtr<UInstancedStaticMeshComponent>> Pools;
 UPROPERTY(Transient) TArray<TObjectPtr<UDecalComponent>> Warnings;
 UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> WarningMIDs;
 UPROPERTY(Transient) TArray<TObjectPtr<UPointLightComponent>> Lights;
 bool bActive=false;
 UInstancedStaticMeshComponent* MakePool(const TCHAR* Mesh,UMaterialInterface* Material);
 void Draw();
};
