#include "Core/TheManGameModeBase.h"
#include "Core/TheManGameStateBase.h"
#include "Core/TheManGameInstance.h"
#include "Core/TheManCharacterTypes.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"

ATheManGameModeBase::ATheManGameModeBase()
{
	GameStateClass = ATheManGameStateBase::StaticClass();
}

UClass* ATheManGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// ── 临时诊断（定位选角色生成问题，查清后删）──
	auto Dbg = [](const FString& Msg, const FColor& Col)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 10.0f, Col, FString::Printf(TEXT("[PawnSelect] %s"), *Msg)); }
		UE_LOG(LogTemp, Warning, TEXT("[PawnSelect] %s"), *Msg);
	};

	UTheManGameInstance* GI = GetGameInstance<UTheManGameInstance>();
	if (!GI)
	{
		Dbg(TEXT("GameInstance is not UTheManGameInstance. Falling back to the default pawn."), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	const FName ID = GI->GetSelectedCharacterID();
	if (ID.IsNone())
	{
		Dbg(TEXT("SelectedCharacterID is empty. Falling back to the default pawn."), FColor::Orange);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (!CharacterRosterTable)
	{
		Dbg(FString::Printf(TEXT("ID=%s but CharacterRosterTable is not assigned. Falling back to the default pawn."), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	FCharacterType* Row = CharacterRosterTable->FindRow<FCharacterType>(ID, TEXT("GetDefaultPawnClass"));
	if (!Row)
	{
		Dbg(FString::Printf(TEXT("Roster row '%s' was not found. Falling back to the default pawn."), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (!Row->CharacterClass)
	{
		Dbg(FString::Printf(TEXT("Roster row '%s' has no CharacterClass. Falling back to the default pawn."), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	Dbg(FString::Printf(TEXT("Spawning selected character: %s -> %s"), *ID.ToString(), *Row->CharacterClass->GetName()), FColor::Green);
	return Row->CharacterClass;
}
