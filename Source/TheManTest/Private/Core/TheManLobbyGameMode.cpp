#include "Core/TheManLobbyGameMode.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ATheManLobbyGameMode::ATheManLobbyGameMode()
{
	// 大厅不需要战斗角色：不生成默认 Pawn
	DefaultPawnClass = nullptr;
}

void ATheManLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC) { return; }

	// 创建并显示选角色 UI
	if (LobbyWidgetClass)
	{
		LobbyWidget = CreateWidget<UUserWidget>(PC, LobbyWidgetClass);
		if (LobbyWidget)
		{
			LobbyWidget->AddToViewport();
		}
	}

	// 鼠标可见 + UI 输入模式（按钮可点）
	PC->bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
}
