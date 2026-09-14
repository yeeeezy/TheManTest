#include "UI/Lobby/LobbyPresentationWidgetBase.h"
#include "Components/Button.h"
#include "Core/CharacterSelectPlayerController.h"

void ULobbyPresentationWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (Button_Character) Button_Character->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowCharacter);
	if (Button_Weapon) Button_Weapon->OnClicked.AddUniqueDynamic(this, &ThisClass::ShowWeapon);
	ConfigureButtonStyles();
}

void ULobbyPresentationWidgetBase::ShowCharacter()
{
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(false);
	}
}

void ULobbyPresentationWidgetBase::ShowWeapon()
{
	if (auto* PC = GetOwningPlayer<ACharacterSelectPlayerController>())
	{
		PC->SetWeaponPresentationView(true);
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
