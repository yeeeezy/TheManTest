#pragma once

#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "MaintenanceWorkerLobbyCharacter.generated.h"

/** Maintenance Worker presentation base; its Blueprint owns the finalized lobby assets. */
UCLASS(Blueprintable)
class THEMANTEST_API AMaintenanceWorkerLobbyCharacter : public ALobbyCharacterBase
{
	GENERATED_BODY()
};
