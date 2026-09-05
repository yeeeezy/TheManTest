#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "TimerManager.h"

void APhantom::BeginPlay()
{
	Super::BeginPlay();
	if (!bStationaryHitTest) return;

	// Remove perception, focus and behavior-tree execution for this test target only.
	if (AController* AI = GetController())
	{
		AI->UnPossess();
		AI->Destroy();
	}
	GetWorldTimerManager().ClearAllTimersForObject(this);
	SetActorTickEnabled(false);
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bIsAiming = false;
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	// Mesh and hit-reaction component keep ticking so directional feedback remains visible.
}

void APhantom::ReactToProjectileHit(AActor* HitInstigator)
{
	if (!bStationaryHitTest) Super::ReactToProjectileHit(HitInstigator);
}

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
