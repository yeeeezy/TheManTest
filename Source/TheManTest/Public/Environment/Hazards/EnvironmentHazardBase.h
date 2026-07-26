#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Environment/Hazards/EnvironmentHazardInterface.h"
#include "EnvironmentHazardBase.generated.h"

class UNiagaraComponent;
class USphereComponent;

/**
 * 环境危险区基类。
 *
 * 架构：
 *   HazardZone（触发球）检测实现了 IHazardSuppressor 的 Actor（RepairGun 泡泡等）。
 *   检测到压制者后关闭 Niagara 特效并暂停伤害 tick；
 *   压制者 Actor 销毁时自动恢复。支持同时存在多个压制者（计数清零才恢复）。
 *
 * 子类只需继承并重写 InflictDamage_Implementation 实现各自的伤害逻辑。
 * 扩展新危险区类型（毒气、冰霜等）时继承本类即可，无需修改 RepairGun 相关代码。
 */
UCLASS(Abstract, Blueprintable)
class THEMANTEST_API AEnvironmentHazardBase : public AActor, public IEnvironmentHazard
{
	GENERATED_BODY()

public:
	AEnvironmentHazardBase();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* ===== IEnvironmentHazard ===== */

	// 留空基类实现；子类重写时实现 GAS 伤害逻辑
	virtual void InflictDamage_Implementation(AActor* Target) override;
	virtual bool IsHazardActive_Implementation() override;

	/* ===== 组件 ===== */

	// 触发球：检测 IHazardSuppressor 压制者（碰撞 WorldDynamic Overlap）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard|Components")
	USphereComponent* HazardZone;

	// Niagara 特效（蓝图中配置具体资产）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hazard|Components")
	UNiagaraComponent* HazardEffect;

	/* ===== 配置 ===== */

	// 伤害 tick 间隔（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hazard|Damage")
	float DamageInterval = 0.5f;

protected:
	// HazardZone 检测到 IHazardSuppressor 进入
	UFUNCTION()
	void OnHazardZoneOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	// 压制者销毁回调（绑在 Suppressor->OnDestroyed 上）
	UFUNCTION()
	void OnSuppressorDestroyed(AActor* DestroyedActor);

	void Suppress(AActor* Suppressor);
	void Resume(AActor* Suppressor);

	void StartDamageLoop();
	void StopDamageLoop();

	UFUNCTION()
	void DamageTick();

private:
	bool bIsActive = true;

	FTimerHandle DamageTimerHandle;

	// 当前所有活跃压制者（全部销毁后才恢复）
	TArray<TWeakObjectPtr<AActor>> ActiveSuppressors;
};
