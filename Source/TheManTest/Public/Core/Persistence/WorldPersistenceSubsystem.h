#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/Persistence/WorldPersistenceTypes.h"
#include "WorldPersistenceSubsystem.generated.h"

class UPersistentStateComponent;
struct FActorsInitializedParams;

UCLASS()
class THEMANTEST_API UWorldPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void RegisterComponent(UPersistentStateComponent* Component);
	void UnregisterComponent(UPersistentStateComponent* Component);
	void RestoreRegisteredComponent(UPersistentStateComponent* Component);

	UFUNCTION(BlueprintCallable, Category="Persistence")
	void CaptureWorldState(UWorld* World);

	UFUNCTION(BlueprintCallable, Category="Persistence")
	void RestoreWorldState(UWorld* World);

	UFUNCTION(BlueprintCallable, Category="Persistence")
	void ResetAllPersistentState();

	void RecordPersistentDestruction(UPersistentStateComponent* Component);

#if WITH_DEV_AUTOMATION_TESTS
	const FPersistentMapState* FindMapStateForTesting(UWorld* World) const;
#endif

private:
	FName GetStableMapId(const UWorld* World) const;
	void HandleWorldInitializedActors(const FActorsInitializedParams& Params);
	void HandleWorldPreBeginPlay(TWeakObjectPtr<UWorld> World);
	void RestoreComponent(UPersistentStateComponent* Component, const FPersistentActorState& State);

	TSet<TWeakObjectPtr<UPersistentStateComponent>> RegisteredComponents;
	TSet<TWeakObjectPtr<UPersistentStateComponent>> RestoredComponents;
	TSet<TWeakObjectPtr<UWorld>> WorldsPendingPreBeginPlay;

	UPROPERTY()
	TMap<FName, FPersistentMapState> StatesByMap;

	FDelegateHandle WorldInitializedActorsHandle;
};
