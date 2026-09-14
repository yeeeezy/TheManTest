#include "Core/CharacterSelectPlayerController.h"

#include "Core/CharacterSelectCameraSwitcher.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
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
#if WITH_EDITOR
		if (TestAction)
		{
			EIC->BindAction(TestAction, ETriggerEvent::Started,
				this, &ACharacterSelectPlayerController::HandleTestInput);
		}
#endif
	}
}

void ACharacterSelectPlayerController::SetPointerOverUI(bool bInPointerOverUI)
{
	bPointerOverUI = bInPointerOverUI;
}

void ACharacterSelectPlayerController::SetWeaponPresentationView(bool bWeapon)
{
	if (ACharacterSelectCameraSwitcher* Switcher = GetCameraSwitcher())
	{
		if (bWeapon) Switcher->SetNearCameraView();
		else Switcher->SetFarCameraView();
	}
}

bool ACharacterSelectPlayerController::IsWeaponPresentationView()
{
	const ACharacterSelectCameraSwitcher* Switcher = GetCameraSwitcher();
	return Switcher && Switcher->IsUsingNearCamera();
}

#if WITH_EDITOR
void ACharacterSelectPlayerController::HandleTestInput()
{
	UWorld* World = GetWorld();
	if (!World || World->WorldType != EWorldType::PIE)
	{
		return;
	}

	for (TActorIterator<ALobbyCharacterBase> It(World); It; ++It)
	{
		ALobbyCharacterBase* LobbyCharacter = *It;
		LobbyCharacter->SetWeaponReady(!LobbyCharacter->IsWeaponReady());
		UE_LOG(LogTemp, Display, TEXT("[LobbyTest] %s weapon ready: %s"),
			*LobbyCharacter->GetName(), LobbyCharacter->IsWeaponReady() ? TEXT("true") : TEXT("false"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[LobbyTest] No LobbyCharacterBase actor found in the current PIE world."));
}
#endif

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
