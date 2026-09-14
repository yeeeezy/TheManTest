#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightPath.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphTailMotion.h"
#include "CoreMorphFlightComponent.generated.h"

class UStaticMeshComponent;
class ACoreMorphBoss;
DECLARE_MULTICAST_DELEGATE(FCoreMorphFlightFinished);

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphFlightComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCoreMorphFlightComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Function) override;
	void RebuildAssembly();
	bool StartFlight();
	bool CanStartFlight() const;
	void StopFlight();
	void ResetPreview();
	void Shutdown();
	bool IsFlying() const { return bFlying; }
	float GetFlightSeconds() const { return FlightSeconds; }
	float GetReleaseSeconds() const { return Path.GetReleaseSeconds(); }
	bool IsPaused() const { return bPaused; }
	void SetPaused(bool bValue) { bPaused = bValue; }
	const TArray<TObjectPtr<UStaticMeshComponent>>& GetPieces() const { return Pieces; }
	const FTransform& GetChoreographyFrame() const { return ChoreographyFrame; }
	const FCoreMorphTailMotion& GetTailMotion() const { return TailMotion; }
	FCoreMorphFlightFinished OnFlightFinished;
private:
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
	FCoreMorphFlightPath Path;
	FCoreMorphTailMotion TailMotion;
	FTransform ChoreographyFrame;
	bool bHaveFrame = false, bFlying = false, bPaused = false, bHolding = false;
	float FlightSeconds = 0, IdleSeconds = 0;
	ACoreMorphBoss* Boss() const;
	void UpdatePose();
};
