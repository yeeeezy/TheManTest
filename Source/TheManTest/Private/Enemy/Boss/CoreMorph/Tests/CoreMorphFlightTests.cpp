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
#include "Components/SplineComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightRoute.h"
#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
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

void ValidateSpline(ACoreMorphBoss* Boss, FAutomationTestBase* Test)
{
	Boss->ResetFlightPreview();
	const FVector Origin = Boss->GetActorLocation();
	auto* Route = Boss->GetWorld()->SpawnActor<ACoreMorphFlightRoute>();
	if (!Test->TestNotNull(TEXT("Editable route spawned in actual PIE"), Route)) return;
	Boss->Flight->FlightRoute = Route;
	Boss->Flight->RouteSpeed = 9000;
	float FirstBank = 0;
	for (float Side : {1.f, -1.f})
	{
		Route->Spline->SetSplinePoints({Origin, Origin + FVector(10000, 3000 * Side, 6000),
			Origin + FVector(17000, 10000 * Side, 12000), Origin + FVector(17000, 20000 * Side, 2000)}, ESplineCoordinateSpace::World);
		Boss->ResetFlightPreview();
		Test->TestTrue(TEXT("Different route starts through the same GA"), Boss->StartFlightPreview());
		Boss->Flight->SetComponentTickEnabled(false);
		Step(Boss, .08f);
		Test->TestTrue(TEXT("Wings prepare before spline acceleration starts"), Boss->Flight->GetMotionState().PowerStroke > .3f && Boss->GetActorLocation().Equals(Origin, .01));
		Step(Boss, 1.92f);
		const auto& State = Boss->Flight->GetMotionState();
		Test->TestTrue(TEXT("Custom route ascent drives motion"), State.Climb > .15f);
		if (Side > 0) FirstBank = State.Bank;
		else Test->TestTrue(TEXT("Mirroring the route reverses bank automatically"), FirstBank * State.Bank < -1.f);
		Test->TestTrue(TEXT("Route speed builds progressively after the power stroke"), Boss->Flight->GetRouteDistance() > 1000 && Boss->Flight->GetRouteDistance() < 18000);
		const FVector Expected = Route->Spline->GetLocationAtDistanceAlongSpline(float(Boss->Flight->GetRouteDistance()), ESplineCoordinateSpace::World);
		Test->TestTrue(TEXT("Boss follows spline distance instead of source choreography"), Boss->GetActorLocation().Equals(Expected, 5));
		Step(Boss, 8);
		Test->TestFalse(TEXT("Open route completes at its endpoint"), Boss->Flight->IsFlying());
		Test->TestTrue(TEXT("Final body reaches actual spline endpoint"), Boss->GetActorLocation().Equals(Route->Spline->GetLocationAtSplinePoint(3, ESplineCoordinateSpace::World), .1));
	}
	Route->Spline->SetClosedLoop(true);
	Boss->ResetFlightPreview(); Boss->StartFlightPreview(); Boss->Flight->SetComponentTickEnabled(false);
	Step(Boss, 15);
	Test->TestTrue(TEXT("Loop route has no fixed 13.4 second presentation cutoff"), Boss->Flight->IsFlying());
	Test->TestTrue(TEXT("Tail history remains bounded on looping routes"), Boss->Flight->GetMotionState().GetHistoryCount() < 1500);
	Boss->Flight->RouteSpeed = 0;
	const FVector Hover = Boss->GetActorLocation();
	Step(Boss, 2);
	Test->TestTrue(TEXT("Zero route speed hovers without pose errors"), Boss->GetActorLocation().Equals(Hover, .01) && Boss->Flight->GetMotionState().Movement < .01);
	for (const auto& Piece : Boss->Flight->GetPieces()) Test->TestFalse(TEXT("Every runtime piece pose stays finite"), Piece->GetComponentTransform().ContainsNaN());
	Route->Destroy();
	Step(Boss, .1f);
	Test->TestFalse(TEXT("Removed route ends flight safely"), Boss->Flight->IsFlying());
	Test->TestFalse(TEXT("Removed route releases active GA"), Boss->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_CoreMorphFlight::StaticClass())->IsActive());
	Boss->Flight->FlightRoute = nullptr;
	Boss->ResetFlightPreview();
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
	ValidateSpline(Boss, Test);
	Boss->ResetFlightPreview();
	const FVector Initial = Boss->GetActorLocation();
	Test->TestTrue(TEXT("Review map body stays at its pre-fix world placement"), Initial.Equals(FVector(-16000, 0, 2500), .01));
	Test->TestTrue(TEXT("Original world choreography is preserved"), Boss->Flight->GetChoreographyFrame().Equals(FTransform(FVector(0, 0, 900)), .001));
	Test->TestTrue(TEXT("Flight starts through GA"), Boss->StartFlightPreview());
	Boss->Flight->SetComponentTickEnabled(false);
	Test->TestFalse(TEXT("Duplicate flight cannot activate"), Boss->StartFlightPreview());
	Step(Boss, 5.f);
	Test->TestTrue(TEXT("Actual boss identity follows flight by over 50 m"), FVector::Dist(Boss->GetActorLocation(), Initial) > 5000.f);
	Test->TestTrue(TEXT("Measured ascent drives the tail before the old timed pulse"), Boss->Flight->GetMotionState().Climb > .2f);
	const FTransform PausedTail = Boss->Flight->GetPieces().Last()->GetComponentTransform();
	const float PausedPhase = Boss->Flight->GetMotionState().Phase;
	Boss->Flight->TickComponent(.5f, LEVELTICK_All, nullptr);
	Test->TestEqual(TEXT("Pause freezes tail phase"), Boss->Flight->GetMotionState().Phase, PausedPhase);
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
	const float CancelledPhase = Boss->Flight->GetMotionState().Phase;
	Step(Boss, .5f);
	Test->TestTrue(TEXT("Cancellation freezes movement and ends the GA"), !Boss->Flight->IsFlying() && Boss->GetActorLocation().Equals(Cancelled, .01));
	Test->TestEqual(TEXT("Cancellation also freezes tail motion"), Boss->Flight->GetMotionState().Phase, CancelledPhase);
	Boss->ResetFlightPreview();
	Test->TestEqual(TEXT("Reset clears accumulated climb response"), Boss->Flight->GetMotionState().Climb, 0.f);
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
	Test->TestTrue(TEXT("Descent fades the climbing tail response"), Boss->Flight->GetMotionState().Climb < .05f);
	Test->TestFalse(TEXT("No implicit replay after completion"), Boss->StartFlightPreview());
	Boss->ResetFlightPreview();
	Boss->StartFlightPreview();
	Boss->Flight->SetComponentTickEnabled(false);
	Step(Boss, 2.f);
	const float BeforeDeathPhase = Boss->Flight->GetMotionState().Phase;
	Damage(Boss, 1000);
	Boss->Flight->TickComponent(.5f, LEVELTICK_All, nullptr);
	Test->TestEqual(TEXT("Death cannot advance tail motion"), Boss->Flight->GetMotionState().Phase, BeforeDeathPhase);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphAdaptiveMotion, "TheManTest.Enemy.CoreMorph.AdaptiveMotion", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCoreMorphAdaptiveMotion::RunTest(const FString& Parameters)
{
	FCoreMorphVisualPiece Wing, Tail, Core;
	Wing.Kind = TEXT("Wing"); Wing.Position = FVector(0, 2200, 0);
	Tail.Kind = TEXT("Tail"); Tail.Position = FVector(-6000, 0, 0); Tail.Order = 0; // Isolate route trail from the wave.
	Core.Kind = TEXT("Body");
	auto RunStraight = [](FCoreMorphFlightMotion& State, FVector Velocity, int32 Frames, float Dt)
	{
		for (int32 I = 0; I < Frames; ++I) State.Update(State.GetBody().GetLocation() + Velocity * Dt, Dt);
	};
	FCoreMorphFlightMotion Level, Climb, Dive, Late;
	for (auto* State : {&Level, &Climb, &Dive, &Late}) State->Reset(FTransform::Identity, 42, 0, 20000);
	FCoreMorphFlightMotion Prepared;
	Prepared.Reset(FTransform::Identity, 42, 0, 20000);
	Prepared.AccelerationIntent = 1;
	RunStraight(Prepared, FVector::ZeroVector, 24, 1.f / 120.f);
	TestTrue(TEXT("Acceleration intent produces a power stroke before translation"), Prepared.PowerStroke > .8f && Prepared.GetSpeed() == 0);
	TestTrue(TEXT("Preparation visibly moves the wing"), FVector::Dist(Prepared.PiecePose(Wing, FVector::OneVector).GetLocation(), Wing.Position) > 50);
	Prepared.AccelerationIntent = 0;
	RunStraight(Prepared, FVector::ZeroVector, 120, 1.f / 120.f);
	TestTrue(TEXT("Removing acceleration intent releases power stroke"), Prepared.PowerStroke < .001f);
	RunStraight(Level, FVector(11000, 0, 0), 240, 1.f / 120.f);
	RunStraight(Climb, FVector(11000, 0, 8000), 240, 1.f / 120.f);
	RunStraight(Dive, FVector(11000, 0, -11000), 240, 1.f / 120.f);
	RunStraight(Late, FVector(11000, 0, 0), 600, 1.f / 120.f);
	RunStraight(Late, FVector(11000, 0, 8000), 240, 1.f / 120.f);
	TestTrue(TEXT("State distinguishes climb, level and dive"), Climb.Climb > .99f && Level.Climb == 0 && Dive.Climb == 0 && Dive.Dive > .99f);
	TestNearlyEqual(TEXT("Climb activation is independent of route time"), Late.Climb, Climb.Climb, .0001f);
	const FVector WingLevel = Level.GetBody().InverseTransformPosition(Level.PiecePose(Wing, FVector::OneVector).GetLocation());
	const FVector WingDive = Dive.GetBody().InverseTransformPosition(Dive.PiecePose(Wing, FVector::OneVector).GetLocation());
	TestTrue(TEXT("Downward velocity folds the actual wing pose"), WingDive.X < WingLevel.X - 300);
	RunStraight(Dive, FVector(11000, 0, 0), 240, 1.f / 120.f);
	TestTrue(TEXT("Wing unfolds again when flight levels out"), Dive.Dive < .001f);
	TestTrue(TEXT("Rigid core follows the computed body"), Climb.PiecePose(Core, FVector::OneVector).Equals(Climb.GetBody(), .001));
	const FTransform Frozen = Climb.PiecePose(Wing, FVector::OneVector);
	Climb.Update(FVector(100, 200, 300), 0);
	TestTrue(TEXT("Zero delta freezes all state"), Frozen.Equals(Climb.PiecePose(Wing, FVector::OneVector)));
	float FirstBank = 0;
	for (float Side : {1.f, -1.f})
	{
		FCoreMorphFlightMotion Circle;
		Circle.Reset(FTransform::Identity, 42, 0, 20000);
		for (int32 I = 1; I <= 360; ++I)
		{
			const float Angle = I / 120.f;
			Circle.Update(FVector(10000 * FMath::Sin(Angle), Side * 10000 * (1 - FMath::Cos(Angle)), Angle * 1500), 1.f / 120.f);
		}
		if (Side > 0) FirstBank = Circle.Bank;
		else TestTrue(TEXT("Left and right turns bank in opposite directions"), FirstBank * Circle.Bank < -100.f);
		const float TrailAngle = 3.f - 6000.f / FMath::Sqrt(10000.f * 10000.f + 1500.f * 1500.f);
		const FVector Expected(10000 * FMath::Sin(TrailAngle), Side * 10000 * (1 - FMath::Cos(TrailAngle)), TrailAngle * 1500);
		TestTrue(TEXT("Tail follows travelled 3D curvature, not a fixed circle formula"), Circle.PiecePose(Tail, FVector::OneVector).GetLocation().Equals(Expected, 3));
	}
	FCoreMorphFlightMotion RandomA, RandomB, RandomC, DisabledA, DisabledB;
	RandomA.Reset(FTransform::Identity, 42, 1, 20000); RandomB.Reset(FTransform::Identity, 42, 1, 20000);
	RandomC.Reset(FTransform::Identity, 73, 1, 20000);
	DisabledA.Reset(FTransform::Identity, 42, 0, 20000); DisabledB.Reset(FTransform::Identity, 73, 0, 20000);
	for (auto* State : {&RandomA, &RandomB, &RandomC, &DisabledA, &DisabledB}) RunStraight(*State, FVector(11000, 0, 3000), 360, 1.f / 120.f);
	TestTrue(TEXT("Same seed is reproducible"), RandomA.PiecePose(Wing, FVector::OneVector).Equals(RandomB.PiecePose(Wing, FVector::OneVector), .001));
	TestFalse(TEXT("Different seeds vary the actual wing"), RandomA.PiecePose(Wing, FVector::OneVector).Equals(RandomC.PiecePose(Wing, FVector::OneVector), .1));
	TestTrue(TEXT("Zero randomness disables seed differences"), DisabledA.PiecePose(Wing, FVector::OneVector).Equals(DisabledB.PiecePose(Wing, FVector::OneVector), .001));
	const FTransform Before = RandomA.PiecePose(Wing, FVector::OneVector);
	RunStraight(RandomA, FVector(11000, 0, 3000), 1, 1.f / 120.f);
	const FVector BeforeLocal = RandomB.GetBody().InverseTransformPosition(Before.GetLocation());
	const FVector AfterLocal = RandomA.GetBody().InverseTransformPosition(RandomA.PiecePose(Wing, FVector::OneVector).GetLocation());
	TestTrue(TEXT("Random motion is smooth between frames"), FVector::Dist(BeforeLocal, AfterLocal) < 80);
	FCoreMorphFlightMotion At30, At120;
	At30.Reset(FTransform::Identity, 42, .18f, 20000); At120.Reset(FTransform::Identity, 42, .18f, 20000);
	RunStraight(At30, FVector(11000, 0, 4000), 60, 1.f / 30.f); RunStraight(At120, FVector(11000, 0, 4000), 240, 1.f / 120.f);
	TestTrue(TEXT("Frame rates preserve the movement response"), FMath::Abs(At30.Climb - At120.Climb) < .001f && At30.GetBody().Equals(At120.GetBody(), .02));
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

namespace
{
class FCoreMorphReviewRoutes : public IAutomationLatentCommand
{
	FAutomationTestBase* Test;
	int32 Index = 0;
	double Started = 0;
	bool bCaptured = false;
public:
	explicit FCoreMorphReviewRoutes(FAutomationTestBase* In) : Test(In) {}
	bool Update() override
	{
		auto* Boss = FindBoss();
		if (!Boss) { Test->AddError(TEXT("Missing boss in route review PIE")); return true; }
		ACoreMorphFlightReview* Review = nullptr;
		for (TActorIterator<ACoreMorphFlightReview> It(Boss->GetWorld()); It; ++It) { Review = *It; break; }
		if (!Review || Review->Routes.Num() != 3) { Test->AddError(TEXT("Three saved review routes must load")); return true; }
		if (!Started)
		{
			if (Index == 0)
			{
				Test->TestTrue(TEXT("First route starts automatically after PIE initialization"), Boss->Flight->IsFlying());
				Test->TestTrue(TEXT("Review camera references the same boss"), Review->Boss == Boss);
				// Switch while the GA is active, then replay the first route.
				Test->TestTrue(TEXT("Switching an active flight cancels and restarts safely"), Review->SelectRoute(2));
			}
			if (!Test->TestTrue(TEXT("Route selection starts its saved flight"), Review->SelectRoute(Index))) return true;
			Test->TestEqual(TEXT("Switching routes keeps one granted ability"), Boss->GetAbilitySystemComponent()->GetActivatableAbilities().Num(), 1);
			Test->TestEqual(TEXT("Switching routes keeps 154 owned pieces"), Boss->Flight->GetPieces().Num(), 154);
			Started = FPlatformTime::Seconds();
			bCaptured = false;
			return false;
		}
		if (!bCaptured && Boss->Flight->GetFlightSeconds() > 7)
		{
			FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("CoreMorphMigration") /
				FString::Printf(TEXT("RouteReview-%d.png"), Index + 1)), false, false);
			bCaptured = true;
		}
		if (Boss->Flight->IsFlying() && FPlatformTime::Seconds() - Started < 60) return false;
		Test->TestFalse(TEXT("Saved route completes with real world ticking"), Boss->Flight->IsFlying());
		const auto* Spline = Review->Routes[Index].Route->Spline.Get();
		Test->TestTrue(TEXT("Review route reaches its actual endpoint"), Boss->GetActorLocation().Equals(
			Spline->GetLocationAtDistanceAlongSpline(Spline->GetSplineLength(), ESplineCoordinateSpace::World), .1));
		for (const auto& Piece : Boss->Flight->GetPieces())
			Test->TestFalse(TEXT("Review route leaves every mesh pose finite"), Piece->GetComponentTransform().ContainsNaN());
		Test->TestFalse(TEXT("Route endpoint ends its GA"), Boss->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_CoreMorphFlight::StaticClass())->IsActive());
		++Index;
		Started = 0;
		if (Index < 3) return false;
		Test->TestTrue(TEXT("Review can restart after all three routes"), Review->SelectRoute(0));
		EndingBoss = Boss;
		return true;
	}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphRoutesPIE, "TheManTest.Enemy.CoreMorph.RouteReview", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCoreMorphRoutesPIE::RunTest(const FString& Parameters)
{
	if (!FEditorFileUtils::LoadMap(FPaths::ProjectContentDir() / TEXT("Maps/CoreMorph/L_CoreMorphRoutes.umap"), false, false)) return false;
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphReviewRoutes(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.3f));
	ADD_LATENT_AUTOMATION_COMMAND(FCoreMorphAfterExit(this));
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
