#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/UI/EnemyHealthBarWidgetBase.h"

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FValidateEnemyHealthBarCommand,
	FAutomationTestBase*, Test);

bool FValidateEnemyHealthBarCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	Test->TestNotNull(TEXT("PIE world exists for enemy health bar test"), World);
	if (!World)
	{
		return true;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	UClass* PhantomClass = LoadClass<AEnemyBase>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
	Test->TestNotNull(TEXT("Phantom class loads for inherited health bar test"), PhantomClass);
	AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(
		PhantomClass ? PhantomClass : AEnemyBase::StaticClass(),
		FVector(0.f, 0.f, 300.f), FRotator::ZeroRotator, SpawnParameters);
	Test->TestNotNull(TEXT("Phantom spawns through EnemyBase in PIE"), Enemy);
	if (!Enemy)
	{
		return true;
	}

	UWidgetComponent* HealthBarComponent = Enemy->GetEnemyHealthBarComponent();
	Test->TestNotNull(TEXT("EnemyBase owns a health bar component"), HealthBarComponent);
	if (HealthBarComponent)
	{
		Test->TestEqual(TEXT("Health bar uses screen space"),
			HealthBarComponent->GetWidgetSpace(), EWidgetSpace::Screen);
		Test->TestEqual(TEXT("Health bar draw size"),
			HealthBarComponent->GetDrawSize(), FVector2D(180.f, 18.f));

		UEnemyHealthBarWidgetBase* HealthBar =
			Cast<UEnemyHealthBarWidgetBase>(HealthBarComponent->GetUserWidgetObject());
		Test->TestNotNull(TEXT("Health bar native widget is initialized"), HealthBar);
		if (HealthBar)
		{
			Test->TestEqual(TEXT("Health bar starts at full current health"),
				HealthBar->GetDisplayedCurrentHealthForTesting(), 100.f);
			Test->TestEqual(TEXT("Health bar starts at full max health"),
				HealthBar->GetDisplayedMaxHealthForTesting(), 100.f);

			if (UAbilitySystemComponent* AbilitySystem = Enemy->GetAbilitySystemComponent())
			{
				AbilitySystem->ApplyModToAttribute(
					UEnemyAttributeSetBase::GetHealthAttribute(), EGameplayModOp::Additive, -25.f);
				Test->TestEqual(TEXT("GAS health change refreshes the enemy health bar immediately"),
					HealthBar->GetDisplayedCurrentHealthForTesting(), 75.f);
			}
		}
	}

	Enemy->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FEnemyHealthBarPIETest,
	"TheManTest.Enemy.Shared.EnemyBaseHealthBar",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FEnemyHealthBarPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateEnemyHealthBarCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
