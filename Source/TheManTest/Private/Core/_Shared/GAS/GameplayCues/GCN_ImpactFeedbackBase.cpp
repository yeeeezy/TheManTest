#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

bool UGCN_ImpactFeedbackBase::OnExecute_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	UWorld* World = Target ? Target->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			World, ImpactSound, Parameters.Location, VolumeMultiplier, PitchMultiplier);
	}
	if (ImpactEffect)
	{
		const FRotator Rotation = Parameters.Normal.IsNearlyZero()
			? FRotator::ZeroRotator
			: Parameters.Normal.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, ImpactEffect, Parameters.Location, Rotation, FVector(EffectScale));
	}
	return ImpactSound != nullptr || ImpactEffect != nullptr;
}
