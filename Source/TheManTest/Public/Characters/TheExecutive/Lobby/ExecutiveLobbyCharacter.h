#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "ExecutiveLobbyCharacter.generated.h"

class UPointLightComponent;

/** Original-look local emissive spill, attached to the Executive's display skeleton. */
UCLASS(Blueprintable)
class THEMANTEST_API AExecutiveLobbyCharacter : public ALobbyCharacterBase
{
	GENERATED_BODY()
public:
	AExecutiveLobbyCharacter();
	virtual void SetActorHiddenInGame(bool bNewHidden) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Effects")
	TObjectPtr<UPointLightComponent> FaceGlow;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Lobby|Effects")
	TObjectPtr<UPointLightComponent> ChestGlow;
};
