#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TheManGameModeBase.generated.h"

class UDataTable;

UCLASS()
class THEMANTEST_API ATheManGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATheManGameModeBase();

	// 据 GameInstance 选定的角色 ID 在花名册里查出角色类作为默认 Pawn（在 PlayerStart 生成）。
	// 选角色地图用各自的 GameMode（不带本逻辑），故不会在菜单里生成战斗角色。
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

protected:
	// 角色花名册（DT_CharacterRoster），ID → 角色类。蓝图 GameMode 上配置。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roster")
	UDataTable* CharacterRosterTable = nullptr;
};
