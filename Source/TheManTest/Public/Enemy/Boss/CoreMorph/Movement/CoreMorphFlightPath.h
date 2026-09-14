#pragma once

#include "CoreMinimal.h"

// Source reference trajectory only. Presentation can consume this or any other route.
struct THEMANTEST_API FCoreMorphFlightPath
{
	FVector GroundPoint = FVector(2400, 0, -785.72);
	float Duration = 8.5f, AscentExtension = 8.f, AirflowDuration = 0.f;
	float CrashDuration = .65f, FeedDuration = 1.15f, ConstructionExtension = .3f;
	float MantaScale = 1.5f;
	float GetMorphDuration() const { return Duration + AscentExtension + AirflowDuration + CrashDuration + FeedDuration + ConstructionExtension; }
	float GetFormScale(int32) const { return MantaScale; }
	FVector GetFormOffset(int32) const { return FVector(-16000, 0, 1600); }
	float GetReleaseSeconds() const { return Schedule(.40f) * GetMorphDuration() + 2.f; }
	void PrepareDive();
	FVector FlightPosition(float NormalizedTime) const;
	float Schedule(float Choreography) const;
private:
	FVector OrbitPosition(float Time) const;
	FVector OrbitVelocity(float Time) const;
	FVector DiveTurnStart = FVector::ZeroVector, DiveTurnVelocity = FVector::ZeroVector;
	FVector DiveTurnRates = FVector::ZeroVector, DiveTurnEnd = FVector::ZeroVector, DiveDirection = FVector::DownVector;
};
