#pragma once
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "CoreMorphTailEffects.generated.h"
class UInstancedStaticMeshComponent;
class UDecalComponent;
class UPointLightComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;
UCLASS(ClassGroup=(Effects),meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphTailEffects : public UActorComponent
{
 GENERATED_BODY()
public:
 UCoreMorphTailEffects();
 virtual void TickComponent(float Dt,ELevelTick Tick,FActorComponentTickFunction* Function) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UPROPERTY(EditDefaultsOnly,Category="Tail Effects") TObjectPtr<UMaterialInterface> EnergyMaterial;
 UPROPERTY(EditDefaultsOnly,Category="Tail Effects") TObjectPtr<UMaterialInterface> WarningMaterial;
 UPROPERTY(EditDefaultsOnly,Category="Tail Effects") TObjectPtr<UMaterialInterface> LightningMaterial;
 void BeginCharge(const FGameplayCueParameters& P);
 void EndCharge();
 void BeginBlast(const FGameplayCueParameters& P);
 void EndBlast();
 void Shutdown();
 bool IsCharging() const {return bCharging;}
 bool IsBlasting() const {return bBlasting;}
 FVector GetWarningCenter() const {return WarningCenter;}
 float GetWarningRadius() const {return WarningRadius;}
private:
 UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> Orbs;
 UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> ChargeArcs;
 UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> BlastArcs;
 UPROPERTY(Transient) TObjectPtr<UInstancedStaticMeshComponent> BlastCore;
 UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> BlastCoreMID;
 UPROPERTY(Transient) TObjectPtr<UDecalComponent> Warning;
 UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> WarningMID;
 UPROPERTY(Transient) TObjectPtr<UPointLightComponent> ChargeLight;
 UPROPERTY(Transient) TObjectPtr<UPointLightComponent> BlastLight;
 FVector WarningCenter=FVector::ZeroVector,BlastCenter=FVector::ZeroVector;
 FVector BlastNormal=FVector::UpVector;
 float WarningRadius=0,BlastRadius=0,ChargeClock=0,BlastClock=0;
 bool bCharging=false,bBlasting=false;
 UInstancedStaticMeshComponent* MakePool(const TCHAR* Mesh,UMaterialInterface* Material);
 UPointLightComponent* MakeLight();
 void DrawCharge(float Dt);
 void DrawBlast(float Dt);
};
