#pragma once

#include "Components/ActorComponent.h"
#include "Core/Persistence/WorldPersistenceTypes.h"
#include "PersistentStateComponent.generated.h"

UCLASS(ClassGroup=(Persistence), meta=(BlueprintSpawnableComponent))
class THEMANTEST_API UPersistentStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPersistentStateComponent();

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Persistence")
	EPersistencePolicy PersistencePolicy = EPersistencePolicy::AcrossRounds;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Persistence")
	FGuid PersistentId;

	UFUNCTION(BlueprintPure, Category="Persistence")
	bool IsPersistentAcrossRounds() const { return PersistencePolicy == EPersistencePolicy::AcrossRounds; }

	UFUNCTION(BlueprintCallable, Category="Persistence")
	void MarkPersistentlyDestroyed();

	void SetPersistentIdForRestore(const FGuid& InPersistentId);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;

#if WITH_EDITOR
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
#endif

private:
	void EnsurePersistentId();
};
