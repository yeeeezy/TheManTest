#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TheManLobbyGameMode.generated.h"

class UUserWidget;

/**
 * ATheManLobbyGameMode
 * 选角色大厅（LobbyMap）专用 GameMode，与测试地图的 ATheManGameModeBase 完全分离——
 * 不生成战斗角色、不跑回合系统。BeginPlay 自动创建选角色 UI、显示鼠标、切到 UI 输入模式。
 *
 * 用法：基于本类建蓝图 GameMode，填 LobbyWidgetClass = WBP_CharacterSelect，
 * 设为 LobbyMap 的 GameMode Override 即可。
 */
UCLASS()
class THEMANTEST_API ATheManLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATheManLobbyGameMode();

	virtual void BeginPlay() override;

protected:
	// 选角色 UI（WBP_CharacterSelect）。留空则不自动创建，由蓝图/关卡自行处理。
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lobby")
	TSubclassOf<UUserWidget> LobbyWidgetClass;

	// 已创建的 UI 实例
	UPROPERTY(BlueprintReadOnly, Category = "Lobby")
	UUserWidget* LobbyWidget = nullptr;
};
