#pragma once

#include "CoreMinimal.h"

struct FCoreMorphVisualPiece;

// Presentation consumes actual motion and intent. It knows
// nothing about route duration, orbit radius, dive timestamps, GA or form state.
struct THEMANTEST_API FCoreMorphFlightMotion
{
	void Reset(const FTransform& Body, int32 Seed, float InRandomness, float InTrailLength, float InRollSpacing = 65000.f);
	void Update(const FVector& WorldPosition, float DeltaSeconds);
	FTransform PiecePose(const FCoreMorphVisualPiece& Piece, const FVector& PieceScale) const;
	const FTransform& GetBody() const { return Body; }
	float Movement = 0.f, Climb = 0.f, Dive = 0.f, Bank = 0.f, Phase = 0.f;
	// Intent arrives before displacement. Propulsion waits for this preparation
	// to become visible instead of only reacting to acceleration after it happened.
	float AccelerationIntent = 0.f, PowerStroke = 0.f;
	float GetSpeed() const { return float(PreviousVelocity.Size()); }
	bool RequestRoll(bool bFast, int32 Direction = 1);
	bool IsRolling() const { return bRolling; }
	float GetRollAngle() const { return RollAngle; }
	float GetWingLag() const { return RootLag + TipLag; }
	bool bAllowRandomRolls = false;
	float RandomRollSpacing = 65000.f;
	int32 GetHistoryCount() const { return History.Num(); }
private:
	struct FTrailSample { double Distance; FTransform Pose; };
	FTransform Body;
	FQuat TravelRotation = FQuat::Identity;
	FRandomStream RollRandom;
	bool bRolling = false;
	float RollAngle = 0, RollRate = 0, RollTarget = 0, RollMaxRate = 0, RollAcceleration = 0;
	float RootLag = 0, RootLagRate = 0, TipLag = 0, TipLagRate = 0, Flutter = 0;
	double NextRollDistance = 0;
	TArray<FTrailSample> History;
	FVector PreviousVelocity = FVector::ZeroVector;
	FVector NoiseOffsets = FVector::ZeroVector;
	float Randomness = 0.f, NoiseClock = 0.f, Stroke = 0.f, Activity = 0.f;
	float Noise = 0.f, Asymmetry = 0.f, TrailLength = 20000.f;
	double Distance = 0;
	FTransform TrailPose(float Behind) const;
	void UpdateRoll(float Dt);
};
