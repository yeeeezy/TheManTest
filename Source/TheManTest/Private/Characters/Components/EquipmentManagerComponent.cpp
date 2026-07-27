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

    USkeletalMeshComponent* TargetMesh = AttachTargetMesh ? AttachTargetMesh : OwnerCharacter->GetMesh();

    // 快速连续切枪可能发生在上一次原子交换完成前；先收好仍留在手里的旧装备。
    if (PendingVisibleEquipment.IsValid())
    {
        FinalizeUnequippedEquipment(PendingVisibleEquipment.Get(), TargetMesh);
        PendingVisibleEquipment.Reset();
    }

    int32 OldEquipmentIndex = CurrentEquipmentIndex;
    int32 NewEquipmentIndex = (CurrentEquipmentIndex + Direction + Inventory.Num()) % Inventory.Num();

    // 目标与当前相同（Direction 为 0 或整除背包数）→ 不做无意义的卸下再装上
    if (NewEquipmentIndex == OldEquipmentIndex)
    {
        return;
    }

    AEquipmentBase* OldEquipment = Inventory[OldEquipmentIndex];
    AEquipmentBase* NewEquipment = Inventory[NewEquipmentIndex];
    const bool bDelayVisibilitySwap = NewEquipment && NewEquipment->GetEquipMontage() != nullptr;
    CurrentEquipmentIndex = NewEquipmentIndex;

    if (OldEquipment)
    {
        OldEquipment->Unequip(); 
        OldEquipment->SetActorEnableCollision(false);
        OldEquipment->SetActorTickEnabled(false);

        if (bDelayVisibilitySwap)
        {
            // 新武器起始姿势尚未就绪，旧武器继续留在手里，避免出现一帧空手。
            PendingVisibleEquipment = OldEquipment;
        }
        else
        {
            FinalizeUnequippedEquipment(OldEquipment, TargetMesh);
        }
    }

    if (NewEquipment)
    {
        NewEquipment->Equip(OwnerCharacter); 
        // 有拔枪 Montage 时先保持隐藏：Linked Layer 初始化完成并评估到动画首帧后再显示，
        // 避免先露出持枪 Idle，再从上方向下混到拔枪起始姿势。
        const bool bHasEquipMontage = NewEquipment->GetEquipMontage() != nullptr;
        NewEquipment->SetActorHiddenInGame(bHasEquipMontage);
        NewEquipment->SetActorEnableCollision(true);
        NewEquipment->SetActorTickEnabled(true);
        
        NewEquipment->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NewEquipment->GetEquipSocketName());

        if (!bHasEquipMontage)
        {
            return;
        }

        // LinkAnimClassLayers 会在下一次动画更新时完成初始化；同帧播放 Montage 会被
        // 该初始化清掉。延迟一帧，并确认快速滚轮后它仍是当前装备再播放。
        const TWeakObjectPtr<AEquipmentBase> EquipmentToPlay = NewEquipment;
        const TWeakObjectPtr<AEquipmentBase> OldEquipmentToFinalize = OldEquipment;
        const TWeakObjectPtr<USkeletalMeshComponent> SwapTargetMesh = TargetMesh;
        GetWorld()->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateWeakLambda(this, [this, EquipmentToPlay, OldEquipmentToFinalize, SwapTargetMesh]()
            {
                if (EquipmentToPlay.IsValid() && GetCurrentEquipment() == EquipmentToPlay.Get())
                {
                    EquipmentToPlay->PlayEquipMontage();

					// Timer 回调发生在本帧动画更新之后；再等一帧，让 Montage 的零混合
					// 起始姿势先写入手臂/身体骨骼，然后才显示武器。
					if (UWorld* World = GetWorld())
					{
						const TWeakObjectPtr<AEquipmentBase> EquipmentToReveal = EquipmentToPlay;
						World->GetTimerManager().SetTimerForNextTick(
							FTimerDelegate::CreateWeakLambda(this, [this, EquipmentToReveal, OldEquipmentToFinalize, SwapTargetMesh]()
							{
								if (EquipmentToReveal.IsValid() && GetCurrentEquipment() == EquipmentToReveal.Get())
								{
									FinalizeUnequippedEquipment(OldEquipmentToFinalize.Get(), SwapTargetMesh.Get());
									EquipmentToReveal->SetActorHiddenInGame(false);
									if (PendingVisibleEquipment == OldEquipmentToFinalize)
									{
										PendingVisibleEquipment.Reset();
									}
								}
							}));
					}
                }
            }));
    }
}
