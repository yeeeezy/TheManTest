// Fill out your copyright notice in the Description page of Project Settings.

#include "Equipment/EquipmentBase/EquipmentBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"

AEquipmentBase::AEquipmentBase()
{
    // 默认关闭 Tick 以节省性能，只有在装备激活时才开启
    PrimaryActorTick.bCanEverTick = false;

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

    if (NewOwner && !bDeferAnimLayerLink)
    {
        LinkEquipmentAnimLayers(NewOwner);
    }
}

void AEquipmentBase::EquipWithoutAnimLayer(AActor* NewOwner)
{
    TGuardValue<bool> DeferLinkGuard(bDeferAnimLayerLink, true);
    Equip(NewOwner);
}

void AEquipmentBase::UnequipWithoutAnimLayer()
{
    TGuardValue<bool> DeferUnlinkGuard(bDeferAnimLayerUnlink, true);
    Unequip();
}

void AEquipmentBase::LinkEquipmentAnimLayers(AActor* AnimOwner)
{
    if (!EquipmentAnimLayerClass || !AnimOwner) { return; }

    TArray<USkeletalMeshComponent*> AnimMeshes;
    GetAnimLayerMeshes(AnimOwner, AnimMeshes);

    for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
    {
        if (!AnimMesh) { continue; }

        if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
        {
            AnimInst->LinkAnimClassLayers(EquipmentAnimLayerClass);
        }
    }
}

void AEquipmentBase::UnlinkEquipmentAnimLayers(AActor* AnimOwner)
{
    if (!EquipmentAnimLayerClass || !AnimOwner) { return; }

    TArray<USkeletalMeshComponent*> AnimMeshes;
    GetAnimLayerMeshes(AnimOwner, AnimMeshes);
    for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
    {
        if (AnimMesh)
        {
            if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
            {
                AnimInst->UnlinkAnimClassLayers(EquipmentAnimLayerClass);
            }
        }
    }
}

float AEquipmentBase::PlayEquipMontage()
{
    AActor* CurrentOwner = GetOwner();
    if (!EquipMontage || !CurrentOwner) { return 0.f; }

    // FPS 角色的 FP 手臂和隐藏身体宿主拥有独立 AnimInstance。两边从同一时间点
    // 播放 Equip Montage，ShadowBodyMesh 再通过 Leader Pose 继承身体动作。
    TArray<USkeletalMeshComponent*> AnimMeshes;
    GetAnimLayerMeshes(CurrentOwner, AnimMeshes);
    float MontageDuration = 0.f;
    for (USkeletalMeshComponent* AnimMesh : AnimMeshes)
    {
        if (AnimMesh)
        {
            if (UAnimInstance* AnimInst = AnimMesh->GetAnimInstance())
            {
                MontageDuration = FMath::Max(MontageDuration, AnimInst->Montage_Play(EquipMontage));
            }
        }
    }
    return MontageDuration;
}

void AEquipmentBase::Unequip()
{
    AActor* CurrentOwner = GetOwner();

    if (CurrentOwner && !bDeferAnimLayerUnlink)
    {
        UnlinkEquipmentAnimLayers(CurrentOwner);
    }

    SetOwner(nullptr);
}
