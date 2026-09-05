#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TheManAudioAssetLibrary.generated.h"

class USoundCue;
class USoundWave;

/** Editor creation helper; audio variation runs in standard engine Sound Cue nodes. */
UCLASS()
class THEMANTEST_API UTheManAudioAssetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Initializes an EMPTY Cue. Refuses to overwrite an authored graph. */
	UFUNCTION(BlueprintCallable, Category="TheManTest|Editor|Audio")
	static bool InitializeVariationCue(USoundCue* Cue, const TArray<USoundWave*>& Waves,
		float PitchMin=0.96f, float PitchMax=1.04f, float VolumeMin=0.95f, float VolumeMax=1.f);
};
