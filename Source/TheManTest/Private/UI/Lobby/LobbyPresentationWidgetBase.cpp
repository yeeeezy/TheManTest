#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Components/Button.h"
#include "Core/CharacterSelectPlayerController.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "EngineUtils.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"

void ULobbyPresentationWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Button_Character) Button_Character->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	if (Button_Weapon) Button_Weapon->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowWeapon);
	if (Button_Back) Button_Back->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	ConfigureButtonStyles();
	if (WeaponDetailsPanel) WeaponDetailsPanel->SetVisibility(ESlateVisibility::Collapsed);
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
	for (UButton* Button : {Button_Character.Get(), Button_Weapon.Get(), Button_Back.Get()})
	{
		if (!Button) continue;
		FButtonStyle Style = Button->GetStyle();
		Style.Normal.OutlineSettings.Color = FSlateColor(FLinearColor(0.12f, 0.13f, 0.14f, 0.65f));
		Style.Pressed = Style.Normal;
		Button->SetStyle(Style);
	}
}
