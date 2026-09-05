#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"
#include "HAL/PlatformTime.h"

UExplosionCameraShake::UExplosionCameraShake(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
 bSingleInstance=true;
 SetRootShakePattern(CreateDefaultSubobject<UExplosionCameraShakePattern>(TEXT("ExplosionImpulse")));
}
void UExplosionCameraShakePattern::GetShakePatternInfoImpl(FCameraShakeInfo& Info) const
{
 Info.Duration=FCameraShakeDuration(Duration);
 Info.BlendIn=0.f;Info.BlendOut=0.f;
}
void UExplosionCameraShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,FCameraShakePatternUpdateResult& Result)
{
 Elapsed=GetWorld()?float(FPlatformTime::Seconds()-StartedAt):Elapsed+Params.DeltaTime;
 const float T=FMath::Clamp(Elapsed/FMath::Max(.01f,Duration),0.f,1.f);
 const float Envelope=FMath::Pow(1.f-T,1.25f)*FMath::Clamp(Elapsed/.015f,0.f,1.f);
 const float Phase=Elapsed*2.f*PI*Frequency;
 const float Impact=FMath::Exp(-Elapsed*30.f);
 const float Kick=Envelope*(Impact+.8f*FMath::Sin(Phase));
 // Never translate the post-modifier view away from the capsule/viewmodel camera.
 Result.Location=FVector::ZeroVector;
 Result.Rotation=FRotator(RotationDegrees*Kick,RotationDegrees*.25f*Envelope*FMath::Sin(Phase*.83f),RotationDegrees*.4f*Envelope*FMath::Sin(Phase*1.17f));
}
void UExplosionCameraShakePattern::StartShakePatternImpl(const FCameraShakePatternStartParams& Params)
{
 Elapsed=0;StartedAt=FPlatformTime::Seconds();
}
