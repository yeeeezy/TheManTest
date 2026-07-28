// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Components/EquipmentManagerComponent.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "Characters/FPSCharacterBase/Animation/FPSCharacterAnimInstance.h"
#include "Equipment/EquipmentBase/EquipmentBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

namespace
{
    void ForEachWeaponTransitionAnimInstance(ACharacter* OwnerCharacter, TFunctionRef<void(UFPSCharacterAnimInstance*)> Callback)
    {
        AFPSCharacterBase* FPSCharacter = Cast<AFPSCharacterBase>(OwnerCharacter);
        if (!FPSCharacter) { return; }

        USkeletalMeshComponent* Meshes[] = { FPSCharacter->GetArmsMesh(), FPSCharacter->GetMesh() };
        for (USkeletalMeshComponent* Mesh : Meshes)
        {
            if (Mesh)
            {
                if (UFPSCharacterAnimInstance* AnimInstance = Cast<UFPSCharacterAnimInstance>(Mesh->GetAnimInstance()))
                {
                    Callback(AnimInstance);
                }
            }
        }
    }
}

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
    if (bVisualSwapPending)
    {
        return;
    }

    if (Inventory.Num() <= 1) 
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("切枪失败：当前空手或背包中没有多余的装备！"));
        return;
    }

    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

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
    const bool bNeedsVisualTransition = NewEquipment && NewEquipment->GetEquipMontage();

    if (bNeedsVisualTransition)
    {
        // 必须在旧 Linked Layer 被移除前保存最终输出姿势。
        ForEachWeaponTransitionAnimInstance(OwnerCharacter, [](UFPSCharacterAnimInstance* AnimInstance)
        {
            AnimInstance->CaptureWeaponTransitionPose();
        });
    }

    CurrentEquipmentIndex = NewEquipmentIndex;

    if (OldEquipment)
    {
        OldEquipment->Unequip();
        OldEquipment->SetActorEnableCollision(false);
        OldEquipment->SetActorTickEnabled(false);
        if (!bNeedsVisualTransition)
        {
            FinalizeUnequippedEquipment(OldEquipment, TargetMesh);
        }
    }

    if (NewEquipment)
    {
        // 先完整 Equip/链接动画层和技能生命周期；视觉层在下方的
        // 单帧过渡中等待新姿势首次求值，与 gameplay 所有权切换解耦。
        NewEquipment->Equip(OwnerCharacter);
        // 有 Montage 时，新武器不能在新动画层首次求值前显示，
        // 否则会短暂附着在旧武器骨骼姿势上并产生数十厘米的跳变。
        NewEquipment->SetActorHiddenInGame(bNeedsVisualTransition);
        NewEquipment->SetActorEnableCollision(true);
        NewEquipment->SetActorTickEnabled(true);
        NewEquipment->AttachToComponent(TargetMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, NewEquipment->GetEquipSocketName());

        // Linked Layer 初始化稳定后，把 Equip Montage 固定在 0 秒；主 AnimBP
        // 从保存的旧姿势直接桥接到该低位起点，随后只执行向上的 Equip 动作。
        if (bNeedsVisualTransition)
        {
            bVisualSwapPending = true;
            const TWeakObjectPtr<AEquipmentBase> EquipmentToFinalize = OldEquipment;
            const TWeakObjectPtr<AEquipmentBase> EquipmentToPlay = NewEquipment;
            GetWorld()->GetTimerManager().SetTimerForNextTick(
                FTimerDelegate::CreateWeakLambda(this, [this, EquipmentToFinalize, EquipmentToPlay, TargetMesh]()
                {
                    if (EquipmentToPlay.IsValid() && GetCurrentEquipment() == EquipmentToPlay.Get())
                    {
                        UAnimMontage* EquipMontage = EquipmentToPlay->GetEquipMontage();
                        ForEachWeaponTransitionAnimInstance(Cast<ACharacter>(GetOwner()), [EquipMontage](UFPSCharacterAnimInstance* AnimInstance)
                        {
                            AnimInstance->StartWeaponTransition(EquipMontage);
                        });

                        // Alpha=0 时新枪与旧枪使用完全相同的已保存骨骼姿势，原子换枪不会空间跳变。
                        FinalizeUnequippedEquipment(EquipmentToFinalize.Get(), TargetMesh);
                        EquipmentToPlay->SetActorHiddenInGame(false);

						// 新旧第一人称枪体在同一像素区域原子替换时，TAA/TSR 仍可能保留
						// 上一枪的颜色历史。标记一次无位移 Camera Cut，只清空该帧时域历史，
						// 不改变相机 Transform，也不会给 gameplay 引入额外阶段。
						ACharacter* TransitionOwner = Cast<ACharacter>(GetOwner());
						if (APlayerController* PlayerController = TransitionOwner ? Cast<APlayerController>(TransitionOwner->GetController()) : nullptr)
						{
							if (PlayerController->PlayerCameraManager)
							{
								PlayerController->PlayerCameraManager->SetGameCameraCutThisFrame();
							}
						}

                        // 这里只负责释放输入锁。Pose Alpha 的完成与 Montage 恢复由
                        // AnimInstance 自己在动画更新中处理，不能依赖世界 Timer 强制跳到 1。
                        FTimerHandle TransitionTimer;
                        GetWorld()->GetTimerManager().SetTimer(
                            TransitionTimer,
                            FTimerDelegate::CreateWeakLambda(this, [this]()
                            {
                                bVisualSwapPending = false;
                            }),
                            0.10f,
                            false);
                        return;
                    }

                    ForEachWeaponTransitionAnimInstance(Cast<ACharacter>(GetOwner()), [](UFPSCharacterAnimInstance* AnimInstance)
                    {
                        AnimInstance->CompleteWeaponTransition();
                    });
                    bVisualSwapPending = false;
                }));
        }
    }
}
