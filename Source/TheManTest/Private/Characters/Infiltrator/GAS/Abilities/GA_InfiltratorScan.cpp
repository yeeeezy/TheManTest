#include "Characters/Infiltrator/GAS/Abilities/GA_InfiltratorScan.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "Characters/Components/ScanEffectComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"

UGA_InfiltratorScan::UGA_InfiltratorScan()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag    = TAG_Input_Character_Interact;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_InfiltratorScan::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AFPSCharacterBase* Character = Cast<AFPSCharacterBase>(GetAvatarActorFromActorInfo());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Scan toggling is authoritative. The hologram is an optional presentation
	// layer and must never gate the MPC wave or terrain overlay.
	if (bScanActive)
	{
		bScanActive = false;
		if (SpawnedHologram.IsValid())
		{
			AActor* Hologram = SpawnedHologram.Get();
			SpawnedHologram = nullptr;

			if (UFunction* HideFunc = Hologram->FindFunction(FName("Hide")))
			{
				Hologram->ProcessEvent(HideFunc, nullptr);
			}
			// Hide 动画约 1.5s（与蓝图 Delay 一致），播完后自动销毁
			Hologram->SetLifeSpan(2.5f);
		}

		// 扫描波从当前位置向内回缩
		if (UScanEffectComponent* ScanComp = Character->GetScanEffect())
		{
			ScanComp->RetractScan();
		}

		if (ScanDeactivateSound)
		{
			UGameplayStatics::PlaySoundAtLocation(Character, ScanDeactivateSound,
				Character->GetActorLocation(), ScanSoundVolume, ScanSoundPitch);
		}

		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Optional hologram presentation. The scan remains functional when the UI
	// class is intentionally unset or when spawning the UI actor fails.
	if (HologramActorClass)
	{
		UCameraComponent* Cam = Character->GetHeadCamera();

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AActor* Hologram = Character->GetWorld()->SpawnActor<AActor>(HologramActorClass, Cam->GetComponentLocation(), FRotator::ZeroRotator, Params);

		if (Hologram)
		{
			// 先 Attach，再用相对坐标定位，保证永远在相机正前方固定位置
			Hologram->AttachToComponent(Cam, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			Hologram->SetActorRelativeLocation(FVector(SpawnDistance, 0.f, 0.f));
			Hologram->SetActorRelativeRotation(FRotator(0.f, 90.f, 0.f));
			SpawnedHologram = Hologram;
			// BP_uiFrame 的 BeginPlay（Autoplay=false）会自动调用 Show，不需要再手动触发

		}
	}

	// Always trigger gameplay scan visuals, even when the hologram UI was removed.
	if (UScanEffectComponent* ScanComp = Character->GetScanEffect())
	{
		const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector FeetOrigin = Character->GetActorLocation() - FVector(0.f, 0.f, HalfHeight);
		ScanComp->TriggerScan(FeetOrigin);
		bScanActive = true;
	}

	if (ScanActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(Character, ScanActivateSound,
			Character->GetActorLocation(), ScanSoundVolume, ScanSoundPitch);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
