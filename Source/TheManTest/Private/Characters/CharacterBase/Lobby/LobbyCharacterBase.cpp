#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

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

void ALobbyCharacterBase::ApplyPresentation()
{
	UAnimSequence* Animation = DisplayPose == ELobbyCharacterPose::Rifle ? RifleAnimation : RelaxedAnimation;
	DisplayMesh->OverrideAnimationData(Animation, true, true);
	DisplayMesh->PlayAnimation(Animation, true);
	if (UAnimSingleNodeInstance* Instance = DisplayMesh->GetSingleNodeInstance())
	{
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
		(DisplayPose == ELobbyCharacterPose::Rifle || bShowWeaponWhenRelaxed));
}
