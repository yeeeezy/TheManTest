#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Components/Button.h"
#include "Core/CharacterSelectPlayerController.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Image.h"
#include "EngineUtils.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"

void ULobbyPresentationWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Button_Character) Button_Character->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	if (Button_Weapon) Button_Weapon->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowWeapon);
	if (Button_Back) Button_Back->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	if (Button_RepairGun) Button_RepairGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectRepairGun);
	if (Button_ExplosionGun) Button_ExplosionGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectExplosionGun);
	if (Button_ElectricGun) Button_ElectricGun->OnClicked.AddUniqueDynamic(this, &ThisClass::SelectElectricGun);
	ConfigureButtonStyles();
	if (WeaponDetailsPanel) WeaponDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
	if (Button_Back) Button_Back->SetVisibility(ESlateVisibility::Collapsed);
	RefreshPresentation();
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
	if (!DisplayCharacter.IsValid())
		for (TActorIterator<ALobbyCharacterBase> It(GetWorld()); It; ++It) { DisplayCharacter = *It; break; }
	const bool bWeapon = PC->IsWeaponPresentationView();
	const int32 Index = DisplayCharacter.IsValid() ? DisplayCharacter->DisplayWeaponIndex : INDEX_NONE;
	if (bWeapon == bShowingWeaponDetails && Index == LastWeaponIndex) return;
	bShowingWeaponDetails = bWeapon;
	LastWeaponIndex = Index;
	if (PresentationMenu) PresentationMenu->SetVisibility(bWeapon ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	if (WeaponDetailsPanel) WeaponDetailsPanel->SetVisibility(bWeapon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (Button_Back) Button_Back->SetVisibility(bWeapon ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	const FLobbyWeaponPresentation* Item = DisplayCharacter.IsValid() && DisplayCharacter->WeaponPresentations.IsValidIndex(Index)
		? &DisplayCharacter->WeaponPresentations[Index] : nullptr;
	// Older map instances serialized the whole presentation array before text fields existed.
	if (Item && Item->DisplayName.IsEmpty() && Item->Description.IsEmpty())
	{
		const auto* Defaults = DisplayCharacter->GetClass()->GetDefaultObject<ALobbyCharacterBase>();
		for (const auto& DefaultItem : Defaults->WeaponPresentations)
			if (DefaultItem.WeaponClass == Item->WeaponClass) { Item = &DefaultItem; break; }
	}
	if (Text_WeaponName) Text_WeaponName->SetText(Item ? Item->DisplayName : FText::GetEmpty());
	if (Text_WeaponDescription) Text_WeaponDescription->SetText(Item ? Item->Description : FText::GetEmpty());
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
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(false);
		RefreshPresentation();
	}
}

void ULobbyPresentationWidgetBase::ShowWeapon()
{
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
