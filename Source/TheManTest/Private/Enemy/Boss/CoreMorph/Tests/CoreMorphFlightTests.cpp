#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Subsystems/EditorActorSubsystem.h"
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
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"
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
	Test->TestTrue(TEXT("Review map body stays at its pre-fix world placement"), Initial.Equals(FVector(-16000, 0, 2500), .01));
	Test->TestTrue(TEXT("Original world choreography is preserved"), Boss->Flight->GetChoreographyFrame().Equals(FTransform(FVector(0, 0, 900)), .001));
	Test->TestTrue(TEXT("Flight starts through GA"), Boss->StartFlightPreview());
	Boss->Flight->SetComponentTickEnabled(false);
	Test->TestFalse(TEXT("Duplicate flight cannot activate"), Boss->StartFlightPreview());
	Step(Boss, 5.f);
	Test->TestTrue(TEXT("Actual boss identity follows flight by over 50 m"), FVector::Dist(Boss->GetActorLocation(), Initial) > 5000.f);
	Test->TestTrue(TEXT("Measured ascent drives the tail before the old timed pulse"), Boss->Flight->GetTailMotion().Climb > .2f);
	const FTransform PausedTail = Boss->Flight->GetPieces().Last()->GetComponentTransform();
	const float PausedPhase = Boss->Flight->GetTailMotion().Phase;
	Boss->Flight->TickComponent(.5f, LEVELTICK_All, nullptr);
	Test->TestEqual(TEXT("Pause freezes tail phase"), Boss->Flight->GetTailMotion().Phase, PausedPhase);
	Test->TestTrue(TEXT("Pause freezes piece pose"), Boss->Flight->GetPieces().Last()->GetComponentTransform().Equals(PausedTail));
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
	const float CancelledPhase = Boss->Flight->GetTailMotion().Phase;
	Step(Boss, .5f);
	Test->TestTrue(TEXT("Cancellation freezes movement and ends the GA"), !Boss->Flight->IsFlying() && Boss->GetActorLocation().Equals(Cancelled, .01));
	Test->TestEqual(TEXT("Cancellation also freezes tail motion"), Boss->Flight->GetTailMotion().Phase, CancelledPhase);
	Boss->ResetFlightPreview();
	Test->TestEqual(TEXT("Reset clears accumulated climb response"), Boss->Flight->GetTailMotion().Climb, 0.f);
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
	Test->TestTrue(TEXT("Descent fades the climbing tail response"), Boss->Flight->GetTailMotion().Climb < .05f);
	Test->TestFalse(TEXT("No implicit replay after completion"), Boss->StartFlightPreview());
	Boss->ResetFlightPreview();
	Boss->StartFlightPreview();
	Boss->Flight->SetComponentTickEnabled(false);
	Step(Boss, 2.f);
	const float BeforeDeathPhase = Boss->Flight->GetTailMotion().Phase;
	Damage(Boss, 1000);
	Boss->Flight->TickComponent(.5f, LEVELTICK_All, nullptr);
	Test->TestEqual(TEXT("Death cannot advance tail motion"), Boss->Flight->GetTailMotion().Phase, BeforeDeathPhase);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphTailResponse, "TheManTest.Enemy.CoreMorph.TailMotion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCoreMorphTailResponse::RunTest(const FString& Parameters)
{
	auto Advance = [](FCoreMorphTailMotion& Motion, FVector Velocity, int32 Frames, float Dt)
	{
		for (int32 I = 0; I < Frames; ++I) Motion.Update(Velocity, Dt);
	};
	FCoreMorphTailMotion Level, Climbing, Descending, Rest;
	Advance(Level, FVector(11000, 0, 0), 120, 1.f / 60.f);
	Advance(Climbing, FVector(11000, 0, 8000), 120, 1.f / 60.f);
	Advance(Descending, FVector(11000, 0, -8000), 120, 1.f / 60.f);
	Advance(Rest, FVector::ZeroVector, 120, 1.f / 60.f);
	TestTrue(TEXT("Actual ascent builds a strong response"), Climbing.Climb > .99f);
	TestEqual(TEXT("Level flight never triggers the climb wave"), Level.Climb, 0.f);
	TestEqual(TEXT("Descent is not mistaken for climb"), Descending.Climb, 0.f);
	TestEqual(TEXT("Stationary motion has no flight stroke"), Rest.Movement, 0.f);
	FVector LevelTip = FVector::ZeroVector, ClimbTip = FVector::ZeroVector, Root = FVector::ZeroVector;
	FRotator LevelBend = FRotator::ZeroRotator, ClimbBend = FRotator::ZeroRotator, RootBend = FRotator::ZeroRotator;
	Level.Apply(1, LevelTip, LevelBend);
	Climbing.Apply(1, ClimbTip, ClimbBend);
	Climbing.Apply(0, Root, RootBend);
	TestTrue(TEXT("Climbing actually increases piece vertical deflection"), FMath::Abs(ClimbTip.Z) > FMath::Abs(LevelTip.Z) * 4);
	TestTrue(TEXT("Tail base remains anchored"), Root.IsZero() && RootBend.IsZero());
	// Trigger ascent after a completely different amount of level flight. No route timestamp is supplied.
	FCoreMorphTailMotion Late;
	Advance(Late, FVector(11000, 0, 0), 1080, 1.f / 60.f);
	Advance(Late, FVector(11000, 0, 8000), 120, 1.f / 60.f);
	TestNearlyEqual(TEXT("Climb response is independent of flight schedule"), Late.Climb, Climbing.Climb, .0001f);
	const float BeforeLevel = Climbing.Climb;
	Climbing.Update(FVector(11000, 0, 0), 1.f / 60.f);
	TestTrue(TEXT("Crest transition fades without snapping"), Climbing.Climb < BeforeLevel && Climbing.Climb > BeforeLevel * .9f);
	Advance(Climbing, FVector(11000, 0, -8000), 120, 1.f / 60.f);
	TestTrue(TEXT("Sustained descent clears climb response"), Climbing.Climb < .002f);
	FCoreMorphTailMotion At30, At120;
	Advance(At30, FVector(11000, 0, 4000), 60, 1.f / 30.f);
	Advance(At120, FVector(11000, 0, 4000), 240, 1.f / 120.f);
	FVector Tip30 = FVector::ZeroVector, Tip120 = FVector::ZeroVector;
	FRotator Bend30 = FRotator::ZeroRotator, Bend120 = FRotator::ZeroRotator;
	At30.Apply(1, Tip30, Bend30); At120.Apply(1, Tip120, Bend120);
	TestTrue(TEXT("30 and 120 FPS give the same response and pose"), Tip30.Equals(Tip120, .02) && Bend30.Equals(Bend120, .01));
	const FCoreMorphTailMotion BeforeZeroDelta = At30;
	At30.Update(FVector(0, 0, 8000), 0);
	TestTrue(TEXT("Zero delta preserves response and phase"), At30.Climb == BeforeZeroDelta.Climb && At30.Phase == BeforeZeroDelta.Phase);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphPlacement, "TheManTest.Enemy.CoreMorph.EditorPlacement", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCoreMorphPlacement::RunTest(const FString& Parameters)
{
	// AutomationOpenMap also starts PIE; placement/duplication must run in editor mode.
	if (!FEditorFileUtils::LoadMap(FPaths::ProjectContentDir() / TEXT("Maps/CoreMorph/L_CoreMorphFlight.umap"), false, false)) return false;
	UWorld* World = GEditor->GetEditorWorldContext().World();
	UClass* Class = LoadClass<ACoreMorphBoss>(nullptr, TEXT("/Game/Enemy/Boss/CoreMorph/Blueprint/BP_CoreMorphBoss.BP_CoreMorphBoss_C"));
	if (!TestNotNull(TEXT("Editor world"), World) || !TestNotNull(TEXT("Final boss Blueprint"), Class)) return false;
	FActorSpawnParameters Spawn;
	Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform Placement(FRotator(0, 37, 0), FVector(800, -2500, 3600), FVector(1.25));
	auto* Boss = World->SpawnActor<ACoreMorphBoss>(Class, Placement, Spawn);
	if (!TestNotNull(TEXT("Editor-placed boss"), Boss)) return false;
	auto Check = [this](ACoreMorphBoss* Actor, const FTransform& Expected)
	{
		TestTrue(FString::Printf(TEXT("Placement root unchanged: actual %s; expected %s"), *Actor->GetActorTransform().ToString(), *Expected.ToString()), Actor->GetActorTransform().Equals(Expected, .001));
		const auto& Pieces = Actor->Flight->GetPieces();
		TestEqual(TEXT("Editor has exactly 154 visual pieces"), Pieces.Num(), 154);
		TestEqual(TEXT("No stale components after construction"), TInlineComponentArray<UStaticMeshComponent*>(Actor).Num(), 154);
		if (!Actor->VisualLayout || Pieces.Num() != Actor->VisualLayout->Pieces.Num()) return;
		double MaxError = 0;
		for (int32 I = 0; I < Pieces.Num(); ++I)
		{
			const FVector Position = Expected.TransformPosition(Actor->VisualLayout->Pieces[I].Position * 1.5);
			MaxError = FMath::Max(MaxError, FVector::Dist(Pieces[I]->GetComponentLocation(), Position));
			TestFalse(TEXT("Editor visual follows parent translation"), Pieces[I]->IsUsingAbsoluteLocation());
		}
		TestTrue(FString::Printf(TEXT("All rest pieces follow the placed body (max error %.6f cm)"), MaxError), MaxError < .01);
	};
	Check(Boss, Placement);
	const FTransform Moved(FRotator(0, -81, 0), FVector(-2200, 1300, 4200), FVector(.8));
	Boss->SetActorTransform(Moved); // The gizmo must move the visuals even before construction reruns.
	Check(Boss, Moved);
	Boss->PostEditMove(true);
	for (int32 I = 0; I < 3; ++I) { Boss->RerunConstructionScripts(); Check(Boss, Moved); }
	const FVector DuplicateOffset(2500, 0, 0);
	auto* Duplicate = Cast<ACoreMorphBoss>(GEditor->GetEditorSubsystem<UEditorActorSubsystem>()->DuplicateActor(Boss, World, DuplicateOffset));
	FTransform DuplicatePlacement = Moved;
	DuplicatePlacement.AddToTranslation(DuplicateOffset);
	if (TestNotNull(TEXT("Duplicated editor instance"), Duplicate)) { Check(Duplicate, DuplicatePlacement); World->DestroyActor(Duplicate); }
	World->DestroyActor(Boss);
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
