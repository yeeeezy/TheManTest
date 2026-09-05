#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyHitAudioComponent.generated.h"
class USoundBase;
class UAudioComponent;

/** Per-enemy pain voice state; never stored on a shared static Gameplay Cue. */
UCLASS()
class THEMANTEST_API UEnemyHitAudioComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	bool TryPlayPain(USoundBase* Sound, FVector Location, float Volume, float Cooldown);
	UAudioComponent* GetPainVoice() const { return PainVoice.Get(); }
protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
	double NextPainTime = 0;
	TWeakObjectPtr<UAudioComponent> PainVoice;
};
