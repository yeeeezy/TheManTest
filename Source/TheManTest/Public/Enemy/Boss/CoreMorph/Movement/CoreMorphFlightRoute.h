#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoreMorphFlightRoute.generated.h"

class USplineComponent;

// Optional, editable route. The boss owns all movement state and presentation.
UCLASS()
class THEMANTEST_API ACoreMorphFlightRoute : public AActor
{
	GENERATED_BODY()
public:
	ACoreMorphFlightRoute();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Flight Route") TObjectPtr<USplineComponent> Spline;
};
