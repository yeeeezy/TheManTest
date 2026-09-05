#include "Enemy/_Shared/Audio/EnemyHitAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformTime.h"
#include "GameFramework/Actor.h"

bool UEnemyHitAudioComponent::TryPlayPain(USoundBase* Sound, FVector Location, float Volume, float Cooldown)
{
	const double Now = FPlatformTime::Seconds();
	if (!Sound || !GetOwner() || !GetOwner()->GetRootComponent() || Volume <= 0.f || Now < NextPainTime
		|| (PainVoice.IsValid() && PainVoice->IsPlaying())) return false;
	PainVoice = UGameplayStatics::SpawnSoundAttached(Sound, GetOwner()->GetRootComponent(), NAME_None,
		Location, FRotator::ZeroRotator, EAttachLocation::KeepWorldPosition, true, Volume, 1.f, 0.f, nullptr, nullptr, true);
	if (!PainVoice.IsValid()) return false;
	NextPainTime = Now + FMath::Max(0.f, Cooldown);
	return true;
}

void UEnemyHitAudioComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (PainVoice.IsValid()) PainVoice->Stop();
	Super::EndPlay(Reason);
}
