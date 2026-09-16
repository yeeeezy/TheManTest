#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPresentationWidgetBase.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class UImage;
class UHorizontalBox;
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
	TObjectPtr<UHorizontalBox> WeaponChoices;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> CharacterDetailsPanel;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CharacterName;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_CharacterDescription;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_MaintenanceWorker;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Executive;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_MaintenanceWorkerChoice;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_ExecutiveChoice;
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
	void ShowMenu();
	UFUNCTION()
	void SelectMaintenanceWorker();
	UFUNCTION()
	void SelectExecutive();
	void SelectCharacter(int32 Index);
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
	bool bShowingCharacterDetails = false;
	bool bNeedsRefresh = true;
	int32 LastCharacterIndex = INDEX_NONE;
};

