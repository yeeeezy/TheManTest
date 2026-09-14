#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Components/Button.h"
#include "Core/CharacterSelectPlayerController.h"

void ULobbyPresentationWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Button_Character) Button_Character->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	if (Button_Weapon) Button_Weapon->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowWeapon);
	auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>();
	ApplySelection(PC && PC->IsWeaponPresentationView());
}

void ULobbyPresentationWidgetBase::ShowCharacter()
{
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(false);
		ApplySelection(PC->IsWeaponPresentationView());
	}
}

void ULobbyPresentationWidgetBase::ShowWeapon()
{
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(true);
		ApplySelection(PC->IsWeaponPresentationView());
	}
}

void ULobbyPresentationWidgetBase::ApplySelection(bool bWeapon)
{
	for (UButton* Button : {Button_Character.Get(), Button_Weapon.Get()})
	{
		if (!Button) continue;
		FButtonStyle Style = Button->GetStyle();
		const bool bSelected = (Button == Button_Weapon) == bWeapon;
		Style.Normal.OutlineSettings.Color = FSlateColor(bSelected
			? FLinearColor(0.55f, 0.38f, 0.10f, 1.f) : FLinearColor(0.12f, 0.13f, 0.14f, 0.65f));
		Button->SetStyle(Style);
	}
}
