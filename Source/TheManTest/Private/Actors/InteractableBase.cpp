#include "Actors/InteractableBase.h"
#include "Characters/_Shared/Components/HighlightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

AInteractableBase::AInteractableBase()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	RootComponent = StaticMesh;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetupAttachment(StaticMesh);

	HighlightComponent = CreateDefaultSubobject<UHighlightComponent>(TEXT("HighlightComponent"));
}

void AInteractableBase::StartHighlight_Implementation(float Duration)
{
	if (HighlightComponent)
	{
		IHighlightable::Execute_StartHighlight(HighlightComponent, Duration);
	}
}

void AInteractableBase::StopHighlight_Implementation()
{
	if (HighlightComponent)
	{
		IHighlightable::Execute_StopHighlight(HighlightComponent);
	}
}
