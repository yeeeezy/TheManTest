#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TheManLobbyAssetLibrary.generated.h"

class UBlueprint;
class UTexture2D;

/** Explicit editor authoring; never replaces an existing widget layout. */
UCLASS()
class THEMANTEST_API UTheManLobbyAssetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Lobby")
	static bool InitializePresentationMenu(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Lobby")
	static bool AddWeaponDetails(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Lobby")
	static bool RefineWeaponDetails(UBlueprint* Blueprint, UTexture2D* CornerTexture);
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Lobby")
	static bool AddCharacterDetails(UBlueprint* Blueprint);
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Lobby")
	static bool RefineCharacterNavigation(UBlueprint* Blueprint);
};
