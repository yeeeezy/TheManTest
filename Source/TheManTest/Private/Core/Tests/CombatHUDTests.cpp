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
#include "Misc/Paths.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Sound/SoundBase.h"

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
		TEXT("/Game/Weapons/RepairGun/GAS/GameplayCues/GC_RepairGun_Impact.GC_RepairGun_Impact_C"));
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
		TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Enemy_Hit.GC_Enemy_Hit_C"));
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
