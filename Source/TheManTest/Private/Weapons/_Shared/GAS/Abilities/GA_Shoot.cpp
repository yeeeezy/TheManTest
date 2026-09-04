#include "Weapons/_Shared/GAS/Abilities/GA_Shoot.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Weapons/_Shared/Firearms/Bullets/BulletBase.h"
#include "Camera/CameraComponent.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UGA_Shoot::UGA_Shoot()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag    = TAG_Input_Weapon_PrimaryFire;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_Shoot::ActivateAbility(
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

	AFirearm* Firearm = Cast<AFirearm>(Character->GetEquipmentManager()->GetCurrentEquipment());
	if (!Firearm)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Ammo is authoritative on the equipped firearm. Empty weapons only produce
	// their dedicated trigger/mechanism click; all live-fire feedback stays gated.
	if (!Firearm->ConsumeRound())
	{
		if (Firearm->DryFireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Firearm->DryFireSound,
				Firearm->GetMuzzleWorldTransform().GetLocation(),
				Firearm->DryFireSoundVolumeMultiplier,
				Firearm->DryFireSoundPitchMultiplier);
		}
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UCameraComponent* Camera        = Character->GetHeadCamera();
	const FVector CameraLocation    = Camera->GetComponentLocation();
	const FVector CameraForward     = Camera->GetForwardVector();

	const FVector MuzzleLocation = Firearm->GetMuzzleWorldTransform().GetLocation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();

	// 子弹生成依赖 BulletClass；未配置子弹时跳过射击/命中逻辑，但仍播放蒙太奇/音效/后坐力
	if (Firearm->GetBulletClass())
	{
		if (Firearm->IsHitscan())
		{
			// ── Hitscan：立即判断命中，子弹生成在命中点作视觉效果 ──
			const FVector TraceEnd = CameraLocation + CameraForward * Firearm->GetHitscanRange();

			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Character);
			Params.AddIgnoredActor(Firearm);

			const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, CameraLocation, TraceEnd, ECC_Visibility, Params);
			const FVector HitPoint = bHit ? Hit.ImpactPoint : TraceEnd;
			DrawDebugLine(GetWorld(), MuzzleLocation, HitPoint, FColor::Red, false, 2.f, 0, 1.f);

			if (bHit)
			{
				ABulletBase* SpawnedBullet = GetWorld()->SpawnActor<ABulletBase>(
					Firearm->GetBulletClass(), Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), SpawnParams);
				if (SpawnedBullet)
				{
					SpawnedBullet->ProcessHit(Hit, Character, SourceASC);
				}
			}
		}
		else
		{
			// ── Projectile：从枪口生成，飞行碰撞后自动触发 ProcessHit ──
			ABulletBase* SpawnedBullet = GetWorld()->SpawnActor<ABulletBase>(
				Firearm->GetBulletClass(), MuzzleLocation, CameraForward.Rotation(), SpawnParams);
			if (SpawnedBullet)
			{
				SpawnedBullet->InitBullet(Character, SourceASC);
			}
		}
	}

	// 播放开火蒙太奇（与子弹解耦：空枪/未配子弹也能播）
	if (Firearm->MuzzleEffect)
	{
		if (UNiagaraComponent* Effect = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this, Firearm->MuzzleEffect, MuzzleLocation,
			CameraForward.Rotation() + Firearm->MuzzleEffectRotation,
			Firearm->MuzzleEffectScale, true, true, ENCPoolMethod::AutoRelease, true))
		{
			Effect->Activate(true);
		}
	}
	Firearm->PlayMuzzleFlashLight();

	if (Firearm->FireMontage)
	{
		if (UAnimInstance* ArmsAnimInst = Character->GetArmsMesh()->GetAnimInstance())
		{
			ArmsAnimInst->Montage_Play(Firearm->FireMontage);
		}
	}

	// 播放开火音效
	if (Firearm->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Firearm->FireSound, MuzzleLocation,
			Firearm->FireSoundVolumeMultiplier, Firearm->FireSoundPitchMultiplier);
	}

	// 武器专属射击震屏；仅本地玩家相机播放，不参与弹道、复制或服务器状态。
	if (Firearm->FireCameraShake)
	{
		if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
		{
			if (PC->PlayerCameraManager)
			{
				PC->PlayerCameraManager->StartCameraShake(
					Firearm->FireCameraShake, Firearm->FireCameraShakeScale);
			}
		}
	}

	// 施加后坐力
	if (Firearm->bEnableViewRecoil)
	{
		const float RandomYaw = FMath::RandRange(Firearm->RecoilYawMin, Firearm->RecoilYawMax);
		Character->AddRecoil(Firearm->RecoilPitch, RandomYaw, Firearm->RecoilDamping);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
