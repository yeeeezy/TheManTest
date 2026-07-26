#include "Characters/Components/HighlightComponent.h"
#include "Components/MeshComponent.h"

UHighlightComponent::UHighlightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHighlightComponent::StartHighlight_Implementation(float Duration)
{
	SetCustomDepthOnMeshes(true);

	if (Duration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(FadeTimerHandle, this,
			&UHighlightComponent::StopHighlight_Implementation, Duration, false);
	}
}

void UHighlightComponent::StopHighlight_Implementation()
{
	SetCustomDepthOnMeshes(false);
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FadeTimerHandle);
	}
}

void UHighlightComponent::SetCustomDepthOnMeshes(bool bEnable)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	TArray<UMeshComponent*> Meshes;
	Owner->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		Mesh->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			Mesh->SetCustomDepthStencilValue(1);
		}
	}
}
