#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Sound/SoundBase.h"
#include "GA_InfiltratorScan.generated.h"

/**
 * UGA_InfiltratorScan
 * Infiltrator 扫描技能。监听 Input.Infiltrator.Scan 事件。
 * 当前为框架占位：打印调试信息确认链路通畅。
 * 后续与 FEAT-015（扫描材质控制系统）对接，调用 TriggerScan(Origin)。
 */
UCLASS()
class THEMANTEST_API UGA_InfiltratorScan : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_InfiltratorScan();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	// 要 Spawn 的全息 UI Actor 蓝图，在 BGA_InfiltratorScan 蓝图中配置
	UPROPERTY(EditDefaultsOnly, Category = "Scan|Hologram")
	TSubclassOf<AActor> HologramActorClass;

	// 距相机的生成距离（cm）
	UPROPERTY(EditDefaultsOnly, Category = "Scan|Hologram")
	float SpawnDistance = 120.f;

	// 触发扫描时播放的音效
	UPROPERTY(EditDefaultsOnly, Category = "Scan|Sound")
	USoundBase* ScanActivateSound = nullptr;

	// 收起扫描时播放的音效
	UPROPERTY(EditDefaultsOnly, Category = "Scan|Sound")
	USoundBase* ScanDeactivateSound = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Scan|Sound")
	float ScanSoundVolume = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Scan|Sound")
	float ScanSoundPitch = 1.f;

private:
	// 当前已生成的全息 Actor（弱引用，Actor 被外部销毁时自动失效）
	TWeakObjectPtr<AActor> SpawnedHologram;
};
