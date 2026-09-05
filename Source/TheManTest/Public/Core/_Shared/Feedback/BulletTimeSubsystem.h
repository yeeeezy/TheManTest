#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BulletTimeSubsystem.generated.h"

USTRUCT(BlueprintType)
struct THEMANTEST_API FBulletTimeSettings
{
 GENERATED_BODY()
 UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEnabled=true;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.01",ClampMax="1")) float TimeScale=.2f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.01",ClampMax="2",Units="s")) float SlowInDuration=.05f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0",ClampMax="2",Units="s")) float HoldDuration=.08f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.01",ClampMax="2",Units="s")) float RecoveryDuration=.25f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0",Units="cm")) float InnerRadius=200.f;
 UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1",Units="cm")) float OuterRadius=1500.f;
 float GetStrength(float Distance) const;
 float GetDuration() const { return SlowInDuration+HoldDuration+RecoveryDuration; }
 float Evaluate(float RealSeconds) const;
};

/** Single-player real-time envelope. Requests during an active envelope are coalesced,
 * never restart the ramp or prevent recovery. Presentation Cues do not own game time. */
UCLASS()
class THEMANTEST_API UBulletTimeSubsystem : public UTickableWorldSubsystem
{
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable, Category="Feedback|Bullet Time") bool RequestBulletTimeAtLocation(FVector Location,const FBulletTimeSettings& Settings);
 UFUNCTION(BlueprintCallable, Category="Feedback|Bullet Time") bool RequestBulletTime(const FBulletTimeSettings& Settings);
 UFUNCTION(BlueprintCallable, Category="Feedback|Bullet Time") void CancelBulletTime();
 UFUNCTION(BlueprintPure, Category="Feedback|Bullet Time") bool IsBulletTimeActive() const { return bActive; }
 virtual void Tick(float DeltaTime) override;
 virtual bool IsTickable() const override { return bActive&&!IsTemplate(); }
 virtual bool IsTickableWhenPaused() const override { return true; }
 virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UBulletTimeSubsystem,STATGROUP_Tickables); }
 virtual void OnWorldEndPlay(UWorld& World) override;
 virtual void Deinitialize() override;
protected:
 virtual bool DoesSupportWorldType(EWorldType::Type Type) const override { return Type==EWorldType::Game||Type==EWorldType::PIE; }
private:
 FBulletTimeSettings ActiveSettings;
 bool bActive=false;
 float OriginalDilation=1.f, AppliedDilation=1.f;
 double StartRealTime=0, RecoveryUntil=0;
};
