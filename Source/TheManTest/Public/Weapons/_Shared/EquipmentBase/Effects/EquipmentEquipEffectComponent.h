#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentEquipEffectComponent.generated.h"

class UMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

USTRUCT()
struct FEquipmentEffectMaterial
{
    GENERATED_BODY()
    UPROPERTY(Transient) TWeakObjectPtr<UMeshComponent> Mesh;
    UPROPERTY(Transient) TObjectPtr<UMaterialInterface> Original;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> Dynamic;
    int32 Slot = 0;
};

// Shared reveal effect for every EquipmentBase, independent of its animation layer.
UCLASS(ClassGroup=(Equipment), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UEquipmentEquipEffectComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UEquipmentEquipEffectComponent();
    void Play();
    void Stop();
    bool IsPlaying() const { return bPlaying; }
    float GetElapsed() const { return Elapsed; }
    float GetAmount() const { return Amount; }
    static constexpr float Duration = 0.5f;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient) TArray<FEquipmentEffectMaterial> Materials;
    float Elapsed = 0.f;
    float Amount = 0.f;
    bool bPlaying = false;
};
