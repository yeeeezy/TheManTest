#include "Weapons/_Shared/GAS/GameplayCues/GCN_ProjectileImpact.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

UGCN_ProjectileImpact::UGCN_ProjectileImpact()
{
	GameplayCueTag = TAG_GameplayCue_Combat_ProjectileImpact;
}

bool UGCN_ProjectileImpact::OnExecute_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	if (ImpactSound)
	{
		if (UWorld* World = Target ? Target->GetWorld() : nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(
				World, ImpactSound, Parameters.Location, VolumeMultiplier, PitchMultiplier);
		}
	}
	return true;
}

UGCN_EnemyHit::UGCN_EnemyHit()
{
	GameplayCueTag = TAG_GameplayCue_Combat_EnemyHit;
}

bool UGCN_EnemyHit::OnExecute_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	return true;
}
