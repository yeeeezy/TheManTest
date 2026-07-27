// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EquipmentManagerComponent.generated.h"

// 前向声明
class AEquipmentBase;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class THEMANTEST_API UEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public: 
	UEquipmentManagerComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public: 
	/* ==========================================
	 * 🌟 核心对外接口
	 * ========================================== */

	// 🌟 新增/修改：接收外部（比如角色基类）传来的武器清单进行初始化
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void InitializeEquipment(const TArray<TSubclassOf<AEquipmentBase>>& EquipmentClasses);

	// 供滚轮输入调用：切换装备 (传入 1 往下滚，-1 往上滚)
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SwitchEquipment(int32 Direction);

	// 获取当前手里拿着的装备
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	AEquipmentBase* GetCurrentEquipment() const;

	// 武器挂载目标 Mesh，由 Character 在 BeginPlay 中赋值
	UPROPERTY()
	USkeletalMeshComponent* AttachTargetMesh = nullptr;

	/* ==========================================
	 * 🎒 内部数据资产
	 * ========================================== */
protected:
	void FinalizeUnequippedEquipment(AEquipmentBase* Equipment, USkeletalMeshComponent* TargetMesh);

	// 真正的背包数组！存放已经实例化的装备实体
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|Inventory")
	TArray<AEquipmentBase*> Inventory;

	// 当前装备在数组中的下标
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment|State")
	int32 CurrentEquipmentIndex = 0;

	// 有 Montage 的切枪期间继续留在手里的旧装备；新姿势就绪后与新装备原子交换。
	TWeakObjectPtr<AEquipmentBase> PendingVisibleEquipment;
};
