#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Misc/Paths.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphFlight.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

namespace
{
TWeakObjectPtr<ACoreMorphBoss> EndingBoss;
ACoreMorphBoss* FindBoss()
{
	if (UWorld* W = GEditor ? GEditor->PlayWorld : nullptr)
		for (TActorIterator<ACoreMorphBoss> It(W); It; ++It) return *It;
	return nullptr;
}

class FCoreMorphRealtime : public IAutomationLatentCommand
{
	FAutomationTestBase* Test;
	double Started = 0;
public:
	explicit FCoreMorphRealtime(FAutomationTestBase* In) : Test(In) {}
	bool Update() override
	{
		auto* Boss = FindBoss();
		if (!Boss) { Test->AddError(TEXT("Missing boss for real-time flight")); return true; }
		if (!Started)
		{
			Started = FPlatformTime::Seconds();
			Boss->ResetFlightPreview();
			Test->TestTrue(TEXT("Real-time GA starts"), Boss->StartFlightPreview());
			return false;
		}
		if (Boss->Flight->IsFlying() && FPlatformTime::Seconds() - Started < 30) return false;
		Test->TestFalse(TEXT("Normal world ticking completes the flight GA"), Boss->Flight->IsFlying());
		Test->TestNearlyEqual(TEXT("Real-time flight clamps at release"), Boss->Flight->GetFlightSeconds(), 13.4f, .002f);
		Boss->ResetFlightPreview();
		Boss->StartFlightPreview();
		EndingBoss = Boss;
		return true; // Exit PIE with the GA still active.
	}
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCoreMorphAfterExit, FAutomationTestBase*, Test);
bool FCoreMorphAfterExit::Update()
{
	Test->TestNull(TEXT("PIE world is closed"), GEditor->PlayWorld);
	Test->TestFalse(TEXT("Active-flight boss is destroyed on PIE exit"), EndingBoss.IsValid());
	EndingBoss.Reset();
	return true;
}
void Step(ACoreMorphBoss* Boss, float Seconds)
{
	Boss->Flight->SetPaused(false);
	for (float Left = Seconds; Left > KINDA_SMALL_NUMBER; Left -= 1.f / 60.f)
		Boss->Flight->TickComponent(FMath::Min(Left, 1.f / 60.f), LEVELTICK_All, nullptr);
	Boss->Flight->SetPaused(true);
}
void Damage(ACoreMorphBoss* Boss, float Amount)
{
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage());
	Effect->DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayModifierInfo& Mod = Effect->Modifiers.AddDefaulted_GetRef();
	Mod.Attribute = UEnemyAttributeSetBase::GetHealthAttribute();
	Mod.ModifierOp = EGameplayModOp::Additive;
	Mod.ModifierMagnitude = FScalableFloat(-Amount);
	auto* ASC = Boss->GetAbilitySystemComponent();
	ASC->ApplyGameplayEffectToSelf(Effect, 1.f, ASC->MakeEffectContext());
}
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCoreMorphValidate, FAutomationTestBase*, Test);
bool FCoreMorphValidate::Update()
{
	auto* Boss = FindBoss();
	if (!Test->TestNotNull(TEXT("Final boss Blueprint loaded into actual PIE"), Boss)) return true;
	auto* ASC = Boss->GetAbilitySystemComponent();
	Test->TestEqual(TEXT("One ASC on one boss"), TInlineComponentArray<UAbilitySystemComponent*>(Boss).Num(), 1);
	Test->TestEqual(TEXT("One attribute set"), ASC->GetSpawnedAttributes().Num(), 1);
	Test->TestEqual(TEXT("One flight ability grant"), ASC->GetActivatableAbilities().Num(), 1);
	Test->TestEqual(TEXT("All 154 final manta pieces"), Boss->Flight->GetPieces().Num(), 154);
	Test->TestEqual(TEXT("No serialized component duplication in PIE"), TInlineComponentArray<UStaticMeshComponent*>(Boss).Num(), 154);
	for (const auto& Piece : Boss->Flight->GetPieces())
	{
		Test->TestTrue(TEXT("Every hit surface belongs to this boss"), Piece->GetOwner() == Boss);
		Test->TestFalse(TEXT("No piece simulates ragdoll physics"), Piece->IsSimulatingPhysics());
		Test->TestTrue(TEXT("Final mesh reference loads"), Piece->GetStaticMesh() != nullptr);
	}
	Test->TestTrue(TEXT("Form is owned by a GE"), ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Form_Manta));
	Test->TestNull(TEXT("Boss has no humanoid skeletal asset"), Boss->GetMesh()->GetSkeletalMeshAsset());
	Test->TestFalse(TEXT("Boss has no humanoid blood cue"), Boss->GetHitReactionCueTag().IsValid());
	Boss->ResetFlightPreview();
	const FVector Initial = Boss->GetActorLocation();
	Test->TestTrue(TEXT("Flight starts through GA"), Boss->StartFlightPreview());
	Boss->Flight->SetComponentTickEnabled(false);
	Test->TestFalse(TEXT("Duplicate flight cannot activate"), Boss->StartFlightPreview());
	Step(Boss, 5.f);
	Test->TestTrue(TEXT("Actual boss identity follows flight by over 50 m"), FVector::Dist(Boss->GetActorLocation(), Initial) > 5000.f);
	const FRotator Heading = Boss->GetActorRotation();
	Boss->ReactToProjectileHit(Boss->GetWorld()->GetFirstPlayerController());
	Damage(Boss, 25);
	Test->TestEqual(TEXT("GE damage updates the one health pool"), ASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()), 75.f);
	Test->TestTrue(TEXT("Damage does not snap heading or cancel flight"), Boss->GetActorRotation().Equals(Heading) && Boss->Flight->IsFlying());
	Boss->SetCombatPhase(2);
	Test->TestEqual(TEXT("Combat phase changes independently"), Boss->GetCombatPhase(), 2);
	Test->TestTrue(TEXT("Phase leaves manta form intact"), Boss->CurrentForm == ECoreMorphForm::Manta && ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Form_Manta));
	Test->TestEqual(TEXT("Phase does not grant flight twice"), ASC->GetActivatableAbilities().Num(), 1);
	ASC->CancelAllAbilities();
	const FVector Cancelled = Boss->GetActorLocation();
	Step(Boss, .5f);
	Test->TestTrue(TEXT("Cancellation freezes movement and ends the GA"), !Boss->Flight->IsFlying() && Boss->GetActorLocation().Equals(Cancelled, .01));
	Boss->ResetFlightPreview();
	Test->TestTrue(TEXT("Reset restores the original choreography frame"), Boss->GetActorLocation().Equals(Initial, .01));
	Test->TestEqual(TEXT("Replay does not heal"), ASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()), 75.f);
	Test->TestEqual(TEXT("Replay does not reset combat phase"), Boss->GetCombatPhase(), 2);
	Test->TestTrue(TEXT("Replay can activate same granted GA"), Boss->StartFlightPreview());
	Boss->Flight->SetComponentTickEnabled(false);
	Step(Boss, 10.f);
	Test->TestTrue(TEXT("Accepted high climb remains above 400 m"), Boss->GetActorLocation().Z - Initial.Z > 40000);
	// Cross the boundary with an ordinary frame and verify clamping, avoiding float summation at exactly 13.4.
	Step(Boss, 3.5f);
	Test->TestFalse(TEXT("Batch one ends before particle release"), Boss->Flight->IsFlying());
	Test->TestNearlyEqual(TEXT("Release boundary is 13.4 seconds"), Boss->Flight->GetFlightSeconds(), 13.4f, .002f);
	Test->TestFalse(TEXT("GA has ended at release boundary"), ASC->FindAbilitySpecFromClass(UGA_CoreMorphFlight::StaticClass())->IsActive());
	Test->TestFalse(TEXT("No implicit replay after completion"), Boss->StartFlightPreview());
	Boss->ResetFlightPreview();
	Boss->StartFlightPreview();
	Boss->Flight->SetComponentTickEnabled(false);
	Step(Boss, 2.f);
	Damage(Boss, 1000);
	Test->TestTrue(TEXT("GE lethal damage enters EnemyBase death lifecycle"), Boss->IsDead());
	Test->TestFalse(TEXT("Death cancels flight"), Boss->Flight->IsFlying());
	Test->TestFalse(TEXT("Death stops flight ticking"), Boss->Flight->IsComponentTickEnabled());
	Test->TestFalse(TEXT("Death removes form effect"), ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Form_Manta));
	for (const auto& Piece : Boss->Flight->GetPieces())
		Test->TestTrue(TEXT("Death disables surfaces without physics"), Piece->GetCollisionEnabled() == ECollisionEnabled::NoCollision && !Piece->IsSimulatingPhysics());
	Test->TestFalse(TEXT("Dead boss cannot start flight"), Boss->StartFlightPreview());
	Boss->ResetFlightPreview();
	Test->TestFalse(TEXT("Reset cannot revive dead boss"), Boss->Flight->IsComponentTickEnabled());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCoreMorphAfterDeath, FAutomationTestBase*, Test);
bool FCoreMorphAfterDeath::Update()
{
	Test->TestNull(TEXT("Dead boss is destroyed after the base lifetime"), FindBoss());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FCoreMorphFrame, FAutomationTestBase*, Test, float, Seconds);
bool FCoreMorphFrame::Update()
{
	auto* Boss = FindBoss();
	if (!Test->TestNotNull(TEXT("Fresh PIE boss for rendered flight review"), Boss)) return true;
	if (Seconds == 0)
	{
		Test->TestEqual(TEXT("PIE restart recreates exactly 154 components"), TInlineComponentArray<UStaticMeshComponent*>(Boss).Num(), 154);
		Boss->ResetFlightPreview();
		Boss->StartFlightPreview();
		Boss->Flight->SetComponentTickEnabled(false);
	}
	Step(Boss, Seconds - Boss->Flight->GetFlightSeconds());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCoreMorphScreenshot, FString, Name);
bool FCoreMorphScreenshot::Update()
{
	FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("CoreMorphMigration") / Name), false, false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphFlightPIE, "TheManTest.Enemy.CoreMorph.FlightBatch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCoreMorphFlightPIE::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/CoreMorph/L_CoreMorphFlight"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphValidate(this));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphAfterDeath(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	for (float Time : {0.f, 5.f, 10.f, 13.4f})
	{
		ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphFrame(this, Time));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.5f));
		ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphScreenshot(FString::Printf(TEXT("Flight-%04d.png"), FMath::RoundToInt(Time * 100))));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.4f));
	}
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphRealtime(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	// FEndPlayMapCommand requests teardown; the editor performs it on its next tick.
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.3f));
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphAfterExit(this));
	return true;
}
#endif
