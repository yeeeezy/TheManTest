#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPresentationWidgetBase.generated.h"

class UButton;

/** Presentation navigation only; character selection/start-game remains separate. */
UCLASS(Abstract)
class THEMANTEST_API ULobbyPresentationWidgetBase : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Character;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Weapon;
	UFUNCTION()
	void ShowCharacter();
	UFUNCTION()
	void ShowWeapon();
	void ApplySelection(bool bWeapon);
};

