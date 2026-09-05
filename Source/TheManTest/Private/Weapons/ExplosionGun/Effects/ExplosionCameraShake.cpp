#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"

UExplosionCameraShake::UExplosionCameraShake(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
 bSingleInstance=false;
 SetRootShakePattern(CreateDefaultSubobject<UExplosionCameraShakePattern>(TEXT("ExplosionImpulse")));
}
void UExplosionCameraShakePattern::GetShakePatternInfoImpl(FCameraShakeInfo& Info) const
{
 Info.Duration=FCameraShakeDuration(Duration);
 Info.BlendIn=0.f;Info.BlendOut=0.f;
}
void UExplosionCameraShakePattern::UpdateShakePatternImpl(const FCameraShakePatternUpdateParams& Params,FCameraShakePatternUpdateResult& Result)
{
 Elapsed+=Params.DeltaTime;
 const float T=FMath::Clamp(Elapsed/FMath::Max(.01f,Duration),0.f,1.f);
 const float Envelope=FMath::Square(1.f-T)*FMath::Min(T*25.f,1.f);
 const float Kick=Envelope*(.7f+FMath::Sin(Elapsed*85.f)*.3f);
 // +X play-space is the pressure travelling from the explosion toward the camera.
 Result.Location=FVector(Displacement*Kick,0,Displacement*.15f*Envelope*FMath::Sin(Elapsed*110.f));
 Result.Rotation=FRotator(RotationDegrees*Kick,0,RotationDegrees*.2f*Envelope*FMath::Sin(Elapsed*95.f));
}
