// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Weapons/_Shared/EquipmentBase/Effects/EquipmentEquipEffectComponent.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"

AEquipmentBase::AEquipmentBase()
{
    // Firearm subclasses may tick; the reveal component ticks only during its effect.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 1. 创建虚拟根节点
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    EquipEffect = CreateDefaultSubobject<UEquipmentEquipEffectComponent>(TEXT("EquipEffect"));

    // 🌟 2a. 创建静态模型并挂载到根节点下
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // FEAT-042：武器投影打开，让地上影子手里有枪（配合 FEAT-038 全身投影 ShadowBodyMesh）
    StaticMesh->CastShadow = false;
    StaticMesh->bCastDynamicShadow = false;

    ShadowStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShadowStaticMesh"));
    ShadowStaticMesh->SetupAttachment(RootComponent);
    ShadowStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShadowStaticMesh->SetHiddenInGame(true);
    ShadowStaticMesh->CastShadow = true;
    ShadowStaticMesh->bCastHiddenShadow = true;

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootComponent);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletalMesh->CastShadow = false;
    SkeletalMesh->bCastDynamicShadow = false;

    EquipmentLight = CreateDefaultSubobject<URectLightComponent>(TEXT("EquipmentLight"));
    EquipmentLight->SetupAttachment(RootComponent);

    // 3. 赋予插槽名称默认值（防呆设计）
    EquipSocketName = TEXT("Grip_Point");
    HolsterSocketName = NAME_None;
}

void AEquipmentBase::BeginPlay()
{
    Super::BeginPlay();
}


static void GetAnimLayerMeshes(AActor* Owner, TArray<USkeletalMeshComponent*>& OutMeshes)
{
    if (AFPSCharacterBase* FPSChar = Cast<AFPSCharacterBase>(Owner))
    {
        // The first-person viewmodel owns weapon layers and montages. CharacterMesh0
        // keeps its uninterrupted full-body locomotion and copies the completed arms
        // upper-body pose in its AnimGraph, so linking the layer twice is unnecessary.
        if (USkeletalMeshComponent* ArmsMesh = FPSChar->GetArmsMesh())
        {
            OutMeshes.AddUnique(ArmsMesh);
        }
        return;
    }
    if (ACharacter* Char = Cast<ACharacter>(Owner))
    {
        if (Char->GetMesh())
        {
            OutMeshes.Add(Char->GetMesh());
        }
    }
}

void AEquipmentBase::Equip(AActor* NewOwner)
{
    SetOwner(NewOwner);

    if (!NewOwner) { return; }

    // Equip runs after Blueprint component overrides and assigned assets are final.
    // Prefer the skeletal presentation when present; retain StaticMesh only as the
    // source asset/transform for the separate world-space shadow proxy below.
    const bool bUseSkeletalPresentation = SkeletalMesh && SkeletalMesh->GetSkeletalMeshAsset();
    if (StaticMesh)
    {
        StaticMesh->SetVisibility(!bUseSkeletalPresentation, true);
    }
    if (SkeletalMesh)
    {
		SkeletalMesh->SetHiddenInGame(!bUseSkeletalPresentation, true);
        SkeletalMesh->SetVisibility(bUseSkeletalPresentation, true);
    }

    if (AFPSCharacterBase* FPSChar = Cast<AFPSCharacterBase>(NewOwner))
    {
        ShadowStaticMesh->SetStaticMesh(StaticMesh->GetStaticMesh());
        for (int32 MaterialIndex = 0; MaterialIndex < StaticMesh->GetNumMaterials(); ++MaterialIndex)
        {
            ShadowStaticMesh->SetMaterial(MaterialIndex, StaticMesh->GetMaterial(MaterialIndex));
        }
        // Attach the shadow-only gun to the same authoritative animated body that now
        // casts the character shadow. Do not attach to the retired duplicate follower.
        ShadowStaticMesh->AttachToComponent(
            FPSChar->GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, EquipSocketName);
        ShadowStaticMesh->SetRelativeTransform(StaticMesh->GetRelativeTransform());

        // The viewmodel mesh is authored in camera space, while CharacterMesh0 is
        // evaluated in world/body space.  Reusing the viewmodel's full relative
        // rotation on the body socket turns the projected rifle sideways even
        // though the player is looking forward. Preserve the authored positional
        // offset and scale, but let the authoritative body socket supply rotation.
        ShadowStaticMesh->SetRelativeRotation(FRotator::ZeroRotator);
    }
    TArray<USkeletalMeshComponent*> AnimMeshes;
    GetAnimLayerMeshes(NewOwner, AnimMeshes);

    for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
    {
        if (!AnimMesh) { continue; }

        if (EquipmentAnimLayerClass)
        {
            if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
            {
                AnimInst->LinkAnimClassLayers(EquipmentAnimLayerClass);
            }
        }
    }

}

void AEquipmentBase::PlayEquipMontage()
{
    AActor* CurrentOwner = GetOwner();
    if (!EquipMontage || !CurrentOwner) { return; }

    // FPS 角色的 FP 手臂和隐藏身体宿主拥有独立 AnimInstance。两边从同一时间点
    // 播放 Equip Montage，ShadowBodyMesh 再通过 Leader Pose 继承身体动作。
    TArray<USkeletalMeshComponent*> AnimMeshes;
    GetAnimLayerMeshes(CurrentOwner, AnimMeshes);
    for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
    {
        if (AnimMesh)
        {
            if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
            {
                AnimInst->Montage_Play(EquipMontage);
            }
        }
    }
}

void AEquipmentBase::PlayEquipEffect()
{
    EquipEffect->Play();
    if (bPlayEquipAnimation) { PlayEquipMontage(); }
}

bool AEquipmentBase::IsEquipEffectPlaying() const
{
    return EquipEffect && EquipEffect->IsPlaying();
}

void AEquipmentBase::Unequip()
{
    EquipEffect->Stop();
    AActor* CurrentOwner = GetOwner();

    if (ShadowStaticMesh)
    {
        ShadowStaticMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        ShadowStaticMesh->SetStaticMesh(nullptr);
    }

    if (CurrentOwner)
    {
        TArray<USkeletalMeshComponent*> AnimMeshes;
        GetAnimLayerMeshes(CurrentOwner, AnimMeshes);
        for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
        {
            if (!AnimMesh) { continue; }
            if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
            {
                // 快速切走时用极短非零 Blend Out 结束旧装备 Montage。
                // 不能使用 0 秒硬停，否则同一 Montage 在实例清理前无法立即重新播放。
                if (EquipMontage && AnimInst->Montage_IsActive(EquipMontage))
                {
                    AnimInst->Montage_Stop(0.01f, EquipMontage);
                }
                if (EquipmentAnimLayerClass)
                {
                    AnimInst->UnlinkAnimClassLayers(EquipmentAnimLayerClass);
                }
            }
        }
    }

    SetOwner(nullptr);
}

#if WITH_DEV_AUTOMATION_TESTS
float AEquipmentBase::GetEquipEffectElapsedForTesting() const { return EquipEffect->GetElapsed(); }
float AEquipmentBase::GetEquipEffectValueForTesting() const { return EquipEffect->GetAmount(); }
#endif
