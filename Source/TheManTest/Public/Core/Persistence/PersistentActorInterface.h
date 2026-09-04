#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Interface.h"
#include "PersistentActorInterface.generated.h"

UINTERFACE(BlueprintType)
class THEMANTEST_API UPersistentActorInterface : public UInterface
{
	GENERATED_BODY()
};

class THEMANTEST_API IPersistentActorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Persistence")
	void CapturePersistentState(UPARAM(ref) FInstancedStruct& OutPersistentState) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Persistence")
	void ApplyPersistentState(const FInstancedStruct& PersistentState);
};
