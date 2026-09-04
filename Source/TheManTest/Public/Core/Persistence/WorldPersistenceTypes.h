#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "WorldPersistenceTypes.generated.h"

UENUM(BlueprintType)
enum class EPersistencePolicy : uint8
{
	None,
	AcrossRounds
};

USTRUCT(BlueprintType)
struct THEMANTEST_API FPersistentActorState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 DataVersion = 1;

	UPROPERTY(SaveGame)
	FGuid PersistentId;

	UPROPERTY(SaveGame)
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(SaveGame)
	FTransform Transform = FTransform::Identity;

	UPROPERTY(SaveGame)
	bool bExists = true;

	UPROPERTY(SaveGame)
	bool bRuntimeSpawned = false;

	UPROPERTY(SaveGame)
	FInstancedStruct CustomData;
};

USTRUCT(BlueprintType)
struct THEMANTEST_API FPersistentMapState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 DataVersion = 1;

	UPROPERTY(SaveGame)
	TMap<FGuid, FPersistentActorState> ActorStates;
};
