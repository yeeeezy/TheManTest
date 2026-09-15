#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPresentationWidgetBase.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UImage;
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
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_RepairGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ExplosionGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_ElectricGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_RepairGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_ExplosionGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_ElectricGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Corner_RepairGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Corner_ExplosionGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> Corner_ElectricGun;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Character;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Weapon;
	UFUNCTION()
	void ShowCharacter();
	UFUNCTION()
	void ShowWeapon();
	UFUNCTION()
	void SelectRepairGun();
	UFUNCTION()
	void SelectExplosionGun();
	UFUNCTION()
	void SelectElectricGun();
	void SelectWeapon(int32 Index);
	void ConfigureButtonStyles();
	void RefreshPresentation();
	TWeakObjectPtr<ALobbyCharacterBase> DisplayCharacter;
	int32 LastWeaponIndex = INDEX_NONE;
	bool bShowingWeaponDetails = false;
};

