#pragma once

#include "CoreMinimal.h"

// Per-flight response to measured world velocity; no knowledge of the authored route or its clock.
struct THEMANTEST_API FCoreMorphTailMotion
{
	float Movement = 0.f;
	float Climb = 0.f;
	float Phase = 0.f;

	void Update(const FVector& WorldVelocity, float DeltaSeconds);
	void Apply(float TailOrder, FVector& LocalPosition, FRotator& LocalBend) const;
};
