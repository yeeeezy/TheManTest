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

	UFUNCTION(BlueprintCallable, Category = "Character Select|Camera")
	void SetWeaponPresentationView(bool bWeapon);

	UFUNCTION(BlueprintPure, Category = "Character Select|Camera")
	bool IsWeaponPresentationView();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	TObjectPtr<UInputMappingContext> CharacterSelectMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	TObjectPtr<UInputAction> ClickAction;

	/** Reuses the project's reserved IA_Test slot for lobby presentation checks in PIE. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	TObjectPtr<UInputAction> TestAction;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Character Select|Camera")
	TObjectPtr<ACharacterSelectCameraSwitcher> CameraSwitcherOverride;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|Input")
	int32 MappingContextPriority = 0;

private:
	UPROPERTY()
	TObjectPtr<ACharacterSelectCameraSwitcher> CachedCameraSwitcher;

	bool bPointerOverUI = false;


#if WITH_EDITOR
	void HandleTestInput();
#endif
	ACharacterSelectCameraSwitcher* GetCameraSwitcher();
};
