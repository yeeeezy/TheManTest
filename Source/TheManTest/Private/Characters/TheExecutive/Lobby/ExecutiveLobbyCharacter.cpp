#include "Characters/TheExecutive/Lobby/ExecutiveLobbyCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"

AExecutiveLobbyCharacter::AExecutiveLobbyCharacter()
{
	FaceGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("FaceGlow"));
	ChestGlow = CreateDefaultSubobject<UPointLightComponent>(TEXT("ChestGlow"));
	FaceGlow->SetupAttachment(DisplayMesh, TEXT("Bip01-Head"));
	ChestGlow->SetupAttachment(DisplayMesh, TEXT("Bip01-Spine4"));
	for (UPointLightComponent* Light : {FaceGlow.Get(), ChestGlow.Get()})
	{
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensityUnits(ELightUnits::Lumens);
		Light->SetLightColor(FLinearColor(1.f, .002f, .001f));
		Light->SetCastShadows(false);
		Light->SetLightingChannels(false, true, false);
		Light->SetIndirectLightingIntensity(0.f);
		Light->SetSpecularScale(.15f);
		Light->SetSourceRadius(3.f);
	}
	FaceGlow->SetIntensity(6.f);
	FaceGlow->SetAttenuationRadius(17.f);
	ChestGlow->SetIntensity(1.5f);
	ChestGlow->SetAttenuationRadius(14.f);
}

void AExecutiveLobbyCharacter::SetActorHiddenInGame(bool bNewHidden)
{
	Super::SetActorHiddenInGame(bNewHidden);
	FaceGlow->SetVisibility(!bNewHidden);
	ChestGlow->SetVisibility(!bNewHidden);
}
