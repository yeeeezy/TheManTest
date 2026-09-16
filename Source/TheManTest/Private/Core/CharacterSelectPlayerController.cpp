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
	if (ALobbyCharacterBase* LobbyCharacter = GetDisplayCharacter())
	{
		if (bWeapon)
		{
			LobbyCharacter->SetWeaponReady(true);
		}
		else
		{
			TArray<int32> ValidIdleIndices;
			if (LobbyCharacter->WeaponPresentations.IsValidIndex(LobbyCharacter->DisplayWeaponIndex))
			{
				const auto& Animations = LobbyCharacter->WeaponPresentations[LobbyCharacter->DisplayWeaponIndex].RelaxedAnimations;
				for (int32 Index = 0; Index < Animations.Num(); ++Index)
				{
					if (Animations[Index]) ValidIdleIndices.Add(Index);
				}
			}
			if (!ValidIdleIndices.IsEmpty())
				LobbyCharacter->SetRelaxedIdleIndex(ValidIdleIndices[FMath::RandHelper(ValidIdleIndices.Num())]);
			else
				LobbyCharacter->SetWeaponReady(false);
		}
	}
	if (ACharacterSelectCameraSwitcher* Switcher = GetCameraSwitcher())
	{
		if (bWeapon) Switcher->SetNearCameraView();
		else Switcher->SetFarCameraView();
	}
}

ALobbyCharacterBase* ACharacterSelectPlayerController::GetDisplayCharacter()
{
	if (IsValid(ActiveDisplayCharacter)) return ActiveDisplayCharacter;
	DisplayCharacters.SetNum(CharacterPresentations.Num());
	for (TActorIterator<ALobbyCharacterBase> It(GetWorld()); It; ++It)
	{
		if (It->IsHidden()) continue;
		ActiveDisplayCharacter = *It;
		for (int32 Index = 0; Index < CharacterPresentations.Num(); ++Index)
			if (CharacterPresentations[Index].DisplayClass && It->IsA(CharacterPresentations[Index].DisplayClass))
			{
				DisplayCharacters[Index] = *It;
				SelectedPresentationIndex = Index;
				break;
			}
		break;
	}
	return ActiveDisplayCharacter;
}

bool ACharacterSelectPlayerController::SelectPresentationCharacter(int32 Index)
{
	if (!CharacterPresentations.IsValidIndex(Index) || !CharacterPresentations[Index].DisplayClass) return false;
	ALobbyCharacterBase* Previous = GetDisplayCharacter();
	if (!Previous) return false;
	DisplayCharacters.SetNum(CharacterPresentations.Num());
	ALobbyCharacterBase* Next = DisplayCharacters[Index];
	if (!IsValid(Next))
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.ObjectFlags |= RF_Transient;
		Next = GetWorld()->SpawnActor<ALobbyCharacterBase>(CharacterPresentations[Index].DisplayClass, Previous->GetActorTransform(), Params);
		if (!Next) return false;
		DisplayCharacters[Index] = Next;
	}
	if (Previous != Next) Previous->SetActorHiddenInGame(true);
	Next->SetActorHiddenInGame(false);
	ActiveDisplayCharacter = Next;
	SelectedPresentationIndex = Index;
	SetWeaponPresentationView(false);
	return true;
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

	if (ALobbyCharacterBase* LobbyCharacter = GetDisplayCharacter())
	{
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
