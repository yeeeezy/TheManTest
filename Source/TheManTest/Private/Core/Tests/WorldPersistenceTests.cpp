#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR

#include "Misc/AutomationTest.h"
#include "Core/Persistence/PersistentActorInterface.h"
#include "Core/Persistence/PersistentStateComponent.h"
#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Core/TheManGameInstance.h"
#include "Actors/Persistence/WorldPersistenceTestDoor.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "Misc/PackageName.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
FString GetPersistenceTestMapName(const UWorld* World)
{
	return World
		? FPackageName::GetShortName(UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()))
		: FString();
}

AWorldPersistenceTestDoor* FindPersistenceAcceptanceDoor(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AWorldPersistenceTestDoor> It(World); It; ++It)
	{
		if (It->GetActorLabel() == TEXT("PersistenceAcceptanceDoor"))
		{
			return *It;
		}
	}
	return nullptr;
}

struct FWorldPersistenceTravelContext
{
	FGuid PersistentId;
	FTransform SavedTransform = FTransform::Identity;
	FGuid RuntimePersistentId;
	FTransform RuntimeSavedTransform = FTransform::Identity;
	double LobbyDeadline = 0.0;
	double TestMapDeadline = 0.0;
	bool bAbort = false;
};
}

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

	AWorldPersistenceTestDoor* AcceptanceDoor = FindPersistenceAcceptanceDoor(World);
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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FStartWorldPersistenceTravelCommand,
	FAutomationTestBase*, Test,
	TSharedPtr<FWorldPersistenceTravelContext>, Context);

bool FStartWorldPersistenceTravelCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	AWorldPersistenceTestDoor* Door = FindPersistenceAcceptanceDoor(World);
	UTheManGameInstance* GameInstance = World
		? Cast<UTheManGameInstance>(World->GetGameInstance())
		: nullptr;
	UWorldPersistenceSubsystem* Subsystem = GameInstance
		? GameInstance->GetSubsystem<UWorldPersistenceSubsystem>()
		: nullptr;

	Test->TestNotNull(TEXT("Travel test starts in a PIE world"), World);
	Test->TestNotNull(TEXT("Travel test finds the placed acceptance door"), Door);
	Test->TestNotNull(TEXT("Travel test uses UTheManGameInstance"), GameInstance);
	Test->TestNotNull(TEXT("Travel test finds the persistence subsystem"), Subsystem);
	if (!World || !Door || !GameInstance || !Subsystem)
	{
		Context->bAbort = true;
		return true;
	}

	Subsystem->ResetAllPersistentState();
	Context->PersistentId = Door->PersistentState->PersistentId;
	Context->SavedTransform = FTransform(
		FRotator(0.f, 41.f, 0.f),
		FVector(745.f, 315.f, 125.f));
	Door->SetActorTransform(Context->SavedTransform, false, nullptr, ETeleportType::TeleportPhysics);
	Door->SetDoorOpen(true);

	Context->RuntimeSavedTransform = FTransform(
		FRotator(0.f, -28.f, 0.f),
		FVector(980.f, -420.f, 150.f));
	AWorldPersistenceTestDoor* RuntimeDoor = World->SpawnActor<AWorldPersistenceTestDoor>(
		AWorldPersistenceTestDoor::StaticClass(), Context->RuntimeSavedTransform);
	Test->TestNotNull(TEXT("Travel test creates a runtime persistent door"), RuntimeDoor);
	if (!RuntimeDoor)
	{
		Context->bAbort = true;
		return true;
	}
	RuntimeDoor->SetDoorOpen(true);
	Context->RuntimePersistentId = RuntimeDoor->PersistentState->PersistentId;

	Context->LobbyDeadline = FPlatformTime::Seconds() + 30.0;
	GameInstance->HandlePlayerDeath(1);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FReturnToPersistenceTestMapCommand,
	FAutomationTestBase*, Test,
	TSharedPtr<FWorldPersistenceTravelContext>, Context);

bool FReturnToPersistenceTestMapCommand::Update()
{
	if (Context->bAbort)
	{
		return true;
	}

	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	if (!World || GetPersistenceTestMapName(World) != TEXT("LobbyMap"))
	{
		if (FPlatformTime::Seconds() >= Context->LobbyDeadline)
		{
			Test->AddError(TEXT("Timed out waiting for PIE to enter LobbyMap."));
			Context->bAbort = true;
			return true;
		}
		return false;
	}

	UTheManGameInstance* GameInstance = Cast<UTheManGameInstance>(World->GetGameInstance());
	Test->TestNotNull(TEXT("GameInstance survives travel to LobbyMap"), GameInstance);
	if (!GameInstance)
	{
		Context->bAbort = true;
		return true;
	}

	GameInstance->SelectCharacterAndStart(FName(TEXT("MaintenanceWorker")));
	Context->TestMapDeadline = FPlatformTime::Seconds() + 30.0;
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FValidateWorldPersistenceTravelCommand,
	FAutomationTestBase*, Test,
	TSharedPtr<FWorldPersistenceTravelContext>, Context);

bool FValidateWorldPersistenceTravelCommand::Update()
{
	if (Context->bAbort)
	{
		return true;
	}

	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	if (!World || GetPersistenceTestMapName(World) != TEXT("TestMap"))
	{
		if (FPlatformTime::Seconds() >= Context->TestMapDeadline)
		{
			Test->AddError(TEXT("Timed out waiting for PIE to return to TestMap."));
			Context->bAbort = true;
			return true;
		}
		return false;
	}

	AWorldPersistenceTestDoor* RestoredDoor = nullptr;
	AWorldPersistenceTestDoor* RestoredRuntimeDoor = nullptr;
	for (TActorIterator<AWorldPersistenceTestDoor> It(World); It; ++It)
	{
		if (It->PersistentState->PersistentId == Context->PersistentId)
		{
			RestoredDoor = *It;
		}
		else if (It->PersistentState->PersistentId == Context->RuntimePersistentId)
		{
			RestoredRuntimeDoor = *It;
		}
	}

	Test->TestNotNull(TEXT("Placed door is matched by its serialized GUID after travel"), RestoredDoor);
	if (!RestoredDoor)
	{
		return true;
	}

	Test->TestTrue(TEXT("Door business state is restored across real map travel"),
		RestoredDoor->IsDoorOpen());
	Test->TestTrue(TEXT("Door transform is restored across real map travel"),
		RestoredDoor->GetActorTransform().Equals(Context->SavedTransform));
	Test->TestTrue(TEXT("Door observes restored business state inside BeginPlay"),
		RestoredDoor->WasOpenAtBeginPlay());
	Test->TestTrue(TEXT("Door observes restored transform inside BeginPlay"),
		RestoredDoor->GetTransformAtBeginPlay().Equals(Context->SavedTransform));

	Test->TestNotNull(TEXT("Runtime door is reconstructed after real map travel"), RestoredRuntimeDoor);
	if (RestoredRuntimeDoor)
	{
		Test->TestTrue(TEXT("Reconstructed runtime door keeps its GUID"),
			RestoredRuntimeDoor->PersistentState->PersistentId == Context->RuntimePersistentId);
		Test->TestTrue(TEXT("Reconstructed runtime door restores business state"),
			RestoredRuntimeDoor->IsDoorOpen());
		Test->TestTrue(TEXT("Reconstructed runtime door restores transform"),
			RestoredRuntimeDoor->GetActorTransform().Equals(Context->RuntimeSavedTransform));
		Test->TestTrue(TEXT("Reconstructed runtime door sees restored state inside BeginPlay"),
			RestoredRuntimeDoor->WasOpenAtBeginPlay());
		Test->TestTrue(TEXT("Reconstructed runtime door sees restored transform inside BeginPlay"),
			RestoredRuntimeDoor->GetTransformAtBeginPlay().Equals(Context->RuntimeSavedTransform));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPersistenceTravelBeforeBeginPlayTest,
	"TheManTest.Core.Persistence.WorldTravelBeforeBeginPlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPersistenceTravelBeforeBeginPlayTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	TSharedPtr<FWorldPersistenceTravelContext> Context = MakeShared<FWorldPersistenceTravelContext>();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FStartWorldPersistenceTravelCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FReturnToPersistenceTestMapCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateWorldPersistenceTravelCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FStartWorldPersistenceTombstoneTravelCommand,
	FAutomationTestBase*, Test,
	TSharedPtr<FWorldPersistenceTravelContext>, Context);

bool FStartWorldPersistenceTombstoneTravelCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	AWorldPersistenceTestDoor* Door = FindPersistenceAcceptanceDoor(World);
	UTheManGameInstance* GameInstance = World
		? Cast<UTheManGameInstance>(World->GetGameInstance())
		: nullptr;
	UWorldPersistenceSubsystem* Subsystem = GameInstance
		? GameInstance->GetSubsystem<UWorldPersistenceSubsystem>()
		: nullptr;

	Test->TestNotNull(TEXT("Tombstone travel test starts in a PIE world"), World);
	Test->TestNotNull(TEXT("Tombstone travel test finds the placed acceptance door"), Door);
	Test->TestNotNull(TEXT("Tombstone travel test uses UTheManGameInstance"), GameInstance);
	Test->TestNotNull(TEXT("Tombstone travel test finds the persistence subsystem"), Subsystem);
	if (!World || !Door || !GameInstance || !Subsystem)
	{
		Context->bAbort = true;
		return true;
	}

	Subsystem->ResetAllPersistentState();
	Context->PersistentId = Door->PersistentState->PersistentId;
	AWorldPersistenceTestDoor::ResetBeginPlayHistoryForTests();
	Door->PersistentState->MarkPersistentlyDestroyed();
	Door->Destroy();

	Context->LobbyDeadline = FPlatformTime::Seconds() + 30.0;
	GameInstance->HandlePlayerDeath(1);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
	FValidateWorldPersistenceTombstoneTravelCommand,
	FAutomationTestBase*, Test,
	TSharedPtr<FWorldPersistenceTravelContext>, Context);

bool FValidateWorldPersistenceTombstoneTravelCommand::Update()
{
	if (Context->bAbort)
	{
		return true;
	}

	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	if (!World || GetPersistenceTestMapName(World) != TEXT("TestMap"))
	{
		if (FPlatformTime::Seconds() >= Context->TestMapDeadline)
		{
			Test->AddError(TEXT("Timed out waiting for tombstone test to return to TestMap."));
			Context->bAbort = true;
			return true;
		}
		return false;
	}

	AWorldPersistenceTestDoor* TombstonedDoor = nullptr;
	for (TActorIterator<AWorldPersistenceTestDoor> It(World); It; ++It)
	{
		if (It->PersistentState->PersistentId == Context->PersistentId)
		{
			TombstonedDoor = *It;
			break;
		}
	}

	Test->TestNull(TEXT("Tombstoned placed door is removed after real map travel"), TombstonedDoor);
	Test->TestFalse(TEXT("Tombstoned placed door never reaches BeginPlay"),
		AWorldPersistenceTestDoor::DidPersistentIdBeginPlayForTests(Context->PersistentId));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWorldPersistenceTombstoneBeforeBeginPlayTest,
	"TheManTest.Core.Persistence.TombstoneBeforeBeginPlay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWorldPersistenceTombstoneBeforeBeginPlayTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	TSharedPtr<FWorldPersistenceTravelContext> Context = MakeShared<FWorldPersistenceTravelContext>();
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FStartWorldPersistenceTombstoneTravelCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FReturnToPersistenceTestMapCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateWorldPersistenceTombstoneTravelCommand(this, Context));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
