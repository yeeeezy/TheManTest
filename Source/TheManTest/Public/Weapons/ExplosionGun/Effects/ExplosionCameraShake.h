#pragma once
#include "Camera/CameraShakeBase.h"
#include "ExplosionCameraShake.generated.h"

UCLASS()
class THEMANTEST_API UExplosionCameraShakePattern : public UCameraShakePattern
{
 GENERATED_BODY()
public:
 UPROPERTY(EditAnywhere, Category="Explosion") float Duration=0.45f;
 UPROPERTY(EditAnywhere, Category="Explosion") float Displacement=5.f;
 UPROPERTY(EditAnywhere, Category="Explosion") float RotationDegrees=1.5f;
protected:
 virtual void GetShakePatternInfoImpl(FCameraShakeInfo& Info) const override;
 virtual void StartShakePatternImpl(const FCameraShakePatternStartParams& Params) override { Elapsed=0.f; }
 virtual void UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,FCameraShakePatternUpdateResult& Result) override;
 virtual bool IsFinishedImpl() const override { return Elapsed>=Duration; }
 virtual void StopShakePatternImpl(const FCameraShakePatternStopParams& Params) override { Elapsed=Duration; }
private:
 float Elapsed=0.f;
};

UCLASS(Blueprintable)
class THEMANTEST_API UExplosionCameraShake : public UCameraShakeBase
{
 GENERATED_BODY()
public:
 UExplosionCameraShake(const FObjectInitializer& ObjectInitializer);
};
