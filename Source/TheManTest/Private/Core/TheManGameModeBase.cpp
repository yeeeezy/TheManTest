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
		Dbg(TEXT("GameInstance 不是 UTheManGameInstance（Project Settings 没设 GameInstance Class？）→ 回退默认 Pawn"), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	const FName ID = GI->GetSelectedCharacterID();
	if (ID.IsNone())
	{
		Dbg(TEXT("SelectedCharacterID 为空（没经过选角色，或直接 PIE 进了 TestMap）→ 回退默认 Pawn"), FColor::Orange);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (!CharacterRosterTable)
	{
		Dbg(FString::Printf(TEXT("ID=%s 但 GameMode 的 CharacterRosterTable 没填 → 回退默认 Pawn"), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	FCharacterType* Row = CharacterRosterTable->FindRow<FCharacterType>(ID, TEXT("GetDefaultPawnClass"));
	if (!Row)
	{
		Dbg(FString::Printf(TEXT("花名册里找不到行名 '%s'（按钮 ID 与 DT 行名不一致？）→ 回退默认 Pawn"), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	if (!Row->CharacterClass)
	{
		Dbg(FString::Printf(TEXT("行 '%s' 的 CharacterClass 为空（DT 该行没填角色蓝图）→ 回退默认 Pawn"), *ID.ToString()), FColor::Red);
		return Super::GetDefaultPawnClassForController_Implementation(InController);
	}

	Dbg(FString::Printf(TEXT("生成选定角色：%s → %s"), *ID.ToString(), *Row->CharacterClass->GetName()), FColor::Green);
	return Row->CharacterClass;
}
