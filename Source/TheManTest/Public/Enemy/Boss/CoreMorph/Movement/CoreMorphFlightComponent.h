#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightPath.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightMotion.h"
#include "CoreMorphFlightComponent.generated.h"

class UStaticMeshComponent;
class ACoreMorphBoss;
class ACoreMorphFlightRoute;
class UCoreMorphVisualLayout;
DECLARE_MULTICAST_DELEGATE(FCoreMorphFlightFinished);

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UCoreMorphFlightComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCoreMorphFlightComponent();
	// Empty uses the source reference route. Spline flights begin at its first point.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category="CoreMorph|Flight") TObjectPtr<ACoreMorphFlightRoute> FlightRoute;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Flight", meta=(ClampMin="0", Units="cm/s")) float RouteSpeed = 11000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Flight", meta=(ClampMin="1")) float RouteAcceleration = 16000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Motion", meta=(ClampMin="0", ClampMax="1")) float MotionRandomness = .18f;
	// Zero picks a new seed on reset. Nonzero gives repeatable motion for review.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Motion") int32 MotionSeed = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Motion") bool bRandomRolls = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CoreMorph|Motion", meta=(ClampMin="1000", Units="cm")) float RandomRollSpacing = 65000.f;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Function) override;
	void RebuildAssembly();
	bool StartFlight();
	bool CanStartFlight() const;
	// Presentation maneuver inside the active flight GA; no dodge or damage state.
	bool RequestRoll(bool bFast, int32 Direction = 1);
	void StopFlight();
	void ResetPreview();
	void Shutdown();
	bool IsFlying() const { return bFlying; }
	float GetFlightSeconds() const { return FlightSeconds; }
	double GetRouteDistance() const { return RouteDistance; }
	float GetReleaseSeconds() const { return Path.GetReleaseSeconds(); }
	bool IsPaused() const { return bPaused; }
	void SetPaused(bool bValue) { bPaused = bValue; }
	const TArray<TObjectPtr<UStaticMeshComponent>>& GetPieces() const { return Pieces; }
	const FTransform& GetChoreographyFrame() const { return ChoreographyFrame; }
	const FCoreMorphFlightMotion& GetMotionState() const { return Motion; }
	FCoreMorphFlightFinished OnFlightFinished;
private:
	UPROPERTY(Transient) TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
	FCoreMorphFlightPath Path;
	UPROPERTY(Transient) TObjectPtr<UCoreMorphVisualLayout> Layout;
	FCoreMorphFlightMotion Motion;
	TWeakObjectPtr<ACoreMorphFlightRoute> ActiveRoute;
	FTransform ChoreographyFrame;
	bool bHaveFrame = false, bFlying = false, bPaused = false, bHolding = false, bUsingRoute = false;
	float FlightSeconds = 0;
	float CurrentRouteSpeed = 0;
	double RouteDistance = 0;
	ACoreMorphBoss* Boss() const;
	void ResetMotion(const FTransform& Body);
	FTransform RestBody() const;
	void UpdatePose();
};
