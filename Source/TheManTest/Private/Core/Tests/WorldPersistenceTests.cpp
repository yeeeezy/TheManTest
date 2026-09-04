#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Core/Persistence/PersistentActorInterface.h"
#include "Core/Persistence/PersistentStateComponent.h"
#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Actors/Persistence/WorldPersistenceTestDoor.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPersistenceDoorTest,
	"TheManTest.Core.Persistence.DoorLifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPersistenceDoorTest::RunTest(const FString& Parameters)
{
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	TestNotNull(TEXT("Editor world exists"), World);
	if (!World)
	{
		return false;
	}

	AWorldPersistenceTestDoor* Door = World->SpawnActor<AWorldPersistenceTestDoor>();
	TestNotNull(TEXT("Test door spawns"), Door);
	if (!Door)
	{
		return false;
	}

	TestEqual(TEXT("Persistence policy defaults to AcrossRounds"),
		Door->PersistentState->PersistencePolicy, EPersistencePolicy::AcrossRounds);
	TestTrue(TEXT("Door receives a valid persistent GUID"), Door->PersistentState->PersistentId.IsValid());

	Door->SetActorTransform(FTransform(FRotator(0.f, 25.f, 0.f), FVector(120.f, 230.f, 340.f)));
	Door->SetDoorOpen(true);
	FInstancedStruct Captured;
	IPersistentActorInterface* DoorInterface = Cast<IPersistentActorInterface>(Door);
	TestNotNull(TEXT("Door implements the persistence interface"), DoorInterface);
	if (DoorInterface)
	{
		DoorInterface->CapturePersistentState_Implementation(Captured);
	}
	const FWorldPersistenceTestDoorState* CapturedDoorState = Captured.GetPtr<FWorldPersistenceTestDoorState>();
	TestNotNull(TEXT("Door returns its typed persistent state"), CapturedDoorState);
	if (CapturedDoorState)
	{
		TestTrue(TEXT("Captured door state records open"), CapturedDoorState->bIsOpen);
	}

	Door->SetDoorOpen(false);
	if (DoorInterface)
	{
		DoorInterface->ApplyPersistentState_Implementation(Captured);
	}
	TestTrue(TEXT("Applying state restores the open door"), Door->IsDoorOpen());

	Door->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPersistencePlacedDoorTest,
	"TheManTest.Core.Persistence.PlacedDoorAsset",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPersistencePlacedDoorTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	TestNotNull(TEXT("TestMap opens"), World);
	if (!World)
	{
		return false;
	}

	AWorldPersistenceTestDoor* AcceptanceDoor = nullptr;
	for (TActorIterator<AWorldPersistenceTestDoor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == TEXT("PersistenceAcceptanceDoor"))
		{
			AcceptanceDoor = *It;
			break;
		}
	}
	TestNotNull(TEXT("TestMap contains PersistenceAcceptanceDoor"), AcceptanceDoor);
	if (AcceptanceDoor)
	{
		TestTrue(TEXT("Placed acceptance door has a serialized persistent GUID"),
			AcceptanceDoor->PersistentState->PersistentId.IsValid());
		TestEqual(TEXT("Placed acceptance door defaults to AcrossRounds"),
			AcceptanceDoor->PersistentState->PersistencePolicy, EPersistencePolicy::AcrossRounds);
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FValidateWorldPersistenceSubsystemCommand, FAutomationTestBase*, Test);

bool FValidateWorldPersistenceSubsystemCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	if (!World || !World->GetGameInstance())
	{
		return false;
	}

	UWorldPersistenceSubsystem* Subsystem =
		World->GetGameInstance()->GetSubsystem<UWorldPersistenceSubsystem>();
	Test->TestNotNull(TEXT("GameInstance owns the persistence subsystem"), Subsystem);
	if (!Subsystem)
	{
		return true;
	}

	const FTransform SavedTransform(FRotator(0.f, 37.f, 0.f), FVector(321.f, 654.f, 987.f));
	AWorldPersistenceTestDoor* Door = World->SpawnActor<AWorldPersistenceTestDoor>(
		AWorldPersistenceTestDoor::StaticClass(), SavedTransform);
	Test->TestNotNull(TEXT("Runtime test door spawns in PIE"), Door);
	if (!Door)
	{
		return true;
	}

	Door->SetDoorOpen(true);
	Subsystem->CaptureWorldState(World);
	const FPersistentMapState* MapState = Subsystem->FindMapStateForTesting(World);
	Test->TestNotNull(TEXT("Subsystem creates a map state"), MapState);
	const FPersistentActorState* SavedState = MapState
		? MapState->ActorStates.Find(Door->PersistentState->PersistentId) : nullptr;
	Test->TestNotNull(TEXT("Registered component is captured without a world scan"), SavedState);
	if (SavedState)
	{
		Test->TestTrue(TEXT("Runtime actor is identified for reconstruction"), SavedState->bRuntimeSpawned);
		Test->TestTrue(TEXT("Snapshot always contains the actor transform"),
			SavedState->Transform.Equals(SavedTransform));
	}

	Door->SetActorTransform(FTransform::Identity);
	Door->SetDoorOpen(false);
	Subsystem->RestoreWorldState(World);
	Test->TestTrue(TEXT("Subsystem restores the door business state"), Door->IsDoorOpen());
	Test->TestTrue(TEXT("Subsystem restores the door transform"),
		Door->GetActorTransform().Equals(SavedTransform));

	const FGuid RuntimeDoorId = Door->PersistentState->PersistentId;
	Door->Destroy();
	Subsystem->RestoreWorldState(World);
	AWorldPersistenceTestDoor* ReconstructedDoor = nullptr;
	for (TActorIterator<AWorldPersistenceTestDoor> It(World); It; ++It)
	{
		if (It->PersistentState->PersistentId == RuntimeDoorId)
		{
			ReconstructedDoor = *It;
			break;
		}
	}
	Test->TestNotNull(TEXT("Missing runtime actor is reconstructed from its snapshot"), ReconstructedDoor);
	if (!ReconstructedDoor)
	{
		return true;
	}
	Test->TestTrue(TEXT("Reconstructed runtime actor keeps its GUID"),
		ReconstructedDoor->PersistentState->PersistentId == RuntimeDoorId);
	Test->TestTrue(TEXT("Reconstructed runtime actor restores business state"),
		ReconstructedDoor->IsDoorOpen());

	ReconstructedDoor->PersistentState->MarkPersistentlyDestroyed();
	MapState = Subsystem->FindMapStateForTesting(World);
	SavedState = MapState ? MapState->ActorStates.Find(RuntimeDoorId) : nullptr;
	Test->TestTrue(TEXT("Explicit Gameplay destruction writes a tombstone"),
		SavedState && !SavedState->bExists);
	ReconstructedDoor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPersistenceSubsystemPIETest,
	"TheManTest.Core.Persistence.SubsystemPIE",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPersistenceSubsystemPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateWorldPersistenceSubsystemCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
