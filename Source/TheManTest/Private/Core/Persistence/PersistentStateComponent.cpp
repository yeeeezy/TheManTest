#include "Core/Persistence/PersistentStateComponent.h"
#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UPersistentStateComponent::UPersistentStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;
}

void UPersistentStateComponent::OnRegister()
{
	Super::OnRegister();
	EnsurePersistentId();
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (UWorldPersistenceSubsystem* Subsystem =
				GameInstance->GetSubsystem<UWorldPersistenceSubsystem>())
			{
				Subsystem->RegisterComponent(this);
			}
		}
	}
}

void UPersistentStateComponent::OnUnregister()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UWorldPersistenceSubsystem* Subsystem =
			GameInstance->GetSubsystem<UWorldPersistenceSubsystem>())
		{
			Subsystem->UnregisterComponent(this);
		}
	}
	Super::OnUnregister();
}

void UPersistentStateComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UWorldPersistenceSubsystem* Subsystem =
			GameInstance->GetSubsystem<UWorldPersistenceSubsystem>())
		{
			Subsystem->RestoreRegisteredComponent(this);
		}
	}
}

void UPersistentStateComponent::MarkPersistentlyDestroyed()
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		GameInstance->GetSubsystem<UWorldPersistenceSubsystem>()->RecordPersistentDestruction(this);
	}
}

void UPersistentStateComponent::SetPersistentIdForRestore(const FGuid& InPersistentId)
{
	PersistentId = InPersistentId;
}

void UPersistentStateComponent::EnsurePersistentId()
{
	if (PersistentId.IsValid() || HasAnyFlags(RF_ClassDefaultObject))
	{
		return;
	}

	PersistentId = FGuid::NewGuid();
#if WITH_EDITOR
	if (AActor* Owner = GetOwner(); Owner && !Owner->GetWorld()->IsGameWorld())
	{
		Owner->Modify();
		Owner->MarkPackageDirty();
	}
#endif
}

#if WITH_EDITOR
void UPersistentStateComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	if (!bDuplicateForPIE && !HasAnyFlags(RF_ClassDefaultObject))
	{
		PersistentId = FGuid::NewGuid();
		if (AActor* Owner = GetOwner())
		{
			Owner->Modify();
			Owner->MarkPackageDirty();
		}
	}
}
#endif
