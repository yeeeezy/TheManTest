#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "CoreMorphFlightReview.generated.h"

class ACoreMorphBoss;
class ACoreMorphFlightRoute;

USTRUCT(BlueprintType)
struct FCoreMorphReviewRoute
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere) TObjectPtr<ACoreMorphFlightRoute> Route;
	UPROPERTY(EditAnywhere) FString Label;
	UPROPERTY(EditAnywhere, meta=(ClampMin="1")) float Speed = 9000.f;
};

/** Controls belong only to the flight review map; no production input changes. */
UCLASS(NotBlueprintable)
class THEMANTEST_API ACoreMorphFlightReview : public ACameraActor
{
	GENERATED_BODY()
public:
	ACoreMorphFlightReview();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY(EditInstanceOnly, Category="Review") TObjectPtr<ACoreMorphBoss> Boss;
	UPROPERTY(EditInstanceOnly, Category="Review") TArray<FCoreMorphReviewRoute> Routes;
	bool SelectRoute(int32 Index);
	void PlayFlight();
	void ResetFlight();
	void PauseFlight();
	void ToggleCamera();
private:
	bool bFollow = true;
	bool bStartFirstRoute = false;
	int32 SelectedRoute = INDEX_NONE;
	void RouteOne() { SelectRoute(0); }
	void RouteTwo() { SelectRoute(1); }
	void RouteThree() { SelectRoute(2); }
};
