#pragma once

#include "CoreMinimal.h"

class UCoreMorphVisualLayout;
struct FCoreMorphTailMotion;

// Accepted FEAT058 choreography in its fixed local frame. No Actor or GAS ownership here.
struct THEMANTEST_API FCoreMorphFlightPath
{
	const UCoreMorphVisualLayout* Layout = nullptr;
	TArray<FVector> SourcePositions;
	FVector GroundPoint = FVector(2400, 0, -785.72);
	float Duration = 8.5f, AscentExtension = 8.f, AirflowDuration = 0.f;
	float CrashDuration = .65f, FeedDuration = 1.15f, ConstructionExtension = .3f;
	float MantaScale = 1.5f;
	float GetMorphDuration() const { return Duration + AscentExtension + AirflowDuration + CrashDuration + FeedDuration + ConstructionExtension; }
	float GetFormScale(int32) const { return MantaScale; }
	FVector GetFormOffset(int32) const { return FVector(-16000, 0, 1600); }
	float GetReleaseSeconds() const { return Schedule(.40f) * GetMorphDuration() + 2.f; }
	void PrepareDive();
	FTransform DivePose(float NormalizedTime) const;
	FTransform SourcePose(int32 Index, float NormalizedTime, const FCoreMorphTailMotion& TailMotion) const;
	float Schedule(float Choreography) const;
	float ChoreographyTime(float NormalizedTime) const;
private:
	FVector OrbitPosition(float Time) const;
	FVector OrbitVelocity(float Time) const;
	FVector FlightPosition(float Time) const;
	FVector DiveTurnStart = FVector::ZeroVector, DiveTurnVelocity = FVector::ZeroVector;
	FVector DiveTurnRates = FVector::ZeroVector, DiveTurnEnd = FVector::ZeroVector, DiveDirection = FVector::DownVector;
};
