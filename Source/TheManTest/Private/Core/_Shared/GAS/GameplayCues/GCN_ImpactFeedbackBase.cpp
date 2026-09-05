#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

UGCN_ImpactFeedbackBase::UGCN_ImpactFeedbackBase()
{
	static ConstructorHelpers::FObjectFinder<USoundAttenuation> Attenuation(TEXT("/Game/Core/_Shared/Audio/SA_ProjectileImpact"));
	ImpactAttenuation=Attenuation.Object;
}
UAudioComponent* UGCN_ImpactFeedbackBase::SpawnImpactSound(UWorld* World,const FVector& Location,bool bCharacterImpact) const
{
	if(!World||!ImpactSound||!ShouldPlayImpactSound(bCharacterImpact))return nullptr;
	return UGameplayStatics::SpawnSoundAtLocation(World,ImpactSound,Location,FRotator::ZeroRotator,
		VolumeMultiplier, PitchMultiplier,
		0.f,ImpactAttenuation,ImpactConcurrency,true);
}
FRotator UGCN_ImpactFeedbackBase::MakeDecalRotation(const FVector& Normal,bool bRandomize)
{
	const FQuat Alignment=FRotationMatrix::MakeFromX(-Normal.GetSafeNormal(UE_SMALL_NUMBER,FVector::UpVector)).ToQuat();
	return (Alignment*FQuat(FVector::ForwardVector,bRandomize?FMath::FRandRange(0.f,2.f*PI):0.f)).Rotator();
}
void UGCN_ImpactFeedbackBase::RandomizeDecalMaterial(UDecalComponent* Decal)
{
	if(!Decal)return;
	if(auto* MID=Decal->CreateDynamicMaterialInstance())
		MID->SetVectorParameterValue(TEXT("DecalPatternOffset"),FLinearColor(FMath::FRandRange(0.f,100.f),FMath::FRandRange(0.f,100.f),0.f,0.f));
}

bool UGCN_ImpactFeedbackBase::OnExecute_Implementation(
	AActor* Target,
	const FGameplayCueParameters& Parameters) const
{
	UWorld* World = Target ? Target->GetWorld() : nullptr;
	if (!World)
	{
		return false;
	}

	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();
	const bool bCharacterImpact = HitResult && Cast<ACharacter>(HitResult->GetActor()) != nullptr;
	SpawnImpactSound(World,Parameters.Location,bCharacterImpact);
	if (ImpactEffect)
	{
		const FRotator Rotation = Parameters.Normal.IsNearlyZero()
			? FRotator::ZeroRotator
			: Parameters.Normal.Rotation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World, ImpactEffect, Parameters.Location, Rotation, FVector(EffectScale));
	}
	if (ImpactDecalMaterial && !bCharacterImpact)
	{
		const FVector Normal = Parameters.Normal.GetSafeNormal(UE_SMALL_NUMBER, FVector::UpVector);
		const FRotator Rotation = MakeDecalRotation(Normal,bRandomizeDecals);
		const float Variation=bRandomizeDecals?FMath::FRandRange(1.f-DecalSizeVariation,1.f+DecalSizeVariation):1.f;
		const float SurfaceSize = 12.f * DecalSizeMultiplier*Variation;
		if (UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
			World,
			ImpactDecalMaterial,
			FVector(4.f, SurfaceSize, SurfaceSize*(bRandomizeDecals?FMath::FRandRange(.8f,1.2f):1.f)),
			Parameters.Location + Normal,
			Rotation,
			DecalLifeSpan))
		{
			if(bRandomizeDecals)RandomizeDecalMaterial(Decal);
			Decal->SetFadeOut(FMath::Max(0.f, DecalLifeSpan - 1.f), 1.f, false);
		}
	}
	return ImpactSound != nullptr || ImpactEffect != nullptr || ImpactDecalMaterial != nullptr;
}
