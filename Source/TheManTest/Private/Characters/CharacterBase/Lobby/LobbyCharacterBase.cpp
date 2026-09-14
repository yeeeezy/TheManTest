#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterAnimInstance.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Engine/SkeletalMeshSocket.h"

ALobbyCharacterBase::ALobbyCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	DisplayMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DisplayMesh"));
	SetRootComponent(DisplayMesh);
	DisplayMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayMesh->SetGenerateOverlapEvents(false);
	DisplayMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	DisplayWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayWeapon"));
	DisplayWeapon->SetupAttachment(DisplayMesh);
	DisplayWeapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplayWeapon->SetGenerateOverlapEvents(false);
	DisplaySkeletalWeapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DisplaySkeletalWeapon"));
	DisplaySkeletalWeapon->SetupAttachment(DisplayMesh);
	DisplaySkeletalWeapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DisplaySkeletalWeapon->SetGenerateOverlapEvents(false);
}

void ALobbyCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPresentation();
}

void ALobbyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyPresentation();
}

void ALobbyCharacterBase::SetDisplayPose(ELobbyCharacterPose NewPose)
{
	if (DisplayPose == NewPose) return;
	DisplayPose = NewPose;
	ApplyPresentation();
}

void ALobbyCharacterBase::SetStandingIdleIndex(int32 NewIndex)
{
	if (!StandingAnimations.IsValidIndex(NewIndex)) return;
	if (StandingIdleIndex == NewIndex && DisplayPose == ELobbyCharacterPose::Standing) return;
	StandingIdleIndex = NewIndex;
	DisplayPose = ELobbyCharacterPose::Standing;
	ApplyPresentation();
}

void ALobbyCharacterBase::SetWeaponReady(bool bReady)
{
	SetDisplayPose(bReady ? ELobbyCharacterPose::Rifle : ELobbyCharacterPose::Relaxed);
}

void ALobbyCharacterBase::SetDisplayWeaponIndex(int32 NewIndex)
{
	if (!WeaponPresentations.IsValidIndex(NewIndex)) return;
	DisplayWeaponIndex = NewIndex;
	ApplyPresentation();
}

bool ALobbyCharacterBase::GetLeftHandTarget(FTransform& OutTarget) const
{
	if (DisplayPose == ELobbyCharacterPose::Standing || !WeaponPresentations.IsValidIndex(DisplayWeaponIndex)) return false;
	if (DisplayPose == ELobbyCharacterPose::Relaxed && !bShowWeaponWhenRelaxed) return false;
	const FLobbyWeaponPresentation& Item = WeaponPresentations[DisplayWeaponIndex];
	if (!Item.WeaponClass) return false;
	const bool bReady = DisplayPose == ELobbyCharacterPose::Rifle;
	OutTarget = (bReady ? Item.ReadyLeftGrip : Item.RelaxedLeftGrip) * (bReady ? Item.ReadyAttachment : Item.RelaxedAttachment);
	if (const USkeletalMeshSocket* Socket = DisplayMesh->GetSocketByName(WeaponAttachSocket))
		OutTarget = OutTarget * Socket->GetSocketLocalTransform();
	return true;
}

void ALobbyCharacterBase::ApplyPresentation()
{
	if (!WeaponPresentations.IsEmpty()) DisplayWeaponIndex = FMath::Clamp(DisplayWeaponIndex, 0, WeaponPresentations.Num() - 1);
	UAnimSequence* Animation = DisplayPose == ELobbyCharacterPose::Rifle ? RifleAnimation : RelaxedAnimation;
	if (DisplayPose == ELobbyCharacterPose::Standing)
	{
		StandingIdleIndex = StandingAnimations.IsEmpty() ? 0 : FMath::Clamp(StandingIdleIndex, 0, StandingAnimations.Num() - 1);
		Animation = StandingAnimations.IsValidIndex(StandingIdleIndex) ? StandingAnimations[StandingIdleIndex].Get() : nullptr;
	}
	DisplayMesh->SetAnimInstanceClass(ULobbyCharacterAnimInstance::StaticClass());
	if (UAnimSingleNodeInstance* Instance = DisplayMesh->GetSingleNodeInstance())
	{
		Instance->SetAnimationAsset(Animation, true, 1.f);
		Instance->SetPlaying(true);
		Instance->SetPosition(0.f, false);
	}
	if (DisplayMesh->IsRegistered())
	{
		DisplayMesh->TickAnimation(0.f, false);
		DisplayMesh->RefreshBoneTransforms();
	}
	const bool bValidSocket = !WeaponAttachSocket.IsNone() && DisplayMesh->DoesSocketExist(WeaponAttachSocket);
	DisplayWeapon->AttachToComponent(DisplayMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
	DisplayWeapon->SetRelativeTransform(DisplayPose == ELobbyCharacterPose::Rifle ? RifleWeaponRelativeTransform : RelaxedWeaponRelativeTransform);
	DisplayWeapon->SetVisibility(bValidSocket && DisplayWeapon->GetStaticMesh() &&
		(DisplayPose == ELobbyCharacterPose::Rifle || (DisplayPose == ELobbyCharacterPose::Relaxed && bShowWeaponWhenRelaxed)));
	DisplaySkeletalWeapon->SetVisibility(false);
	if (WeaponPresentations.IsValidIndex(DisplayWeaponIndex))
	{
		const FLobbyWeaponPresentation& Item = WeaponPresentations[DisplayWeaponIndex];
		const AEquipmentBase* Source = Item.WeaponClass ? Item.WeaponClass->GetDefaultObject<AEquipmentBase>() : nullptr;
		const USkeletalMeshComponent* SkeletalSource = Source ? Source->GetSkeletalMesh() : nullptr;
		const UStaticMeshComponent* StaticSource = Source ? Source->GetStaticMesh() : nullptr;
		const bool bSkeletal = SkeletalSource && SkeletalSource->GetSkeletalMeshAsset();
		const bool bVisible = bValidSocket && (DisplayPose == ELobbyCharacterPose::Rifle || (DisplayPose == ELobbyCharacterPose::Relaxed && bShowWeaponWhenRelaxed));
		const FTransform Mount = DisplayPose == ELobbyCharacterPose::Rifle ? Item.ReadyAttachment : Item.RelaxedAttachment;
		DisplayWeapon->SetVisibility(false);
		if (bSkeletal)
		{
			DisplaySkeletalWeapon->SetSkeletalMesh(SkeletalSource->GetSkeletalMeshAsset());
			DisplaySkeletalWeapon->OverrideMaterials.Empty();
			for (int32 Index = 0; Index < SkeletalSource->GetNumMaterials(); ++Index)
				DisplaySkeletalWeapon->SetMaterial(Index, SkeletalSource->GetMaterial(Index));
			DisplaySkeletalWeapon->AttachToComponent(DisplayMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocket);
			DisplaySkeletalWeapon->SetRelativeTransform(SkeletalSource->GetRelativeTransform() * Mount);
			DisplaySkeletalWeapon->SetVisibility(bVisible);
		}
		else if (StaticSource && StaticSource->GetStaticMesh())
		{
			DisplayWeapon->SetStaticMesh(StaticSource->GetStaticMesh());
			DisplayWeapon->OverrideMaterials.Empty();
			for (int32 Index = 0; Index < StaticSource->GetNumMaterials(); ++Index)
				DisplayWeapon->SetMaterial(Index, StaticSource->GetMaterial(Index));
			DisplayWeapon->SetRelativeTransform(StaticSource->GetRelativeTransform() * Mount);
			DisplayWeapon->SetVisibility(bVisible);
		}
	}
}
