#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "LobbyCharacterAnimInstance.generated.h"

/** Loop the authored lobby pose and keep the support hand on the selected weapon. */
UCLASS(Transient)
class THEMANTEST_API ULobbyCharacterAnimInstance : public UAnimSingleNodeInstance
{
	GENERATED_BODY()

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
