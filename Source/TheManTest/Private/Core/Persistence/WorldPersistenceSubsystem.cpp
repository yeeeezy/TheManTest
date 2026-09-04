#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Core/Persistence/PersistentActorInterface.h"
#include "Core/Persistence/PersistentStateComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

void UWorldPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
		this, &UWorldPersistenceSubsystem::HandlePostLoadMap);
}

void UWorldPersistenceSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	RegisteredComponents.Reset();
	StatesByMap.Reset();
	Super::Deinitialize();
}

void UWorldPersistenceSubsystem::RegisterComponent(UPersistentStateComponent* Component)
{
	if (IsValid(Component))
	{
		RegisteredComponents.Add(Component);
	}
}

void UWorldPersistenceSubsystem::UnregisterComponent(UPersistentStateComponent* Component)
{
	RegisteredComponents.Remove(Component);
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
	if (!IsValid(Owner))
	{
		return;
	}

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
	const FPersistentMapState* MapState = StatesByMap.Find(GetStableMapId(World));
	if (!World || !MapState)
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
		UPersistentStateComponent* SpawnedComponent = SpawnedActor->FindComponentByClass<UPersistentStateComponent>();
		if (!SpawnedComponent)
		{
			SpawnedActor->Destroy();
			continue;
		}
		SpawnedComponent->SetPersistentIdForRestore(State.PersistentId);
		SpawnedActor->FinishSpawning(State.Transform);
		RestoreComponent(SpawnedComponent, State);
	}
}

void UWorldPersistenceSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (LoadedWorld && LoadedWorld->IsGameWorld())
	{
		LoadedWorld->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UWorldPersistenceSubsystem::RestoreWorldState, LoadedWorld));
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
