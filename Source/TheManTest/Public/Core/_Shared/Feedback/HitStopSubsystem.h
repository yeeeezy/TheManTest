#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HitStopSubsystem.generated.h"

USTRUCT(BlueprintType)
struct THEMANTEST_API FHitStopSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="0.5", Units="s")) float Duration = .06f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.001", ClampMax="1")) float TimeScale = .05f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", Units="cm")) float InnerRadius = 200.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", Units="cm")) float OuterRadius = 1500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="0.5", Units="s")) float MaxContinuousDuration = .12f;
	float GetStrength(float Distance) const;
};

/** Single-player world feedback. Never owned by a projectile or a Gameplay Cue. */
UCLASS()
class THEMANTEST_API UHitStopSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Feedback|Hit Stop")
	bool RequestHitStopAtLocation(FVector Location, const FHitStopSettings& Settings);
	UFUNCTION(BlueprintCallable, Category="Feedback|Hit Stop")
	bool RequestHitStop(float Duration, float TimeScale, float MaxContinuousDuration=.12f);
	UFUNCTION(BlueprintCallable, Category="Feedback|Hit Stop") void CancelHitStop();
	UFUNCTION(BlueprintPure, Category="Feedback|Hit Stop") bool IsHitStopActive() const { return bActive; }
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return bActive && !IsTemplate(); }
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UHitStopSubsystem, STATGROUP_Tickables); }
	virtual void OnWorldEndPlay(UWorld& World) override;
	virtual void Deinitialize() override;
protected:
	virtual bool DoesSupportWorldType(EWorldType::Type Type) const override { return Type == EWorldType::Game || Type == EWorldType::PIE; }
private:
	bool bActive = false;
	float OriginalDilation = 1.f, AppliedDilation = 1.f;
	double StartRealTime = 0, EndRealTime = 0, CapRealTime = 0, RecoveryUntil = 0;
};
