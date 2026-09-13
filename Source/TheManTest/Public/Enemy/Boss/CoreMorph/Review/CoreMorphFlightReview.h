#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "CoreMorphFlightReview.generated.h"

class ACoreMorphBoss;

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
	void PlayFlight();
	void ResetFlight();
	void PauseFlight();
	void ToggleCamera();
private:
	bool bFollow = true;
};
