#include "Core/CharacterSelectPlayerController.h"

#include "Core/CharacterSelectCameraSwitcher.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EngineUtils.h"
#include "InputAction.h"
#include "InputMappingContext.h"

void ACharacterSelectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (CharacterSelectMappingContext)
		{
			Subsystem->AddMappingContext(CharacterSelectMappingContext, MappingContextPriority);
		}
	}
}

void ACharacterSelectPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EIC->BindAction(ClickAction, ETriggerEvent::Started,
				this, &ACharacterSelectPlayerController::HandleClick);
		}
	}
}

void ACharacterSelectPlayerController::SetPointerOverUI(bool bInPointerOverUI)
{
	bPointerOverUI = bInPointerOverUI;
}

void ACharacterSelectPlayerController::HandleClick()
{
	if (bPointerOverUI)
	{
		return;
	}

	if (ACharacterSelectCameraSwitcher* Switcher = GetCameraSwitcher())
	{
		Switcher->ToggleCameraView();
	}
}

ACharacterSelectCameraSwitcher* ACharacterSelectPlayerController::GetCameraSwitcher()
{
	if (CameraSwitcherOverride)
	{
		return CameraSwitcherOverride;
	}

	if (CachedCameraSwitcher)
	{
		return CachedCameraSwitcher;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<ACharacterSelectCameraSwitcher> It(World); It; ++It)
	{
		CachedCameraSwitcher = *It;
		break;
	}

	return CachedCameraSwitcher;
}
