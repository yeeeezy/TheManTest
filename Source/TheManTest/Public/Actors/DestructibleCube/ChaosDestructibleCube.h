#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChaosDestructibleCube.generated.h"
class UGeometryCollection;
class UGeometryCollectionComponent;

// Placeable Chaos prop; explosion triggering belongs to ExplosionGunBullet.
UCLASS()
class THEMANTEST_API AChaosDestructibleCube : public AActor
{
 GENERATED_BODY()
public:
 AChaosDestructibleCube();
 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Destructible Cube")
 TObjectPtr<UGeometryCollectionComponent> GeometryCollection;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Destructible Cube")
 TObjectPtr<UGeometryCollection> FractureAsset;
 UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Destructible Cube",meta=(ClampMin="1"))
 float Toughness=100000.f;
 virtual void OnConstruction(const FTransform& Transform) override;
 // Explicit editor authoring command, never run during gameplay or construction.
 UFUNCTION(BlueprintCallable,Category="Destructible Cube|Editor",meta=(DevelopmentOnly))
 static UGeometryCollection* CreateTestCubeAsset(bool bRebuild=false);
};
