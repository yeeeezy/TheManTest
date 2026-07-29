#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ScanEffectComponent.generated.h"

class UMaterialParameterCollection;
class UMaterialInterface;
class UDecalComponent;

/**
 * UScanEffectComponent
 * 挂到角色上，负责驱动 MPC_ScanEffect 实现单次球形扫描波。
 * TriggerScan(Origin) 从蓝图或 GA_InfiltratorScan 调用。
 * 扫描波扩张时自动检测范围内实现 IHighlightable 的 Actor 并触发高亮。
 */
UCLASS(ClassGroup = "TheManTest", meta = (BlueprintSpawnableComponent))
class THEMANTEST_API UScanEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UScanEffectComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// 从指定世界坐标触发一次扫描波（向外扩张）
	UFUNCTION(BlueprintCallable, Category = "Scan")
	void TriggerScan(FVector Origin);

	// 触发回缩（从当前位置向内收回）
	UFUNCTION(BlueprintCallable, Category = "Scan")
	void RetractScan();

	// MPC_ScanEffect 资产，在角色蓝图中赋值
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan")
	UMaterialParameterCollection* ScanMPC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan|Terrain Overlay")
	UMaterialInterface* TerrainOverlayMaterial = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scan|Terrain Overlay", meta = (ClampMin = "100.0"))
	float TerrainOverlayDepth = 5000.f;

	// 扫描波从 0 扩张到最大的总时长（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scan")
	float ScanDuration = 2.0f;

	// 扫描波最大世界空间半径（cm），需与材质 SphereMask Radius 的 Multiply 系数一致
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scan")
	float MaxScanRadius = 3000.f;

	// 高亮持续时间（秒），传给 IHighlightable::StartHighlight；< 0 = 永不消失
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Scan")
	float HighlightDuration = 10.f;

private:
	bool    bScanning     = false;
	bool    bRetracting   = false;
	float   ScanProgress  = 0.0f;
	FVector WorldSpaceOrigin = FVector::ZeroVector;

	UPROPERTY(Transient)
	UDecalComponent* TerrainOverlayDecal = nullptr;

	// 本次扫描已触发高亮的 Actor，防止重复触发
	TArray<TWeakObjectPtr<AActor>> AlreadyHighlighted;

	void DetectAndHighlight(float CurrentRadius);

	// MPC 参数名（与 MPC_ScanEffect 内命名一致）
	static const FName ParamTimeName;
	static const FName ParamOriginName;
	static const FName ParamAlphaName;
};
