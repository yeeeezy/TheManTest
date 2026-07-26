#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/Highlightable.h"
#include "InteractableBase.generated.h"

class UHighlightComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

UCLASS()
class THEMANTEST_API AInteractableBase : public AActor, public IHighlightable
{
	GENERATED_BODY()

public:
	AInteractableBase();

	virtual void StartHighlight_Implementation(float Duration) override;
	virtual void StopHighlight_Implementation() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Highlight")
	UHighlightComponent* HighlightComponent;
};
