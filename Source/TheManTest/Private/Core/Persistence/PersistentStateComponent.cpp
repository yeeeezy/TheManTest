#include "Core/Persistence/PersistentStateComponent.h"
#include "Core/Persistence/WorldPersistenceSubsystem.h"
#include "Engine/GameInstance.h"

UPersistentStateComponent::UPersistentStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPersistentStateComponent::OnRegister()
{
	Super::OnRegister();
	EnsurePersistentId();
}

void UPersistentStateComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsurePersistentId();
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		GameInstance->GetSubsystem<UWorldPersistenceSubsystem>()->RegisterComponent(this);
	}
}

void UPersistentStateComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		GameInstance->GetSubsystem<UWorldPersistenceSubsystem>()->UnregisterComponent(this);
	}
	Super::EndPlay(EndPlayReason);
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
