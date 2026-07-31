#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyCoverPoint.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS(Blueprintable)
class THEMANTEST_API AEnemyCoverPoint : public AActor
{
	GENERATED_BODY()

public:
	AEnemyCoverPoint();

	UFUNCTION(BlueprintPure, Category="Enemy|Cover")
	FVector GetStandLocation() const;

	// 在场景内选择兼顾距离、目标背向和真实遮挡的最佳掩体；不依赖具体 Enemy 类型。
	UFUNCTION(BlueprintCallable, Category="Enemy|Cover", meta=(WorldContext="WorldContextObject"))
	static AEnemyCoverPoint* FindBestCover(const UObject* WorldContextObject, const FVector& SeekerLocation,
		const FVector& ThreatLocation, float MaxDistance = 2000.f);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CoverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> CoverCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> StandPoint;
};
