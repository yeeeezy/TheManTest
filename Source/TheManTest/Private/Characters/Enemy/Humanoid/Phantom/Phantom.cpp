#include "Characters/Enemy/Humanoid/Phantom/Phantom.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CapsuleComponent.h"

void APhantom::SetCombatPhase(int32 NewPhase)
{
	Super::SetCombatPhase(NewPhase);
	SetCloaked(NewPhase >= 2);
}

void APhantom::SetCloaked(bool bEnabled)
{
	bCloaked = bEnabled;
	if (!GetMesh()) return;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,
		bCloaked ? ECR_Ignore : ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1,
		bCloaked ? ECR_Ignore : ECR_Block);

	if (OriginalMaterials.IsEmpty())
	{
		for (int32 Index = 0; Index < GetMesh()->GetNumMaterials(); ++Index)
			OriginalMaterials.Add(GetMesh()->GetMaterial(Index));
	}
	for (int32 Index = 0; Index < GetMesh()->GetNumMaterials(); ++Index)
	{
		if (bCloaked && CloakMaterial)
		{
			GetMesh()->SetMaterial(Index, CloakMaterial);
		}
		else if (!bCloaked && OriginalMaterials.IsValidIndex(Index))
		{
			GetMesh()->SetMaterial(Index, OriginalMaterials[Index]);
		}
		else if (UMaterialInstanceDynamic* MID = GetMesh()->CreateAndSetMaterialInstanceDynamic(Index))
		{
			MID->SetScalarParameterValue(OpacityParameterName, bCloaked ? CloakedOpacity : 1.f);
		}
	}
}
