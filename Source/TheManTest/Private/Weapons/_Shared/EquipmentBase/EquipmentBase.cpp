// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"

namespace
{
    const FName EquipDissolveParameter(TEXT("Amount (S)"));
    constexpr float EquipDissolveDuration = 0.45f;
    constexpr float EquipDissolveHiddenValue = 1.f;
    constexpr float EquipDissolveVisibleValue = -1.f;
}

AEquipmentBase::AEquipmentBase()
{
    // Equip dissolve uses Tick only while this equipment is active; inventory items
    // remain disabled by EquipmentManager until equipped.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 1. 创建虚拟根节点
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // 🌟 2a. 创建静态模型并挂载到根节点下
    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(RootComponent);
    StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    // FEAT-042：武器投影打开，让地上影子手里有枪（配合 FEAT-038 全身投影 ShadowBodyMesh）
    StaticMesh->CastShadow = true;
    StaticMesh->bCastDynamicShadow = true;

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootComponent);
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkeletalMesh->CastShadow = true;
    SkeletalMesh->bCastDynamicShadow = true;

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

void AEquipmentBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEquipEffectActive) { return; }

    EquipEffectElapsed = FMath::Min(EquipEffectElapsed + DeltaTime, EquipDissolveDuration);
    const float Alpha = EquipDissolveDuration > 0.f ? EquipEffectElapsed / EquipDissolveDuration : 1.f;
    const float DissolveAmount = FMath::Lerp(
        EquipDissolveHiddenValue,
        EquipDissolveVisibleValue,
        FMath::SmoothStep(0.f, 1.f, Alpha));
    for (UMaterialInstanceDynamic* Material : EquipEffectMaterials)
    {
        if (Material)
        {
            Material->SetScalarParameterValue(EquipDissolveParameter, DissolveAmount);
        }
    }
    bEquipEffectActive = Alpha < 1.f;
}

static void GetAnimLayerMeshes(AActor* Owner, TArray<USkeletalMeshComponent*>& OutMeshes)
{
    if (AFPSCharacterBase* FPSChar = Cast<AFPSCharacterBase>(Owner))
    {
        if (FPSChar->GetArmsMesh())
        {
            OutMeshes.Add(FPSChar->GetArmsMesh());
        }
        if (FPSChar->GetMesh())
        {
            OutMeshes.AddUnique(FPSChar->GetMesh());
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
    EquipEffectMaterials.Reset();

    TArray<UMeshComponent*> MeshComponents;
    GetComponents<UMeshComponent>(MeshComponents);
    for (UMeshComponent* MeshComponent : MeshComponents)
    {
        if (!MeshComponent) { continue; }

        for (int32 MaterialIndex = 0; MaterialIndex < MeshComponent->GetNumMaterials(); ++MaterialIndex)
        {
            if (UMaterialInstanceDynamic* Material = MeshComponent->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
            {
                Material->SetScalarParameterValue(EquipDissolveParameter, EquipDissolveHiddenValue);
                EquipEffectMaterials.Add(Material);
            }
        }
    }

    EquipEffectElapsed = 0.f;
    bEquipEffectActive = EquipEffectMaterials.Num() > 0;
}

void AEquipmentBase::Unequip()
{
    AActor* CurrentOwner = GetOwner();

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
