#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Characters/CharacterBase/Lobby/LobbyPoseBlendAnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"

ALobbyCharacterBase::ALobbyCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
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
	DisplayMesh->AddTickPrerequisiteActor(this);
	ApplyPresentation();
}

void ALobbyCharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	BlendElapsed += DeltaSeconds;
	const float LinearAlpha = ActiveBlendDuration > 0.f ? FMath::Clamp(BlendElapsed / ActiveBlendDuration, 0.f, 1.f) : 1.f;
	PresentationBlendAlpha = FMath::SmoothStep(0.f, 1.f, LinearAlpha);
	if (auto* Instance = Cast<ULobbyPoseBlendAnimInstance>(DisplayMesh->GetAnimInstance()))
		Instance->SetPoseBlendAlpha(PresentationBlendAlpha);
	FTransform Transform;
	Transform.Blend(StaticBlendStart, StaticBlendTarget, PresentationBlendAlpha);
	DisplayWeapon->SetRelativeTransform(Transform);
	Transform.Blend(SkeletalBlendStart, SkeletalBlendTarget, PresentationBlendAlpha);
	DisplaySkeletalWeapon->SetRelativeTransform(Transform);
	if (LinearAlpha >= 1.f) SetActorTickEnabled(false);
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
	if (DisplayWeaponIndex == NewIndex) return;
	DisplayWeaponIndex = NewIndex;
	ApplyPresentation();
}

void ALobbyCharacterBase::SetRelaxedIdleIndex(int32 NewIndex)
{
	if (!WeaponPresentations.IsValidIndex(DisplayWeaponIndex) ||
		!WeaponPresentations[DisplayWeaponIndex].RelaxedAnimations.IsValidIndex(NewIndex)) return;
	if (DisplayPose == ELobbyCharacterPose::Relaxed && RelaxedIdleIndex == NewIndex) return;
	RelaxedIdleIndex = NewIndex;
	DisplayPose = ELobbyCharacterPose::Relaxed;
	ApplyPresentation();
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
	else if (WeaponPresentations.IsValidIndex(DisplayWeaponIndex))
	{
		const FLobbyWeaponPresentation& Item = WeaponPresentations[DisplayWeaponIndex];
		RelaxedIdleIndex = Item.RelaxedAnimations.IsEmpty() ? 0 : FMath::Clamp(RelaxedIdleIndex, 0, Item.RelaxedAnimations.Num() - 1);
		Animation = DisplayPose == ELobbyCharacterPose::Rifle ? Item.ReadyAnimation.Get() :
			(Item.RelaxedAnimations.IsValidIndex(RelaxedIdleIndex) ? Item.RelaxedAnimations[RelaxedIdleIndex].Get() : nullptr);
	}
	const FTransform PreviousStatic = DisplayWeapon->GetRelativeTransform();
	const FTransform PreviousSkeletal = DisplaySkeletalWeapon->GetRelativeTransform();
	bool bBlending = false;
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		if (!Cast<ULobbyPoseBlendAnimInstance>(DisplayMesh->GetAnimInstance()))
			DisplayMesh->SetAnimInstanceClass(ULobbyPoseBlendAnimInstance::StaticClass());
		if (auto* Instance = Cast<ULobbyPoseBlendAnimInstance>(DisplayMesh->GetAnimInstance()))
			bBlending = Instance->PlayPose(Animation, PoseBlendDuration > 0.f && LastPresentedWeaponIndex == DisplayWeaponIndex);
	}
	else
	{
		DisplayMesh->OverrideAnimationData(Animation, true, true);
		DisplayMesh->PlayAnimation(Animation, true);
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
	LastPresentedWeaponIndex = DisplayWeaponIndex;
	PresentationBlendAlpha = bBlending ? 0.f : 1.f;
	SetActorTickEnabled(bBlending);
	if (bBlending)
	{
		BlendElapsed = 0.f;
		ActiveBlendDuration = PoseBlendDuration;
		StaticBlendStart = PreviousStatic;
		SkeletalBlendStart = PreviousSkeletal;
		StaticBlendTarget = DisplayWeapon->GetRelativeTransform();
		SkeletalBlendTarget = DisplaySkeletalWeapon->GetRelativeTransform();
		DisplayWeapon->SetRelativeTransform(StaticBlendStart);
		DisplaySkeletalWeapon->SetRelativeTransform(SkeletalBlendStart);
	}
}
