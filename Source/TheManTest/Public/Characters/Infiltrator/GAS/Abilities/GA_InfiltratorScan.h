#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Sound/SoundBase.h"
#include "GA_InfiltratorScan.generated.h"

/**
 * UGA_InfiltratorScan
 * 玩家扫描技能。监听 Input.Character.Interact 事件并切换扫描效果。
 * 全息 UI 为可选展示，不参与扫描状态或材质触发。
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
	// Scan state is independent from the optional hologram UI. The hologram asset
	// may be absent while the MPC wave and terrain overlay still toggle normally.
	bool bScanActive = false;

	// 当前已生成的全息 Actor（弱引用，Actor 被外部销毁时自动失效）
	TWeakObjectPtr<AActor> SpawnedHologram;
};
