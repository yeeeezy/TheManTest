#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Core/TheManPlayerController.h"
#include "Core/TheManPlayerState.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Characters/CharacterBase/TheManAttributeSetBase.h"
#include "UI/Combat/CombatHUDWidgetBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Weapons/RepairGun/GAS/GameplayCues/GCN_RepairGunImpact.h"
#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Weapons/RepairGun/Bullets/RepairGunBullet.h"
#include "Enemy/EnemyBase.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Editor.h"
#include "HighResScreenshot.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/_Shared/GAS/GameplayCues/GCN_ImpactFeedbackBase.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThreeWeaponBaselineTest,
	"TheManTest.Player.Weapons.ThreeWeaponBaseline",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FThreeWeaponBaselineTest::RunTest(const FString& Parameters)
{
	const ABulletBase* BulletBaseDefault = GetDefault<ABulletBase>();
	TestNotNull(TEXT("BulletBase supplies a default hit effect"),
		BulletBaseDefault ? BulletBaseDefault->HitEffectClass.Get() : nullptr);
	TestEqual(TEXT("BulletBase default hit effect is shared GE_BulletDamage"),
		BulletBaseDefault && BulletBaseDefault->HitEffectClass
			? BulletBaseDefault->HitEffectClass->GetPathName()
			: FString(),
		FString(TEXT("/Game/Weapons/_Shared/GAS/Effects/GE_BulletDamage.GE_BulletDamage_C")));

	struct FWeaponExpectation
	{
		const TCHAR* Name;
		const TCHAR* WeaponClassPath;
		const TCHAR* MeshPath;
		const TCHAR* MuzzlePath;
		const TCHAR* ImpactPath;
		const TCHAR* DecalPath;
		const TCHAR* CueClassPath;
		const TCHAR* BulletClassPath;
		const TCHAR* ProjectileMeshPath;
		FGameplayTag ImpactTag;
		float DecalScale;
	};

	const FWeaponExpectation Expectations[] = {
		{
			TEXT("ElectricGun"),
			TEXT("/Game/Weapons/ElectricGun/Blueprint/BP_ElectricGun.BP_ElectricGun_C"),
			TEXT("/Game/Weapons/ElectricGun/Meshes/SM_ElectricGun.SM_ElectricGun"),
			TEXT("/Game/Weapons/ElectricGun/Effects/Muzzle/Systems/NS_ElectricGun_LaserMuzzle.NS_ElectricGun_LaserMuzzle"),
			TEXT("/Game/Weapons/ElectricGun/Effects/Impact/Systems/NS_ElectricGun_LaserImpact.NS_ElectricGun_LaserImpact"),
			TEXT("/Game/Weapons/ElectricGun/Effects/Impact/Materials/MI_ElectricGun_ImpactDecal.MI_ElectricGun_ImpactDecal"),
			TEXT("/Game/Weapons/ElectricGun/GAS/GameplayCues/GC_Weapon_ElectricGun_Impact.GC_Weapon_ElectricGun_Impact_C"),
			TEXT("/Game/Weapons/ElectricGun/Blueprint/BP_ElectricGunBullet.BP_ElectricGunBullet_C"),
			TEXT("/Game/Weapons/ElectricGun/Meshes/SM_ElectricGun_Projectile.SM_ElectricGun_Projectile"),
			TAG_GameplayCue_Weapon_ElectricGun_Impact,
			1.1f,
		},
		{
			TEXT("ExplosionGun"),
			TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGun.BP_ExplosionGun_C"),
			TEXT("/Game/Weapons/ExplosionGun/Meshes/SM_ExplosionGun.SM_ExplosionGun"),
			TEXT("/Game/Weapons/ExplosionGun/Effects/Muzzle/Systems/NS_ExplosionGun_Muzzle.NS_ExplosionGun_Muzzle"),
			TEXT("/Game/Weapons/ExplosionGun/Effects/Impact/Systems/NS_ExplosionGun_Impact.NS_ExplosionGun_Impact"),
			TEXT("/Game/Weapons/ExplosionGun/Effects/Impact/Materials/MI_ExplosionGun_ImpactDecal.MI_ExplosionGun_ImpactDecal"),
			TEXT("/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Impact.GC_Weapon_ExplosionGun_Impact_C"),
			TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"),
			TEXT("/Game/Weapons/ExplosionGun/Meshes/SM_ExplosionGun_Projectile.SM_ExplosionGun_Projectile"),
			TAG_GameplayCue_Weapon_ExplosionGun_Impact,
			2.0f,
		},
	};

	const UClass* RepairClass = LoadClass<AFirearm>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Blueprint/BP_RepairGun.BP_RepairGun_C"));
	const AFirearm* RepairGun = RepairClass ? RepairClass->GetDefaultObject<AFirearm>() : nullptr;
	TestNotNull(TEXT("RepairGun baseline loads"), RepairGun);
	TestNotNull(TEXT("RepairGun fire montage is configured"), RepairGun ? RepairGun->FireMontage : nullptr);
	TestFalse(TEXT("RepairGun view recoil is temporarily disabled"),
		RepairGun ? RepairGun->bEnableViewRecoil : true);
	TestNotNull(TEXT("RepairGun camera shake remains configured"),
		RepairGun ? RepairGun->FireCameraShake.Get() : nullptr);
	TestFalse(TEXT("RepairGun muzzle flash light stays disabled"),
		RepairGun ? RepairGun->bEnableMuzzleFlashLight : true);

	for (const FWeaponExpectation& Expected : Expectations)
	{
		UClass* WeaponClass = LoadClass<AFirearm>(nullptr, Expected.WeaponClassPath);
		const AFirearm* Weapon = WeaponClass ? WeaponClass->GetDefaultObject<AFirearm>() : nullptr;
		TestNotNull(FString::Printf(TEXT("%s class loads"), Expected.Name), Weapon);
		if (!Weapon || !RepairGun)
		{
			continue;
		}

		TestEqual(FString::Printf(TEXT("%s keeps RepairGun fire rate"), Expected.Name), Weapon->FireRate, RepairGun->FireRate);
		TestEqual(FString::Printf(TEXT("%s keeps RepairGun magazine"), Expected.Name), Weapon->MagazineCapacity, RepairGun->MagazineCapacity);
		TestEqual(FString::Printf(TEXT("%s keeps RepairGun recoil"), Expected.Name), Weapon->RecoilDamping, RepairGun->RecoilDamping);
		TestFalse(FString::Printf(TEXT("%s view recoil is temporarily disabled"), Expected.Name),
			Weapon->bEnableViewRecoil);
		TestNotNull(FString::Printf(TEXT("%s camera shake remains configured"), Expected.Name),
			Weapon->FireCameraShake.Get());
		TestNotNull(FString::Printf(TEXT("%s owns a muzzle flash light component"), Expected.Name),
			Weapon->GetMuzzleFlashLight());
		TestEqual(FString::Printf(TEXT("%s muzzle light uses source unitless intensity"), Expected.Name),
			Weapon->GetMuzzleFlashLight()->IntensityUnits, ELightUnits::Unitless);
		TestTrue(FString::Printf(TEXT("%s muzzle light uses inverse-square falloff"), Expected.Name),
			Weapon->GetMuzzleFlashLight()->bUseInverseSquaredFalloff);
		TestTrue(FString::Printf(TEXT("%s muzzle light affects translucent materials"), Expected.Name),
			Weapon->GetMuzzleFlashLight()->bAffectTranslucentLighting);
		const bool bExpectedMuzzleLight = FCString::Strcmp(Expected.Name, TEXT("ElectricGun")) == 0;
		TestEqual(FString::Printf(TEXT("%s muzzle flash light enablement is correct"), Expected.Name),
			Weapon->bEnableMuzzleFlashLight, bExpectedMuzzleLight);
		if (bExpectedMuzzleLight)
		{
			TestTrue(TEXT("ElectricGun muzzle location matches the source weapon"),
				Weapon->MuzzleLocalTransform.GetTranslation().Equals(
					FVector(0.000751f, 67.696307f, 6.434297f), KINDA_SMALL_NUMBER));
			TestTrue(TEXT("ElectricGun directional muzzle VFX stays aligned to camera forward"),
				Weapon->MuzzleEffectRotation.Equals(FRotator::ZeroRotator));
			TestTrue(TEXT("ElectricGun muzzle VFX uses source scale"),
				Weapon->MuzzleEffectScale.Equals(FVector::OneVector));
			TestEqual(TEXT("ElectricGun muzzle effect size scalar matches its largest axis"),
				Weapon->GetMuzzleEffectSizeScale(), 1.f);
			const FNiagaraVariable MuzzleScaleParameter(
				FNiagaraTypeDefinition::GetFloatDef(), TEXT("User.MuzzleScale"));
			TestTrue(TEXT("Electric muzzle exposes its sprite-size scale parameter"),
				Weapon->MuzzleEffect->GetExposedParameters().IndexOf(MuzzleScaleParameter) != INDEX_NONE);
			TestTrue(TEXT("ElectricGun muzzle light uses source cyan-green color"),
				Weapon->MuzzleFlashLightColor.Equals(
					FLinearColor(0.075319f, 1.f, 0.652928f, 1.f), KINDA_SMALL_NUMBER));
			TestEqual(TEXT("ElectricGun muzzle light intensity compensates for the UE 5.7 render path"),
				Weapon->MuzzleFlashLightIntensity, 1800.f);
			TestEqual(TEXT("ElectricGun muzzle light radius matches source"),
				Weapon->MuzzleFlashLightAttenuationRadius, 200.f);
			TestEqual(TEXT("ElectricGun muzzle light source radius matches source"),
				Weapon->MuzzleFlashLightSourceRadius, 60.f);
			TestTrue(TEXT("ElectricGun muzzle light is offset out of the barrel"),
				Weapon->MuzzleFlashLightLocalOffset.Equals(FVector(1.480382f, -6.734961f, -15.577805f)));
			TestEqual(TEXT("ElectricGun muzzle light fade is short"),
				Weapon->MuzzleFlashLightDuration, 0.1f);
		}
		TestTrue(FString::Printf(TEXT("%s owns its fire montage"), Expected.Name),
			Weapon->FireMontage && Weapon->FireMontage->GetPathName().Contains(Expected.Name));
		TestTrue(FString::Printf(TEXT("%s owns its equip montage"), Expected.Name),
			Weapon->GetEquipMontage() && Weapon->GetEquipMontage()->GetPathName().Contains(Expected.Name));
		TestEqual(FString::Printf(TEXT("%s uses requested mesh"), Expected.Name),
			Weapon->GetStaticMesh()->GetStaticMesh().Get(), LoadObject<UStaticMesh>(nullptr, Expected.MeshPath));
		TestNull(FString::Printf(TEXT("%s disables inherited skeletal mesh"), Expected.Name),
			Weapon->GetSkeletalMesh()->GetSkeletalMeshAsset());
		TestEqual(FString::Printf(TEXT("%s uses requested muzzle VFX"), Expected.Name),
			Weapon->MuzzleEffect.Get(), LoadObject<UNiagaraSystem>(nullptr, Expected.MuzzlePath));
		TestTrue(FString::Printf(TEXT("%s owns its bullet class"), Expected.Name),
			Weapon->BulletClass && Weapon->BulletClass->GetPathName().Contains(Expected.Name));

		UClass* BulletClass = LoadClass<ABulletBase>(nullptr, Expected.BulletClassPath);
		const ABulletBase* Bullet = BulletClass ? BulletClass->GetDefaultObject<ABulletBase>() : nullptr;
		TestNotNull(FString::Printf(TEXT("%s bullet loads"), Expected.Name), Bullet);
		TestTrue(FString::Printf(TEXT("%s bullet uses unique impact tag"), Expected.Name),
			Bullet && Bullet->ImpactCueTag.MatchesTagExact(Expected.ImpactTag));
		TestEqual(FString::Printf(TEXT("%s uses generated projectile mesh"), Expected.Name),
			Bullet && Bullet->BulletMesh ? Bullet->BulletMesh->GetStaticMesh().Get() : nullptr,
			LoadObject<UStaticMesh>(nullptr, Expected.ProjectileMeshPath));

		UClass* CueClass = LoadClass<UGCN_ImpactFeedbackBase>(nullptr, Expected.CueClassPath);
		const UGCN_ImpactFeedbackBase* Cue = CueClass
			? CueClass->GetDefaultObject<UGCN_ImpactFeedbackBase>() : nullptr;
		TestNotNull(FString::Printf(TEXT("%s impact cue loads"), Expected.Name), Cue);
		const FString RemovedCharacterImpactPath = FString::Printf(
			TEXT("/Game/Weapons/%s/Effects/Impact/Systems/NS_%s_EnemyImpact.NS_%s_EnemyImpact"),
			Expected.Name, Expected.Name, Expected.Name);
		TestNull(FString::Printf(TEXT("%s character-only impact asset is removed"), Expected.Name),
			LoadObject<UNiagaraSystem>(nullptr, *RemovedCharacterImpactPath));
		if (Cue)
		{
			TestTrue(FString::Printf(TEXT("%s cue uses unique tag"), Expected.Name),
				Cue->GameplayCueTag.MatchesTagExact(Expected.ImpactTag));
			TestEqual(FString::Printf(TEXT("%s uses requested impact VFX"), Expected.Name),
				Cue->ImpactEffect.Get(), Expected.ImpactPath
					? LoadObject<UNiagaraSystem>(nullptr, Expected.ImpactPath) : nullptr);
			TestEqual(FString::Printf(TEXT("%s uses requested decal"), Expected.Name),
				Cue->ImpactDecalMaterial.Get(), LoadObject<UMaterialInterface>(nullptr, Expected.DecalPath));
			TestEqual(FString::Printf(TEXT("%s uses requested decal scale"), Expected.Name),
				Cue->DecalSizeMultiplier, Expected.DecalScale);
		}
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FValidateEquippedWeaponAnimationCommand,
	FAutomationTestBase*, Test,
	FString, ExpectedWeaponName);

bool FValidateEquippedWeaponAnimationCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = Controller ? Cast<AFPSCharacterBase>(Controller->GetPawn()) : nullptr;
	AFirearm* Weapon = Player && Player->GetEquipmentManager()
		? Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()) : nullptr;
	Test->TestNotNull(FString::Printf(TEXT("%s is equipped in PIE"), *ExpectedWeaponName), Weapon);
	if (!Weapon || !Player)
	{
		return true;
	}

	Test->TestTrue(FString::Printf(TEXT("%s class is current"), *ExpectedWeaponName),
		Weapon->GetClass()->GetPathName().Contains(ExpectedWeaponName));
	Test->TestFalse(FString::Printf(TEXT("%s is visible after switch"), *ExpectedWeaponName),
		Weapon->IsHidden());
	Test->TestNotNull(FString::Printf(TEXT("%s has its weapon mesh"), *ExpectedWeaponName),
		Weapon->GetStaticMesh()->GetStaticMesh().Get());
	Test->TestNotNull(FString::Printf(TEXT("%s has its fire montage"), *ExpectedWeaponName),
		Weapon->FireMontage);

	UAnimInstance* ArmsAnimInstance = Player->GetArmsMesh()
		? Player->GetArmsMesh()->GetAnimInstance() : nullptr;
	Test->TestNotNull(FString::Printf(TEXT("%s arms animation instance exists"), *ExpectedWeaponName),
		ArmsAnimInstance);
	if (ArmsAnimInstance && Weapon->FireMontage)
	{
		const float MontageLength = ArmsAnimInstance->Montage_Play(Weapon->FireMontage);
		Test->TestTrue(FString::Printf(TEXT("%s fire montage starts on the live arms"), *ExpectedWeaponName),
			MontageLength > 0.f && ArmsAnimInstance->Montage_IsPlaying(Weapon->FireMontage));
		ArmsAnimInstance->Montage_Stop(0.f, Weapon->FireMontage);
	}

	if (ExpectedWeaponName == TEXT("ElectricGun"))
	{
		Weapon->PlayMuzzleFlashLight();
		Test->TestTrue(TEXT("ElectricGun muzzle flash light becomes visible when fired"),
			Weapon->GetMuzzleFlashLight() && Weapon->GetMuzzleFlashLight()->IsVisible());
		Test->TestEqual(TEXT("ElectricGun muzzle flash light starts at configured intensity"),
			Weapon->GetMuzzleFlashLight() ? Weapon->GetMuzzleFlashLight()->Intensity : 0.f,
			Weapon->MuzzleFlashLightIntensity);
		Test->TestEqual(TEXT("ElectricGun muzzle light attenuation follows MuzzleEffectScale"),
			Weapon->GetMuzzleFlashLight() ? Weapon->GetMuzzleFlashLight()->AttenuationRadius : 0.f,
			Weapon->MuzzleFlashLightAttenuationRadius * Weapon->GetMuzzleEffectSizeScale());
		Test->TestEqual(TEXT("ElectricGun muzzle light source radius follows MuzzleEffectScale"),
			Weapon->GetMuzzleFlashLight() ? Weapon->GetMuzzleFlashLight()->SourceRadius : 0.f,
			Weapon->MuzzleFlashLightSourceRadius * Weapon->GetMuzzleEffectSizeScale());
		Test->TestTrue(TEXT("ElectricGun muzzle flash light uses the configured muzzle-local offset"),
			Weapon->GetMuzzleFlashLight() && Weapon->GetMuzzleFlashLight()->GetComponentLocation().Equals(
				Weapon->GetMuzzleWorldTransform().TransformPosition(Weapon->MuzzleFlashLightLocalOffset), 0.1f));
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FValidateElectricMuzzleLightFadedCommand,
	FAutomationTestBase*, Test);

bool FValidateElectricMuzzleLightFadedCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = Controller ? Cast<AFPSCharacterBase>(Controller->GetPawn()) : nullptr;
	AFirearm* Weapon = Player && Player->GetEquipmentManager()
		? Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()) : nullptr;
	Test->TestNotNull(TEXT("ElectricGun remains equipped while muzzle light fades"), Weapon);
	if (Weapon)
	{
		Test->TestTrue(TEXT("ElectricGun remains current during muzzle light fade check"),
			Weapon->GetClass()->GetPathName().Contains(TEXT("ElectricGun")));
		Test->TestFalse(TEXT("ElectricGun muzzle flash light hides after its short fade"),
			Weapon->GetMuzzleFlashLight() && Weapon->GetMuzzleFlashLight()->IsVisible());
		Test->TestEqual(TEXT("ElectricGun muzzle flash light reaches zero intensity"),
			Weapon->GetMuzzleFlashLight() ? Weapon->GetMuzzleFlashLight()->Intensity : -1.f,
			0.f);
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FSwitchToNextWeaponCommand, FAutomationTestBase*, Test);

bool FSwitchToNextWeaponCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = Controller ? Cast<AFPSCharacterBase>(Controller->GetPawn()) : nullptr;
	UEquipmentManagerComponent* Manager = Player ? Player->GetEquipmentManager() : nullptr;
	Test->TestNotNull(TEXT("Equipment manager exists before switching"), Manager);
	if (Manager)
	{
		Manager->SwitchEquipment(1);
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FTriggerElectricMuzzleFlashCommand,
	FAutomationTestBase*, Test);

bool FTriggerElectricMuzzleFlashCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = Controller ? Cast<AFPSCharacterBase>(Controller->GetPawn()) : nullptr;
	AFirearm* Weapon = Player && Player->GetEquipmentManager()
		? Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()) : nullptr;
	Test->TestNotNull(TEXT("ElectricGun is available for reference-video capture"), Weapon);
	if (!Player || !Weapon)
	{
		return true;
	}

	Test->TestTrue(TEXT("ElectricGun is equipped for reference-video capture"),
		Weapon->GetClass()->GetPathName().Contains(TEXT("ElectricGun")));
	Player->PrimaryFire();
	Test->TestTrue(TEXT("ElectricGun muzzle light is active on the captured firing frame"),
		Weapon->GetMuzzleFlashLight() && Weapon->GetMuzzleFlashLight()->IsVisible());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCaptureElectricMuzzleFlashCommand,
	FAutomationTestBase*, Test);

bool FCaptureElectricMuzzleFlashCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* Controller = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = Controller ? Cast<AFPSCharacterBase>(Controller->GetPawn()) : nullptr;
	AFirearm* Weapon = Player && Player->GetEquipmentManager()
		? Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()) : nullptr;
	Test->TestTrue(TEXT("ElectricGun muzzle light is still active when pixels are captured"),
		Weapon && Weapon->GetMuzzleFlashLight() && Weapon->GetMuzzleFlashLight()->IsVisible());
	if (Weapon && Weapon->GetMuzzleFlashLight())
	{
		Test->AddInfo(FString::Printf(TEXT("Captured ElectricGun muzzle light intensity: %.2f"),
			Weapon->GetMuzzleFlashLight()->Intensity));
		Test->AddInfo(FString::Printf(TEXT("Captured ElectricGun muzzle scale %.2f, attenuation %.2f, source radius %.2f"),
			Weapon->GetMuzzleEffectSizeScale(),
			Weapon->GetMuzzleFlashLight()->AttenuationRadius,
			Weapon->GetMuzzleFlashLight()->SourceRadius));
	}

	UGameViewportClient* GameViewportClient = World ? World->GetGameViewport() : nullptr;
	FViewport* GameViewport = GameViewportClient ? GameViewportClient->Viewport : nullptr;
	Test->TestNotNull(TEXT("PIE game viewport is available for reference-video capture"), GameViewport);
	if (!GameViewport)
	{
		return true;
	}

	TArray<FColor> Pixels;
	const FIntPoint ViewportSize = GameViewport->GetSizeXY();
	const bool bReadPixels = GameViewport->ReadPixels(Pixels);
	Test->TestTrue(TEXT("PIE game viewport pixels were read"), bReadPixels);
	if (!bReadPixels || Pixels.Num() != ViewportSize.X * ViewportSize.Y)
	{
		return true;
	}

	const FString Filename = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Screenshots/WindowsEditor/TMT_ElectricMuzzle_VideoStandard.png"));
	TArray64<uint8> CompressedPng;
	FImageUtils::PNGCompressImageArray(ViewportSize.X, ViewportSize.Y, Pixels, CompressedPng);
	Test->TestTrue(TEXT("ElectricGun reference-video screenshot was written"),
		FFileHelper::SaveArrayToFile(CompressedPng, *Filename));
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FValidateElectricMuzzleCaptureCommand,
	FAutomationTestBase*, Test);

bool FValidateElectricMuzzleCaptureCommand::Update()
{
	const FString Filename = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Screenshots/WindowsEditor/TMT_ElectricMuzzle_VideoStandard.png"));
	Test->TestTrue(TEXT("ElectricGun reference-video screenshot was written"),
		FPaths::FileExists(Filename));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FThreeWeaponPIESwitchTest,
	"TheManTest.Player.Weapons.ThreeWeaponPIESwitch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FThreeWeaponPIESwitchTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateEquippedWeaponAnimationCommand(this, TEXT("RepairGun")));
	ADD_LATENT_AUTOMATION_COMMAND(FSwitchToNextWeaponCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.7f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateEquippedWeaponAnimationCommand(this, TEXT("ElectricGun")));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateElectricMuzzleLightFadedCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FSwitchToNextWeaponCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.7f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateEquippedWeaponAnimationCommand(this, TEXT("ExplosionGun")));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FElectricMuzzleVisualCaptureTest,
	"TheManTest.Player.Weapons.ElectricMuzzleVisualCapture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FElectricMuzzleVisualCaptureTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FSwitchToNextWeaponCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.7f));
	// Warm the migrated Niagara systems before capturing the measured shot.
	ADD_LATENT_AUTOMATION_COMMAND(FTriggerElectricMuzzleFlashCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FTriggerElectricMuzzleFlashCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.016f));
	ADD_LATENT_AUTOMATION_COMMAND(FCaptureElectricMuzzleFlashCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.3f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateElectricMuzzleCaptureCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FValidateCombatHUDCommand, FAutomationTestBase*, Test);
bool FValidateCombatHUDCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	ATheManPlayerController* Controller = World
		? Cast<ATheManPlayerController>(World->GetFirstPlayerController())
		: nullptr;
	AFPSCharacterBase* PlayerCharacter = Controller
		? Cast<AFPSCharacterBase>(Controller->GetPawn())
		: nullptr;
	UCombatHUDWidgetBase* Widget = Controller ? Controller->GetCombatHUDWidgetForTesting() : nullptr;
	AFirearm* Firearm = PlayerCharacter && PlayerCharacter->GetEquipmentManager()
		? Cast<AFirearm>(PlayerCharacter->GetEquipmentManager()->GetCurrentEquipment())
		: nullptr;
	if (!Controller || !PlayerCharacter || !Widget || !Firearm)
	{
		return false;
	}

	Test->TestTrue(TEXT("Combat HUD is added to the local player screen"), Widget->IsInViewport());
	Test->TestNotNull(TEXT("Player controller has IA_Reload configured"), Controller->GetReloadAction());
	Test->TestTrue(TEXT("Ammo block is visible for a firearm"), Widget->IsAmmoVisibleForTesting());
	Test->TestEqual(TEXT("Default current ammo"), Firearm->GetCurrentAmmo(), 30);
	Test->TestEqual(TEXT("Default magazine capacity"), Firearm->GetMagazineCapacity(), 30);
	Test->TestEqual(TEXT("Default spare magazine count"), Firearm->GetSpareMagazineCount(), 3);
	USoundBase* ExpectedDryFireSound = LoadObject<USoundBase>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Audio/S_RepairGun_DryFire.S_RepairGun_DryFire"));
	Test->TestNotNull(TEXT("RepairGun dry-fire sound asset loads"), ExpectedDryFireSound);
	Test->TestEqual(TEXT("Equipped RepairGun uses its dedicated dry-fire sound"), Firearm->DryFireSound, ExpectedDryFireSound);
	Test->TestEqual(TEXT("HUD displays current ammo"), Widget->GetDisplayedCurrentAmmoForTesting(), 30);
	Test->TestEqual(TEXT("HUD displays magazine capacity"), Widget->GetDisplayedMagazineCapacityForTesting(), 30);
	Test->TestEqual(TEXT("HUD displays spare magazines"), Widget->GetDisplayedSpareMagazineCountForTesting(), 3);
	PlayerCharacter->Reload();
	Test->TestEqual(TEXT("Reload input does not consume a spare magazine while full"), Firearm->GetSpareMagazineCount(), 3);
	Test->TestTrue(TEXT("Health block is visible for the possessed player"), Widget->IsHealthVisibleForTesting());
	Test->TestEqual(TEXT("HUD displays default current health"), Widget->GetDisplayedCurrentHealthForTesting(), 100.f);
	if (ATheManPlayerState* TheManPlayerState = Controller->GetPlayerState<ATheManPlayerState>())
	{
		UAbilitySystemComponent* AbilitySystem = TheManPlayerState->GetAbilitySystemComponent();
		Test->TestNotNull(TEXT("Player state owns an ability system"), AbilitySystem);
		if (AbilitySystem)
		{
			AbilitySystem->ApplyModToAttribute(
				UTheManAttributeSetBase::GetHealthAttribute(), EGameplayModOp::Additive, -25.f);
			Test->TestEqual(TEXT("Health delegate updates HUD immediately"), Widget->GetDisplayedCurrentHealthForTesting(), 75.f);
			AbilitySystem->ApplyModToAttribute(
				UTheManAttributeSetBase::GetHealthAttribute(), EGameplayModOp::Additive, 25.f);
		}
	}

	Test->TestTrue(TEXT("A round can be consumed"), Firearm->ConsumeRound());
	Test->TestEqual(TEXT("Firing decrements weapon ammo"), Firearm->GetCurrentAmmo(), 29);
	Test->TestEqual(TEXT("Ammo delegate updates HUD immediately"), Widget->GetDisplayedCurrentAmmoForTesting(), 29);
	for (int32 Shot = 0; Shot < 29; ++Shot)
	{
		Test->TestTrue(TEXT("Remaining magazine round can be consumed"), Firearm->ConsumeRound());
	}
	Test->TestEqual(TEXT("Magazine reaches empty"), Firearm->GetCurrentAmmo(), 0);
	Test->TestEqual(TEXT("HUD displays empty magazine"), Widget->GetDisplayedCurrentAmmoForTesting(), 0);
	Test->TestFalse(TEXT("Empty magazine rejects an additional shot"), Firearm->ConsumeRound());
	UAbilitySystemComponent* AbilitySystem = PlayerCharacter->GetAbilitySystemComponent();
	Test->TestNotNull(TEXT("Player owns an ability system for reload input"), AbilitySystem);
	if (AbilitySystem)
	{
		PlayerCharacter->Reload();
	}
	Test->TestEqual(TEXT("Reload fills current magazine"), Firearm->GetCurrentAmmo(), 30);
	Test->TestEqual(TEXT("Reload consumes one spare magazine"), Firearm->GetSpareMagazineCount(), 2);
	Test->TestEqual(TEXT("HUD displays reloaded ammo"), Widget->GetDisplayedCurrentAmmoForTesting(), 30);
	Test->TestEqual(TEXT("HUD displays decremented spare magazines"), Widget->GetDisplayedSpareMagazineCountForTesting(), 2);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FCombatHUDScreenshotCommand);
bool FCombatHUDScreenshotCommand::Update()
{
	const FString Filename = FPaths::Combine(
		FPaths::ProjectSavedDir(),
		TEXT("Screenshots/WindowsEditor/TMT_CombatHUD.png"));
	FScreenshotRequest::RequestScreenshot(Filename, true, false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCombatHUDAndAmmoTest,
	"TheManTest.Player.CombatHUD.AmmoLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCombatHUDAndAmmoTest::RunTest(const FString& Parameters)
{
	USoundBase* RepairGunImpactSound = LoadObject<USoundBase>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Audio/S_RepairGun_Impact.S_RepairGun_Impact"));
	TestNotNull(TEXT("RepairGun impact sound loads"), RepairGunImpactSound);
	UClass* RepairGunCueClass = LoadClass<UGCN_RepairGunImpact>(nullptr,
		TEXT("/Game/Weapons/RepairGun/GAS/GameplayCues/GC_Weapon_RepairGun_Impact.GC_Weapon_RepairGun_Impact_C"));
	const UGCN_RepairGunImpact* RepairGunCue = RepairGunCueClass
		? Cast<UGCN_RepairGunImpact>(RepairGunCueClass->GetDefaultObject()) : nullptr;
	TestNotNull(TEXT("RepairGun impact cue loads"), RepairGunCue);
	if (RepairGunCue)
	{
		TestEqual(TEXT("RepairGun impact cue owns its sound"), RepairGunCue->ImpactSound.Get(), RepairGunImpactSound);
		TestTrue(TEXT("RepairGun impact cue uses its weapon tag"),
			RepairGunCue->GameplayCueTag.MatchesTagExact(TAG_GameplayCue_Weapon_RepairGun_Impact));
	}
	const ARepairGunBullet* RepairGunBullet = GetDefault<ARepairGunBullet>();
	TestTrue(TEXT("RepairGun bullet selects the RepairGun impact cue"),
		RepairGunBullet->ImpactCueTag.MatchesTagExact(TAG_GameplayCue_Weapon_RepairGun_Impact));

	UClass* EnemyCueClass = LoadClass<UGCN_EnemyHit>(nullptr,
		TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"));
	const UGCN_EnemyHit* EnemyCue = EnemyCueClass
		? Cast<UGCN_EnemyHit>(EnemyCueClass->GetDefaultObject()) : nullptr;
	TestNotNull(TEXT("Enemy-owned hit cue loads"), EnemyCue);
	if (EnemyCue)
	{
		TestTrue(TEXT("Enemy hit cue uses the enemy tag"),
			EnemyCue->GameplayCueTag.MatchesTagExact(TAG_GameplayCue_Character_Enemy_Hit));
	}
	TestTrue(TEXT("Enemy base selects its independently configurable hit cue"),
		GetDefault<AEnemyBase>()->GetHitReactionCueTag().MatchesTagExact(TAG_GameplayCue_Character_Enemy_Hit));

	UClass* DamageEffectClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/Weapons/_Shared/GAS/Effects/GE_BulletDamage.GE_BulletDamage_C"));
	const UGameplayEffect* DamageEffect = DamageEffectClass
		? Cast<UGameplayEffect>(DamageEffectClass->GetDefaultObject()) : nullptr;
	TestNotNull(TEXT("Shared bullet damage effect loads"), DamageEffect);
	TestTrue(TEXT("Shared damage effect remains presentation-free"),
		DamageEffect && DamageEffect->GameplayCues.IsEmpty());

	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateCombatHUDCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FCombatHUDScreenshotCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.3f));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
