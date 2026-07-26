#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CharacterSelectPlayerController.generated.h"

class ACharacterSelectCameraSwitcher;
class UInputAction;
class UInputMappingContext;

UCLASS()
class THEMANTEST_API ACharacterSelectPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Character Select|Input")
	void SetPointerOverUI(bool bInPointerOverUI);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	TObjectPtr<UInputMappingContext> CharacterSelectMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character Select|Camera")
	TObjectPtr<ACharacterSelectCameraSwitcher> CameraSwitcherOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	int32 MappingContextPriority = 0;

private:
	UPROPERTY()
	TObjectPtr<ACharacterSelectCameraSwitcher> CachedCameraSwitcher;

	bool bPointerOverUI = false;

	void HandleClick();
	ACharacterSelectCameraSwitcher* GetCameraSwitcher();
};
