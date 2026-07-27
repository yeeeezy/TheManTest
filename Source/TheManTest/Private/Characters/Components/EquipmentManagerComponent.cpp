// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Components/EquipmentManagerComponent.h"
#include "Equipment/EquipmentBase/EquipmentBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

UEquipmentManagerComponent::UEquipmentManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UEquipmentManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    // 🌟 留空！组件不再自己初始化了，等待 Owner (Character) 来发号施令
}

void UEquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 角色被销毁（如切换角色）前，先卸下当前装备，回收它授予到持久 ASC（PlayerState）上的技能。
    // 否则技能规格会残留累积：每切一次角色就多一个 GA_Shoot 规格，开火时 HandleGameplayEvent
    // 会激活全部规格 → 同一枪口同帧生成多颗子弹，子弹互相 Block 当即在枪口炸开。
    if (AEquipmentBase* Current = GetCurrentEquipment())
    {
        Current->Unequip();
    }

    for (AEquipmentBase* Equipment : Inventory)
    {
        if (IsValid(Equipment))
        {
            Equipment->Destroy();
        }
    }
    Inventory.Empty();

    Super::EndPlay(EndPlayReason);
}

// 🌟 修改：核心初始化逻辑，现在根据传入的参数 EquipmentClasses 来生成
void UEquipmentManagerComponent::InitializeEquipment(const TArray<TSubclassOf<AEquipmentBase>>& EquipmentClasses)
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    USkeletalMeshComponent* TargetMesh = AttachTargetMesh ? AttachTargetMesh : OwnerCharacter->GetMesh();

    // 1. 遍历传进来的清单，全部生成实体装进背包
    for (TSubclassOf<AEquipmentBase> EquipmentClass : EquipmentClasses)
    {
        if (EquipmentClass)
        {
            FActorSpawnParameters SpawnParams;
            SpawnParams.Owner = OwnerCharacter; 
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            // 在角色当前位置生成
            AEquipmentBase* SpawnedEquipment = GetWorld()->SpawnActor<AEquipmentBase>(EquipmentClass, OwnerCharacter->GetActorTransform(), SpawnParams);

            if (SpawnedEquipment)
            {
                Inventory.Add(SpawnedEquipment);
                
                SpawnedEquipment->SetActorHiddenInGame(true);
                SpawnedEquipment->SetActorEnableCollision(false);
                SpawnedEquipment->SetActorTickEnabled(false);
            }
        }
    }

    // 2. 如果包里有东西，自动装备第 0 把武器
    if (Inventory.Num() > 0)
    {
        CurrentEquipmentIndex = 0;
        if (AEquipmentBase* FirstEquipment = Inventory[CurrentEquipmentIndex])
        {
            FirstEquipment->Equip(OwnerCharacter);
            FirstEquipment->SetActorHiddenInGame(false);
            FirstEquipment->SetActorEnableCollision(true);
            FirstEquipment->SetActorTickEnabled(true);
            
            FirstEquipment->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, FirstEquipment->GetEquipSocketName());
        }
    }
}

AEquipmentBase* UEquipmentManagerComponent::GetCurrentEquipment() const
{
    if (Inventory.IsValidIndex(CurrentEquipmentIndex))
    {
        return Inventory[CurrentEquipmentIndex];
    }
    return nullptr;
}

void UEquipmentManagerComponent::FinalizeUnequippedEquipment(
    AEquipmentBase* Equipment,
    USkeletalMeshComponent* TargetMesh)
{
    if (!Equipment) { return; }

    const FName HolsterName = Equipment->GetHolsterSocketName();
    if (HolsterName != NAME_None && TargetMesh)
    {
        Equipment->AttachToComponent(
            TargetMesh,
            FAttachmentTransformRules::SnapToTargetIncludingScale,
            HolsterName);
    }
    else
    {
        Equipment->SetActorHiddenInGame(true);
    }
}

// 极其纯净的无动画切枪逻辑
void UEquipmentManagerComponent::SwitchEquipment(int32 Direction)
{
    if (Inventory.Num() <= 1) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("切枪失败：当前空手或背包中没有多余的装备！"));
        return;
    }

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    if (bEquipTransitionInProgress)
    {
        return;
    }

    USkeletalMeshComponent* TargetMesh = AttachTargetMesh ? AttachTargetMesh : OwnerCharacter->GetMesh();

    int32 OldEquipmentIndex = CurrentEquipmentIndex;
    int32 NewEquipmentIndex = (CurrentEquipmentIndex + Direction + Inventory.Num()) % Inventory.Num();

    // 目标与当前相同（Direction 为 0 或整除背包数）→ 不做无意义的卸下再装上
    if (NewEquipmentIndex == OldEquipmentIndex)
    {
        return;
    }

    AEquipmentBase* OldEquipment = Inventory[OldEquipmentIndex];
    AEquipmentBase* NewEquipment = Inventory[NewEquipmentIndex];
    const bool bHasEquipMontage = NewEquipment && NewEquipment->GetEquipMontage() != nullptr;
    CurrentEquipmentIndex = NewEquipmentIndex;

    if (OldEquipment)
    {
        // Montage 期间保留旧 Linked Layer 作为稳定底层，避免任何动画图重初始化。
        if (bHasEquipMontage)
        {
            OldEquipment->UnequipWithoutAnimLayer();
        }
        else
        {
            OldEquipment->Unequip();
        }
        OldEquipment->SetActorEnableCollision(false);
        OldEquipment->SetActorTickEnabled(false);
        FinalizeUnequippedEquipment(OldEquipment, TargetMesh);
    }

    if (NewEquipment)
    {
        if (bHasEquipMontage)
        {
            NewEquipment->EquipWithoutAnimLayer(OwnerCharacter);
        }
        else
        {
            NewEquipment->Equip(OwnerCharacter);
        }
        NewEquipment->SetActorHiddenInGame(false);
        NewEquipment->SetActorEnableCollision(true);
        NewEquipment->SetActorTickEnabled(true);
        NewEquipment->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NewEquipment->GetEquipSocketName());

        if (!bHasEquipMontage)
        {
            return;
        }

        // 此时动画图没有发生 Link/Unlink，直接播放 Montage 不会被下一帧初始化清掉。
        // Montage 结束后才原子切换到新武器层的最终 Idle。
        bEquipTransitionInProgress = true;
        const float MontageDuration = NewEquipment->PlayEquipMontage();
        const TWeakObjectPtr<AEquipmentBase> OutgoingEquipment = OldEquipment;
        const TWeakObjectPtr<AEquipmentBase> IncomingEquipment = NewEquipment;
        const TWeakObjectPtr<AActor> AnimOwner = OwnerCharacter;

        const auto CompleteTransition = [this, OutgoingEquipment, IncomingEquipment, AnimOwner]()
        {
            if (IncomingEquipment.IsValid() && AnimOwner.IsValid()
                && GetCurrentEquipment() == IncomingEquipment.Get())
            {
                if (OutgoingEquipment.IsValid())
                {
                    OutgoingEquipment->UnlinkEquipmentAnimLayers(AnimOwner.Get());
                }
                IncomingEquipment->LinkEquipmentAnimLayers(AnimOwner.Get());
            }
            bEquipTransitionInProgress = false;
        };

        if (MontageDuration > 0.f)
        {
            FTimerHandle TransitionTimer;
            GetWorld()->GetTimerManager().SetTimer(
                TransitionTimer,
                FTimerDelegate::CreateWeakLambda(this, CompleteTransition),
                MontageDuration,
                false);
        }
        else
        {
            CompleteTransition();
        }
    }
}
