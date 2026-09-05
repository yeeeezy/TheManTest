#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBloodSpray.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

// Short-lived cosmetic droplets owned by the enemy Hit Cue, never by a weapon.
UCLASS(NotBlueprintable, Transient)
class THEMANTEST_API AEnemyBloodSpray : public AActor
{
 GENERATED_BODY()
public:
 AEnemyBloodSpray();
 void Initialize(UMaterialInterface* Material, const FVector& Direction, float Scale);
 virtual void Tick(float DeltaSeconds) override;
private:
 UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Droplets;
 UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> SprayMaterial;
 TArray<FVector> Velocities;
 float Age=0.f;
 static constexpr float Duration=0.55f;
};
