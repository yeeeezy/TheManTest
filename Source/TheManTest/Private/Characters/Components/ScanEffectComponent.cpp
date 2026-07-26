#include "Characters/Components/ScanEffectComponent.h"
#include "Characters/Components/HighlightComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/Highlightable.h"

const FName UScanEffectComponent::ParamTimeName   = TEXT("ScanTime");
const FName UScanEffectComponent::ParamOriginName = TEXT("ScanOrigin");
const FName UScanEffectComponent::ParamAlphaName  = TEXT("ScanAlpha");

UScanEffectComponent::UScanEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UScanEffectComponent::TriggerScan(FVector Origin)
{
	if (!ScanMPC) return;
	UWorld* World = GetWorld();
	if (!World) return;

	WorldSpaceOrigin = Origin;
	// Alpha 在 Tick 第一帧写入，确保 ScanOrigin 和 ScanTime 已经正确同步后再显示
	UKismetMaterialLibrary::SetScalarParameterValue(World, ScanMPC, ParamTimeName, 0.f);
	UKismetMaterialLibrary::SetScalarParameterValue(World, ScanMPC, ParamAlphaName, 0.f);

	ScanProgress  = 0.0f;
	bScanning     = true;
	bRetracting   = false;
	AlreadyHighlighted.Empty();
	SetComponentTickEnabled(true);
}

void UScanEffectComponent::RetractScan()
{
	if (!ScanMPC) return;

	bScanning   = false;
	bRetracting = false;
	ScanProgress = 0.f;
	SetComponentTickEnabled(false);
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScanMPC, ParamAlphaName, 0.f);
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScanMPC, ParamTimeName, 0.f);
}

void UScanEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ScanMPC) return;

	// WorldPosition = 像素位置 - 摄像机位置，每帧同步相机相对坐标
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FVector CamPos; FRotator CamRot;
		PC->GetPlayerViewPoint(CamPos, CamRot);
		const FVector Rel = WorldSpaceOrigin - CamPos;
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), ScanMPC, ParamOriginName,
			FLinearColor(Rel.X, Rel.Y, Rel.Z, 0.f));
	}

	const float Step = DeltaTime / FMath::Max(ScanDuration, 0.01f);

	if (bScanning)
	{
		ScanProgress = FMath::Min(ScanProgress + Step, 1.0f);
		const float Eased = 1.f - FMath::Pow(1.f - ScanProgress, 2.f);
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScanMPC, ParamTimeName, Eased);
		UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), ScanMPC, ParamAlphaName, 1.f);

		DetectAndHighlight(MaxScanRadius * Eased);

		// 扩张完成后停住，等待 RetractScan() 调用
		if (ScanProgress >= 1.0f)
		{
			bScanning = false;
			SetComponentTickEnabled(false);
		}
	}
}

void UScanEffectComponent::DetectAndHighlight(float CurrentRadius)
{
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes = {
		UEngineTypes::ConvertToObjectType(ECC_WorldStatic),
		UEngineTypes::ConvertToObjectType(ECC_WorldDynamic),
		UEngineTypes::ConvertToObjectType(ECC_Pawn),
	};

	TArray<AActor*> OverlappedActors;
	UKismetSystemLibrary::SphereOverlapActors(World, WorldSpaceOrigin, CurrentRadius,
		ObjectTypes, nullptr, {}, OverlappedActors);

	for (AActor* Actor : OverlappedActors)
	{
		if (!Actor) continue;

		TWeakObjectPtr<AActor> Weak(Actor);
		if (AlreadyHighlighted.Contains(Weak)) continue;

		if (Actor->Implements<UHighlightable>())
		{
			AlreadyHighlighted.Add(Weak);
			IHighlightable::Execute_StartHighlight(Actor, HighlightDuration);
		}
		else if (UHighlightComponent* HC = Actor->FindComponentByClass<UHighlightComponent>())
		{
			AlreadyHighlighted.Add(Weak);
			IHighlightable::Execute_StartHighlight(HC, HighlightDuration);
		}
	}
}
