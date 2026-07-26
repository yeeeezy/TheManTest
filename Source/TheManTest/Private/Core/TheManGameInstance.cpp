#include "Core/TheManGameInstance.h"
#include "Kismet/GameplayStatics.h"

void UTheManGameInstance::SelectCharacterAndStart(FName CharacterID)
{
	SelectedCharacterID = CharacterID;
	bPendingTransition = false;   // 新一局开始，解除死亡切换守卫
	UGameplayStatics::OpenLevel(this, TestMapName);
}

void UTheManGameInstance::HandlePlayerDeath(int32 CurrentRoundNumber)
{
	if (bPendingTransition) { return; }   // 同帧多源死亡只切一次
	bPendingTransition = true;

	CarriedRoundNumber = CurrentRoundNumber;
	UGameplayStatics::OpenLevel(this, LobbyMapName);
}

void UTheManGameInstance::HandleGameOver()
{
	if (bPendingTransition) { return; }   // 与死亡切换共用守卫，防同帧重复
	bPendingTransition = true;

	bGameOver = true;
	UGameplayStatics::OpenLevel(this, LobbyMapName);
}
