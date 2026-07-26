// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquipmentBase.generated.h"

// 前向声明，加快编译速度
class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneComponent;
class URectLightComponent;
class UAnimMontage;
class UAnimInstance;

UCLASS()
class THEMANTEST_API AEquipmentBase : public AActor
{
    GENERATED_BODY()
    
public: 
    // 设置此 Actor 的属性默认值
    AEquipmentBase();

protected:
    // 游戏开始或生成时调用
    virtual void BeginPlay() override;

public: 
    // 每一帧调用
    virtual void Tick(float DeltaTime) override;

    /* ==========================================
     * 🌟 核心生命周期接口 (留空待实现)
     * ========================================== */
public:
    // 装备被拿在手里时调用（赋予GAS技能、开启碰撞、绑定输入等）
    UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
    virtual void Equip(AActor* NewOwner);

    // 装备被收起或丢弃时调用（回收GAS技能、关闭碰撞、解绑输入等）
    UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
    virtual void Unequip();

    // 播放拔枪/装备蒙太奇。与 Equip() 解耦：滚轮切枪时立即调用；
    // 切换角色初次装备时推迟到手臂姿势就绪的下一帧再调用，避免起始位置错乱、音效误触发。
    UFUNCTION(BlueprintCallable, Category = "Equipment|Action")
    void PlayEquipMontage();

    /* ==========================================
     * 🎒 物理表现与组件
     * ========================================== */
protected:
    // 虚拟根组件，用于在蓝图中随意调整 Mesh 的偏移和旋转以对齐手部
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Components")
    USceneComponent* RootSceneComponent;

    // 🌟 新增：静态模型组件 (适用于大剑、平底锅、法杖等没有内部机械结构的武器)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Components")
    UStaticMeshComponent* StaticMesh;

    // 🌟 修正：更名为骨骼模型组件 (适用于枪械、弓箭等需要播放自身动画的武器)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Components")
    USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Components")
    URectLightComponent* EquipmentLight;

    /* ==========================================
     * 🔗 插槽与动画配置
     * ========================================== */
protected:
    // 这件装备拿在手里时，应该挂在角色的哪个插槽？(例如 "Hand_R_Socket")
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Attachment")
    FName EquipSocketName;

    // 装备收起时，应该挂在角色的哪个插槽上？(例如 "Holster_Back" 或留空代表直接隐藏)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Attachment")
    FName HolsterSocketName;

    // 这件装备专属的拔出动画蒙太奇
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Animation")
    UAnimMontage* EquipMontage;

    // 装备时整体替换 ArmsMesh 的基础动画蓝图（为 null 则不替换）
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Animation")
    TSubclassOf<UAnimInstance> EquipmentAnimClass;

    // 这件装备专属的动画链接图层类 (Linked Anim Layer)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment|Animation")
    TSubclassOf<UAnimInstance> EquipmentAnimLayerClass;

public:
    // 对外暴露的 Getter，供 Component 获取数据
    FORCEINLINE UStaticMeshComponent* GetStaticMesh() const { return StaticMesh; }       // 🌟 新增 Getter
    FORCEINLINE USkeletalMeshComponent* GetSkeletalMesh() const { return SkeletalMesh; } // 🌟 修正 Getter
    FORCEINLINE FName GetEquipSocketName() const { return EquipSocketName; }
    FORCEINLINE FName GetHolsterSocketName() const { return HolsterSocketName; }
    FORCEINLINE UAnimMontage* GetEquipMontage() const { return EquipMontage; }
};