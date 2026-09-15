#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPresentationWidgetBase.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class ALobbyCharacterBase;

/** Presentation navigation only; character selection/start-game remains separate. */
UCLASS(Abstract)
class THEMANTEST_API ULobbyPresentationWidgetBase : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> PresentationMenu;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> WeaponDetailsPanel;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponName;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_WeaponDescription;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Back;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Character;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Weapon;
	UFUNCTION()
	void ShowCharacter();
	UFUNCTION()
	void ShowWeapon();
	void ConfigureButtonStyles();
	void RefreshPresentation();
	TWeakObjectPtr<ALobbyCharacterBase> DisplayCharacter;
	int32 LastWeaponIndex = INDEX_NONE;
	bool bShowingWeaponDetails = false;
};

