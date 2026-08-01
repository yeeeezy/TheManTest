#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Highlightable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UHighlightable : public UInterface
{
	GENERATED_BODY()
};

class THEMANTEST_API IHighlightable
{
	GENERATED_BODY()
public:
	// Duration < 0 = 永不自动消失
	UFUNCTION(BlueprintNativeEvent, Category = "Highlight")
	void StartHighlight(float Duration);

	UFUNCTION(BlueprintNativeEvent, Category = "Highlight")
	void StopHighlight();
};
