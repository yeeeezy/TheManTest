#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
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
	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	const bool bCharacterImpact = HitResult && Cast<ACharacter>(HitResult->GetActor()) != nullptr;
	UNiagaraSystem* SelectedImpactEffect = bCharacterImpact && CharacterImpactEffect
		? CharacterImpactEffect.Get()
		: ImpactEffect.Get();
	if (SelectedImpactEffect)
	{
		const FRotator Rotation = Parameters.Normal.IsNearlyZero()
			? FRotator::ZeroRotator
			: Parameters.Normal.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, SelectedImpactEffect, Parameters.Location, Rotation, FVector(EffectScale));
	}
	if (ImpactDecalMaterial && (!bCharacterImpact || bSpawnDecalOnCharacters))
	{
		const FVector Normal = Parameters.Normal.GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
		const FRotator Rotation = FRotationMatrix::MakeFromX(-Normal).Rotator();
		const float SurfaceSize = 12.f * DecalSizeMultiplier;
		if (UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
			World,
			ImpactDecalMaterial,
			FVector(4.f, SurfaceSize, SurfaceSize),
			Parameters.Location + Normal,
			Rotation,
			DecalLifeSpan))
		{
			Decal->SetFadeOut(FMath::Max(0.f, DecalLifeSpan - 1.f), 1.f, false);
		}
	}
	return ImpactSound != nullptr || ImpactEffect != nullptr || CharacterImpactEffect != nullptr
		|| ImpactDecalMaterial != nullptr;
}
