#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Core/Persistence/PersistentActorInterface.h"
#include "Core/Persistence/PersistentStateComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

void UWorldPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	WorldInitializedActorsHandle = FWorldDelegates::OnWorldInitializedActors.AddUObject(
		this, &UWorldPersistenceSubsystem::HandleWorldInitializedActors);
}

void UWorldPersistenceSubsystem::Deinitialize()
{
	FWorldDelegates::OnWorldInitializedActors.Remove(WorldInitializedActorsHandle);
	RegisteredComponents.Reset();
	RestoredComponents.Reset();
	WorldsPendingPreBeginPlay.Reset();
	StatesByMap.Reset();
	Super::Deinitialize();
}

void UWorldPersistenceSubsystem::RegisterComponent(UPersistentStateComponent* Component)
{
	if (IsValid(Component) && Component->GetWorld()
		&& Component->GetWorld()->GetGameInstance() == GetGameInstance())
	{
		RegisteredComponents.Add(Component);
	}
}

void UWorldPersistenceSubsystem::UnregisterComponent(UPersistentStateComponent* Component)
{
	RegisteredComponents.Remove(Component);
	RestoredComponents.Remove(Component);
}

void UWorldPersistenceSubsystem::RestoreRegisteredComponent(UPersistentStateComponent* Component)
{
	if (!IsValid(Component) || !Component->IsPersistentAcrossRounds()
		|| !Component->PersistentId.IsValid() || RestoredComponents.Contains(Component))
	{
		return;
	}

	UWorld* World = Component->GetWorld();
	AActor* Owner = Component->GetOwner();
	if (!World || World->GetGameInstance() != GetGameInstance() || !IsValid(Owner))
	{
		return;
	}

	const FPersistentMapState* MapState = StatesByMap.Find(GetStableMapId(World));
	const FPersistentActorState* State = MapState
		? MapState->ActorStates.Find(Component->PersistentId)
		: nullptr;
	if (!State)
	{
		return;
	}

	if (!State->bExists)
	{
		RestoredComponents.Add(Component);
		Owner->Destroy();
		return;
	}

	RestoreComponent(Component, *State);
}

FName UWorldPersistenceSubsystem::GetStableMapId(const UWorld* World) const
{
	if (!World)
	{
		return NAME_None;
	}
	return FName(*UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()));
}

void UWorldPersistenceSubsystem::CaptureWorldState(UWorld* World)
{
	if (!World || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	const FName MapId = GetStableMapId(World);
	if (MapId.IsNone())
	{
		return;
	}

	FPersistentMapState& MapState = StatesByMap.FindOrAdd(MapId);
	TSet<FGuid> SeenIds;
	for (auto It = RegisteredComponents.CreateIterator(); It; ++It)
	{
		UPersistentStateComponent* Component = It->Get();
		if (!IsValid(Component))
		{
			It.RemoveCurrent();
			continue;
		}

		AActor* Owner = Component->GetOwner();
		if (!IsValid(Owner) || Owner->GetWorld() != World)
		{
			continue;
		}

		if (!Component->IsPersistentAcrossRounds())
		{
			MapState.ActorStates.Remove(Component->PersistentId);
			continue;
		}

		if (!Component->PersistentId.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("[WorldPersistence] %s has an invalid PersistentId."), *Owner->GetPathName());
			continue;
		}
		if (SeenIds.Contains(Component->PersistentId))
		{
			UE_LOG(LogTemp, Error, TEXT("[WorldPersistence] Duplicate PersistentId %s in %s; duplicate skipped."),
				*Component->PersistentId.ToString(), *MapId.ToString());
			continue;
		}
		SeenIds.Add(Component->PersistentId);

		FPersistentActorState& State = MapState.ActorStates.FindOrAdd(Component->PersistentId);
		State.PersistentId = Component->PersistentId;
		State.ActorClass = Owner->GetClass();
		State.Transform = Owner->GetActorTransform();
		State.bExists = true;
		State.bRuntimeSpawned = !Owner->IsNetStartupActor();
		State.CustomData.Reset();
		if (Owner->GetClass()->ImplementsInterface(UPersistentActorInterface::StaticClass()))
		{
			if (Owner->GetClass()->IsFunctionImplementedInScript(TEXT("CapturePersistentState")))
			{
				IPersistentActorInterface::Execute_CapturePersistentState(Owner, State.CustomData);
			}
			else if (const IPersistentActorInterface* NativeInterface = Cast<IPersistentActorInterface>(Owner))
			{
				NativeInterface->CapturePersistentState_Implementation(State.CustomData);
			}
		}
	}
}

void UWorldPersistenceSubsystem::RecordPersistentDestruction(UPersistentStateComponent* Component)
{
	if (!IsValid(Component) || !Component->IsPersistentAcrossRounds() || !Component->PersistentId.IsValid())
	{
		return;
	}

	AActor* Owner = Component->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	FPersistentActorState& State = StatesByMap.FindOrAdd(GetStableMapId(Owner->GetWorld()))
		.ActorStates.FindOrAdd(Component->PersistentId);
	State.PersistentId = Component->PersistentId;
	State.ActorClass = Owner->GetClass();
	State.Transform = Owner->GetActorTransform();
	State.bRuntimeSpawned = !Owner->IsNetStartupActor();
	State.bExists = false;
}

void UWorldPersistenceSubsystem::RestoreComponent(
	UPersistentStateComponent* Component, const FPersistentActorState& State)
{
	AActor* Owner = Component ? Component->GetOwner() : nullptr;
	if (!IsValid(Owner) || RestoredComponents.Contains(Component))
	{
		return;
	}

	RestoredComponents.Add(Component);
	Owner->SetActorTransform(State.Transform, false, nullptr, ETeleportType::TeleportPhysics);
	if (Owner->GetClass()->ImplementsInterface(UPersistentActorInterface::StaticClass()))
	{
		if (Owner->GetClass()->IsFunctionImplementedInScript(TEXT("ApplyPersistentState")))
		{
			IPersistentActorInterface::Execute_ApplyPersistentState(Owner, State.CustomData);
		}
		else if (IPersistentActorInterface* NativeInterface = Cast<IPersistentActorInterface>(Owner))
		{
			NativeInterface->ApplyPersistentState_Implementation(State.CustomData);
		}
	}
}

void UWorldPersistenceSubsystem::RestoreWorldState(UWorld* World)
{
	if (!World || !World->IsGameWorld() || World->GetGameInstance() != GetGameInstance())
	{
		return;
	}

	const FPersistentMapState* MapState = StatesByMap.Find(GetStableMapId(World));
	if (!MapState)
	{
		return;
	}

	TMap<FGuid, UPersistentStateComponent*> ComponentsById;
	for (auto It = RegisteredComponents.CreateIterator(); It; ++It)
	{
		UPersistentStateComponent* Component = It->Get();
		if (!IsValid(Component))
		{
			It.RemoveCurrent();
			continue;
		}
		if (Component->GetWorld() != World || !Component->IsPersistentAcrossRounds())
		{
			continue;
		}

		if (ComponentsById.Contains(Component->PersistentId))
		{
			UE_LOG(LogTemp, Error, TEXT("[WorldPersistence] Duplicate PersistentId %s in %s."),
				*Component->PersistentId.ToString(), *GetStableMapId(World).ToString());
			continue;
		}
		ComponentsById.Add(Component->PersistentId, Component);
	}

	for (const TPair<FGuid, FPersistentActorState>& Pair : MapState->ActorStates)
	{
		const FPersistentActorState& State = Pair.Value;
		if (UPersistentStateComponent** Existing = ComponentsById.Find(Pair.Key))
		{
			if (!State.bExists)
			{
				(*Existing)->GetOwner()->Destroy();
			}
			else
			{
				RestoreComponent(*Existing, State);
			}
			continue;
		}

		if (!State.bExists || !State.bRuntimeSpawned)
		{
			continue;
		}

		UClass* ActorClass = State.ActorClass.LoadSynchronous();
		if (!ActorClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[WorldPersistence] Cannot load runtime actor class for %s."), *Pair.Key.ToString());
			continue;
		}

		AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(ActorClass, State.Transform);
		if (!SpawnedActor)
		{
			continue;
		}
		SpawnedActor->FinishSpawning(State.Transform);

		UPersistentStateComponent* SpawnedComponent = SpawnedActor->FindComponentByClass<UPersistentStateComponent>();
		if (!SpawnedComponent)
		{
			UE_LOG(LogTemp, Error,
				TEXT("[WorldPersistence] Restored runtime actor %s has no PersistentStateComponent."),
				*SpawnedActor->GetPathName());
			SpawnedActor->Destroy();
			continue;
		}
		SpawnedComponent->SetPersistentIdForRestore(State.PersistentId);
		RestoreComponent(SpawnedComponent, State);
	}
}

void UWorldPersistenceSubsystem::HandleWorldInitializedActors(const FActorsInitializedParams& Params)
{
	UWorld* World = Params.World;
	const TWeakObjectPtr<UWorld> WeakWorld(World);
	if (!World || !World->IsGameWorld() || World->GetGameInstance() != GetGameInstance()
		|| World->HasBegunPlay() || WorldsPendingPreBeginPlay.Contains(WeakWorld))
	{
		return;
	}

	WorldsPendingPreBeginPlay.Add(WeakWorld);
	World->OnWorldPreBeginPlay.AddWeakLambda(this, [this, WeakWorld]()
	{
		HandleWorldPreBeginPlay(WeakWorld);
	});
}

void UWorldPersistenceSubsystem::HandleWorldPreBeginPlay(TWeakObjectPtr<UWorld> WeakWorld)
{
	WorldsPendingPreBeginPlay.Remove(WeakWorld);
	UWorld* World = WeakWorld.Get();
	if (World && World->IsGameWorld() && World->GetGameInstance() == GetGameInstance())
	{
		RestoreWorldState(World);
	}
}

void UWorldPersistenceSubsystem::ResetAllPersistentState()
{
	StatesByMap.Reset();
}

#if WITH_DEV_AUTOMATION_TESTS
const FPersistentMapState* UWorldPersistenceSubsystem::FindMapStateForTesting(UWorld* World) const
{
	return StatesByMap.Find(GetStableMapId(World));
}
#endif
