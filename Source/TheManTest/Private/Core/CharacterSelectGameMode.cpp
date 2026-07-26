#include "Core/CharacterSelectGameMode.h"

#include "Blueprint/UserWidget.h"
#include "Core/CharacterSelectPlayerController.h"

ACharacterSelectGameMode::ACharacterSelectGameMode()
{
	DefaultPawnClass = nullptr;
	PlayerControllerClass = ACharacterSelectPlayerController::StaticClass();
}

void ACharacterSelectGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!CharacterSelectWidgetClass)
	{
		return;
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return;
	}

	CharacterSelectWidget = CreateWidget<UUserWidget>(PC, CharacterSelectWidgetClass);
	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->AddToViewport();
	}
}
