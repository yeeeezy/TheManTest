#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CharacterSelectGameMode.generated.h"

class UUserWidget;

UCLASS()
class THEMANTEST_API ACharacterSelectGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACharacterSelectGameMode();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Select|UI")
	TSubclassOf<UUserWidget> CharacterSelectWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|UI")
	TObjectPtr<UUserWidget> CharacterSelectWidget;
};
