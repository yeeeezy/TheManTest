#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/Persistence/PersistentActorInterface.h"
#include "WorldPersistenceTestDoor.generated.h"

class UPersistentStateComponent;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct THEMANTEST_API FWorldPersistenceTestDoorState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame, BlueprintReadWrite)
	bool bIsOpen = false;
};

UCLASS(Blueprintable)
class THEMANTEST_API AWorldPersistenceTestDoor : public AActor, public IPersistentActorInterface
{
	GENERATED_BODY()

public:
	AWorldPersistenceTestDoor();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="Persistence Test Door")
	void ToggleDoor();

	UFUNCTION(BlueprintCallable, Category="Persistence Test Door")
	void SetDoorOpen(bool bOpen);

	UFUNCTION(BlueprintPure, Category="Persistence Test Door")
	bool IsDoorOpen() const { return bIsOpen; }

	bool WasOpenAtBeginPlay() const { return bWasOpenAtBeginPlay; }
	const FTransform& GetTransformAtBeginPlay() const { return TransformAtBeginPlay; }

#if WITH_DEV_AUTOMATION_TESTS
	static void ResetBeginPlayHistoryForTests();
	static bool DidPersistentIdBeginPlayForTests(const FGuid& PersistentId);
#endif

	virtual void CapturePersistentState_Implementation(FInstancedStruct& OutPersistentState) const override;
	virtual void ApplyPersistentState_Implementation(const FInstancedStruct& PersistentState) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UPersistentStateComponent> PersistentState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> DoorMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> OpenTrigger;

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category="Persistence Test Door")
	bool bIsOpen = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Persistence Test Door")
	float OpenYaw = 90.f;

private:
	bool bWasOpenAtBeginPlay = false;
	FTransform TransformAtBeginPlay = FTransform::Identity;

#if WITH_DEV_AUTOMATION_TESTS
	static TSet<FGuid> BeginPlayIdsForTests;
#endif

	UFUNCTION()
	void HandleTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	void ApplyDoorVisual();
};
