#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Components/Button.h"
#include "Core/CharacterSelectPlayerController.h"
#include "Core/TheManGameInstance.h"
#include "InputCoreTypes.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "EngineUtils.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"

void ULobbyPresentationWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Button_Character) Button_Character->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	// The former WEAPON main-menu entry is now the reserved SETTINGS entry.
	if (Button_Weapon) Button_Weapon->SetIsEnabled(false);
	if (Button_Back) Button_Back->OnClicked.AddUniqueDynamic(this, &ThisClass::GoBack);
	if (Button_NextCharacter) Button_NextCharacter->OnClicked.AddUniqueDynamic(this, &ThisClass::NextCharacter);
	if (Button_ViewWeapons)
	{
		Button_ViewWeapons->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowWeapon);
		Button_ViewWeapons->OnHovered.AddUniqueDynamic(this, &ThisClass::HoverViewWeapons);
		Button_ViewWeapons->OnUnhovered.AddUniqueDynamic(this, &ThisClass::UnhoverViewWeapons);
	}
	if (Button_StartGame) Button_StartGame->OnClicked.AddUniqueDynamic(this, &ThisClass::StartGame);
	SetIsFocusable(true);
	if (Button_MaintenanceWorker) Button_MaintenanceWorker->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectMaintenanceWorker);
	if (Button_Executive) Button_Executive->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectExecutive);
	if (Button_RepairGun) Button_RepairGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectRepairGun);
	if (Button_ExplosionGun) Button_ExplosionGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectExplosionGun);
	if (Button_ElectricGun) Button_ElectricGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectElectricGun);
	ConfigureButtonStyles();
	if (WeaponDetailsPanel) WeaponDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	if (Button_Back) Button_Back->SetVisibility(ESlateVisibility::Collapsed);
	RefreshPresentation();
}

FReply ULobbyPresentationWidgetBase::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Tab && bShowingCharacterDetails)
	{
		if (!InKeyEvent.IsRepeat()) NextCharacter();
		return FReply::Handled();
	}
	if (InKeyEvent.GetKey() == EKeys::Escape && (bShowingCharacterDetails || bShowingWeaponDetails))
	{
		if (!InKeyEvent.IsRepeat()) GoBack();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void ULobbyPresentationWidgetBase::NextCharacter()
{
	auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>();
	if (!PC || !bShowingCharacterDetails || bStartingGame) return;
	for (int32 Offset = 1; Offset <= PC->CharacterPresentations.Num(); ++Offset)
	{
		const int32 Index = (PC->SelectedPresentationIndex + Offset) % PC->CharacterPresentations.Num();
		if (PC->CharacterPresentations[Index].DisplayClass)
		{
			SelectCharacter(Index);
			break;
		}
	}
}

void ULobbyPresentationWidgetBase::GoBack()
{
	if (bStartingGame) return;
	if (bShowingWeaponDetails) ShowCharacter();
	else ShowMenu();
}

void ULobbyPresentationWidgetBase::StartGame()
{
	auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>();
	auto* GI = GetGameInstance<UTheManGameInstance>();
	if (bStartingGame || !bShowingWeaponDetails || !PC || !GI || GI->IsGameOver()
		|| !PC->CharacterPresentations.IsValidIndex(PC->SelectedPresentationIndex)) return;
	const FName CharacterID = PC->CharacterPresentations[PC->SelectedPresentationIndex].CharacterID;
	if (CharacterID.IsNone()) return;
	bStartingGame = true;
	if (Button_StartGame) Button_StartGame->SetIsEnabled(false);
	GI->SelectCharacterAndStart(CharacterID);
}

void ULobbyPresentationWidgetBase::HoverViewWeapons()
{
	if (Text_ViewWeapons) Text_ViewWeapons->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.40f, 0.09f, 1.f)));
}

void ULobbyPresentationWidgetBase::UnhoverViewWeapons()
{
	if (Text_ViewWeapons) Text_ViewWeapons->SetColorAndOpacity(FSlateColor(FLinearColor(0.65f, 0.68f, 0.70f, 1.f)));
}

void ULobbyPresentationWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	RefreshPresentation();
}

void ULobbyPresentationWidgetBase::RefreshPresentation()
{
	auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>();
	if (!PC) return;
	ALobbyCharacterBase* Active = PC->GetDisplayCharacter();
	if (DisplayCharacter.Get() != Active) { DisplayCharacter = Active; bNeedsRefresh = true; }
	const bool bWeapon = PC->IsWeaponPresentationView();
	const int32 Index = DisplayCharacter.IsValid() ? DisplayCharacter->DisplayWeaponIndex : INDEX_NONE;
	if (!bNeedsRefresh && bWeapon == bShowingWeaponDetails && Index == LastWeaponIndex && LastCharacterIndex == PC->SelectedPresentationIndex) return;
	bNeedsRefresh = false;
	LastCharacterIndex = PC->SelectedPresentationIndex;
	if (bWeapon) bShowingCharacterDetails = false;
	bShowingWeaponDetails = bWeapon;
	LastWeaponIndex = Index;
	if (PresentationMenu) PresentationMenu->SetVisibility(bWeapon || bShowingCharacterDetails ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	if (WeaponDetailsPanel) WeaponDetailsPanel->SetVisibility(bWeapon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (CharacterDetailsPanel) CharacterDetailsPanel->SetVisibility(bShowingCharacterDetails ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (Button_Back) Button_Back->SetVisibility(bWeapon || bShowingCharacterDetails ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	const auto* Character = PC->CharacterPresentations.IsValidIndex(LastCharacterIndex) ? &PC->CharacterPresentations[LastCharacterIndex] : nullptr;
	if (Text_CharacterName) Text_CharacterName->SetText(Character ? Character->DisplayName : FText::GetEmpty());
	if (Text_CharacterDescription) Text_CharacterDescription->SetText(Character ? Character->Description : FText::GetEmpty());
	if (Button_StartGame)
	{
		const auto* GI = GetGameInstance<UTheManGameInstance>();
		Button_StartGame->SetIsEnabled(Character && !Character->CharacterID.IsNone() && GI && !GI->IsGameOver() && !bStartingGame);
	}
	UButton* CharacterButtons[] = {Button_MaintenanceWorker, Button_Executive};
	UTextBlock* CharacterLabels[] = {Text_MaintenanceWorkerChoice, Text_ExecutiveChoice};
	for (int32 Choice = 0; Choice < UE_ARRAY_COUNT(CharacterButtons); ++Choice)
	{
		const bool bValid = PC->CharacterPresentations.IsValidIndex(Choice) && PC->CharacterPresentations[Choice].DisplayClass;
		if (CharacterLabels[Choice]) CharacterLabels[Choice]->SetText(bValid ? PC->CharacterPresentations[Choice].DisplayName : FText::GetEmpty());
		if (CharacterButtons[Choice])
		{
			CharacterButtons[Choice]->SetIsEnabled(bValid);
			FButtonStyle Style = CharacterButtons[Choice]->GetStyle();
			Style.Normal.OutlineSettings.Color = FSlateColor(Choice == LastCharacterIndex
				? FLinearColor(0.65f, 0.40f, 0.09f, 1.f) : FLinearColor(0.20f, 0.22f, 0.24f, 0.65f));
			Style.Pressed = Style.Normal;
			CharacterButtons[Choice]->SetStyle(Style);
		}
	}
	const FLobbyWeaponPresentation* Item = DisplayCharacter.IsValid() && DisplayCharacter->WeaponPresentations.IsValidIndex(Index)
		? &DisplayCharacter->WeaponPresentations[Index] : nullptr;
	// Older map instances serialized the whole presentation array before text fields existed.
	if (Item && Item->DisplayName.IsEmpty() && Item->Description.IsEmpty())
	{
		const auto* Defaults = DisplayCharacter->GetClass()->GetDefaultObject<ALobbyCharacterBase>();
		for (const auto& DefaultItem : Defaults->WeaponPresentations)
			if (DefaultItem.WeaponClass == Item->WeaponClass) { Item = &DefaultItem; break; }
	}
	if (Text_WeaponName) Text_WeaponName->SetText(Item ? Item->DisplayName : (Active ? Active->DisplayWeaponName : FText::GetEmpty()));
	if (Text_WeaponDescription) Text_WeaponDescription->SetText(Item ? Item->Description : (Active ? Active->DisplayWeaponDescription : FText::GetEmpty()));
	if (WeaponChoices) WeaponChoices->SetVisibility(Active && !Active->WeaponPresentations.IsEmpty() ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	UButton* Buttons[] = {Button_RepairGun, Button_ExplosionGun, Button_ElectricGun};
	UImage* Images[] = {Image_RepairGun, Image_ExplosionGun, Image_ElectricGun};
	UImage* Corners[] = {Corner_RepairGun, Corner_ExplosionGun, Corner_ElectricGun};
	for (int32 Choice = 0; Choice < UE_ARRAY_COUNT(Buttons); ++Choice)
	{
		const bool bValid = DisplayCharacter.IsValid() && DisplayCharacter->WeaponPresentations.IsValidIndex(Choice)
			&& DisplayCharacter->WeaponPresentations[Choice].WeaponClass;
		if (Buttons[Choice])
		{
			Buttons[Choice]->SetIsEnabled(bValid);
			FButtonStyle Style = Buttons[Choice]->GetStyle();
			Style.Normal.OutlineSettings.Color = FSlateColor(Choice == Index
				? FLinearColor(0.65f, 0.40f, 0.09f, 1.f) : FLinearColor(0.20f, 0.22f, 0.24f, 0.65f));
			Style.Pressed = Style.Normal;
			Buttons[Choice]->SetStyle(Style);
		}
		if (Images[Choice]) Images[Choice]->SetBrushFromTexture(bValid ? DisplayCharacter->WeaponPresentations[Choice].Thumbnail : nullptr);
		if (Corners[Choice]) Corners[Choice]->SetVisibility(bValid && Choice == Index ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void ULobbyPresentationWidgetBase::SelectRepairGun() { SelectWeapon(0); }
void ULobbyPresentationWidgetBase::SelectExplosionGun() { SelectWeapon(1); }
void ULobbyPresentationWidgetBase::SelectElectricGun() { SelectWeapon(2); }

void ULobbyPresentationWidgetBase::SelectWeapon(int32 Index)
{
	if (!bShowingWeaponDetails || !DisplayCharacter.IsValid()) return;
	DisplayCharacter->SetDisplayWeaponIndex(Index);
	DisplayCharacter->SetWeaponReady(true);
	RefreshPresentation();
}

void ULobbyPresentationWidgetBase::ShowCharacter()
{
	bShowingCharacterDetails = true;
	bNeedsRefresh = true;
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(false);
		RefreshPresentation();
		SetKeyboardFocus();
	}
}

void ULobbyPresentationWidgetBase::ShowMenu()
{
	bShowingCharacterDetails = false;
	bNeedsRefresh = true;
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>()) PC->SetWeaponPresentationView(false);
	RefreshPresentation();
}

void ULobbyPresentationWidgetBase::SelectMaintenanceWorker() { SelectCharacter(0); }
void ULobbyPresentationWidgetBase::SelectExecutive() { SelectCharacter(1); }

void ULobbyPresentationWidgetBase::SelectCharacter(int32 Index)
{
	if (!bShowingCharacterDetails) return;
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
		if (PC->SelectPresentationCharacter(Index)) { bNeedsRefresh = true; RefreshPresentation(); }
}

void ULobbyPresentationWidgetBase::ShowWeapon()
{
	bShowingCharacterDetails = false;
	bNeedsRefresh = true;
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(true);
		RefreshPresentation();
	}
}

void ULobbyPresentationWidgetBase::ConfigureButtonStyles()
{
	for (UButton* Button : {Button_Character.Get(), Button_Weapon.Get()})
	{
		if (!Button) continue;
		FButtonStyle Style = Button->GetStyle();
		Style.Normal.OutlineSettings.Color = FSlateColor(FLinearColor(0.12f, 0.13f, 0.14f, 0.65f));
		Style.Pressed = Style.Normal;
		Button->SetStyle(Style);
	}
}
