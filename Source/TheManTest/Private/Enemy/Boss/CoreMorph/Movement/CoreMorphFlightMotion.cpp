#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightMotion.h"
#include "Enemy/Boss/CoreMorph/Data/CoreMorphVisualLayout.h"

void FCoreMorphFlightMotion::Reset(const FTransform& InBody, int32 Seed, float InRandomness, float InTrailLength)
{
	*this = FCoreMorphFlightMotion();
	Body = InBody;
	Randomness = FMath::Clamp(InRandomness, 0.f, 1.f);
	TrailLength = FMath::Max(100.f, InTrailLength);
	FRandomStream Random(Seed);
	NoiseOffsets = FVector(Random.FRand(), Random.FRand(), Random.FRand()) * (2.f * PI);
	History.Add({0., Body});
}

void FCoreMorphFlightMotion::Update(const FVector& Position, float Dt)
{
	if (Dt <= 0.f || !FMath::IsFinite(Dt) || Position.ContainsNaN()) return;
	const FVector Displacement = Position - Body.GetLocation();
	const FVector Velocity = Displacement / Dt;
	const float Speed = float(Velocity.Size());
	const float PreviousSpeed = float(PreviousVelocity.Size());
	const float Blend = 1.f - FMath::Exp(-Dt / .25f);
	Movement = FMath::Lerp(Movement, FMath::Clamp(Speed / 11000.f, 0.f, 1.f), Blend);
	Climb = FMath::Lerp(Climb, FMath::Clamp(float(Velocity.Z) / 8000.f, 0.f, 1.f), Blend);
	const float DiveTarget = FMath::Clamp(float(-Velocity.Z) / FMath::Max(Speed, 1.f) / .65f, 0.f, 1.f)
		* FMath::Clamp(Speed / 4000.f, 0.f, 1.f);
	Dive = FMath::Lerp(Dive, DiveTarget, Blend);
	Activity = FMath::Lerp(Activity, 1.f, Blend);
	PowerStroke = FMath::Lerp(PowerStroke, FMath::Clamp(AccelerationIntent, 0.f, 1.f), 1.f - FMath::Exp(-Dt / .12f));
	const float Acceleration = FMath::Clamp((Speed - PreviousSpeed) / Dt / 8000.f, 0.f, 1.f);
	Stroke = FMath::Lerp(Stroke, FMath::Clamp(.18f + .6f * Movement + .5f * Climb + .2f * Acceleration + .9f * PowerStroke, 0.f, 1.6f) * (1.f - .8f * Dive), Blend);

	// Seeded, low-frequency variation is shared across each connected surface.
	// No per-frame random draws and no independent jitter on neighbouring pieces.
	NoiseClock += Dt;
	Noise = Randomness * (.65f * FMath::Sin(NoiseClock * .73f + NoiseOffsets.X) + .35f * FMath::Sin(NoiseClock * 1.13f + NoiseOffsets.Y));
	Asymmetry = Randomness * FMath::Sin(NoiseClock * .47f + NoiseOffsets.Z);
	Phase = FMath::Fmod(Phase + Dt * 2.f * PI * (.35f + .45f * Movement + .3f * Climb + .25f * PowerStroke) * (1.f + .15f * Noise), 2.f * PI);

	float TurnRate = 0.f;
	if (Velocity.SizeSquared2D() > 10000. && PreviousVelocity.SizeSquared2D() > 10000.)
	{
		const FVector Before = PreviousVelocity.GetSafeNormal2D(), After = Velocity.GetSafeNormal2D();
		TurnRate = FMath::Atan2(float(FVector::CrossProduct(Before, After).Z), float(FVector::DotProduct(Before, After))) / Dt;
	}
	const float BankTarget = FMath::Clamp(FMath::RadiansToDegrees(FMath::Atan2(float(Velocity.Size2D()) * TurnRate, 8000.f)), -55.f, 55.f);
	Bank = FMath::Lerp(Bank, BankTarget, Blend);
	if (Speed > 10.f)
	{
		FRotator Facing = Velocity.Rotation();
		// Keep yaw well-defined at a vertical tangent and keep all turns continuous.
		if (Velocity.Size2D() < Speed * .02f) Facing.Yaw = Body.Rotator().Yaw;
		Facing.Roll = Bank + 1.5f * Asymmetry * Movement * (1.f - Dive);
		Body.SetRotation(FQuat::Slerp(Body.GetRotation(), Facing.Quaternion(), 1.f - FMath::Exp(-Dt / .08f)).GetNormalized());
	}
	Body.SetLocation(Position);
	PreviousVelocity = Velocity;
	Distance += Displacement.Size();
	if (History.IsEmpty()) History.Add({Distance, Body});
	else if (Distance - History.Last().Distance >= 30.) History.Add({Distance, Body});
	// Retain only the part of the travelled route that the tail can still use.
	while (History.Num() > 2 && History[1].Distance < Distance - TrailLength - 100.) History.RemoveAt(0);
}

FTransform FCoreMorphFlightMotion::TrailPose(float Behind) const
{
	const double Target = Distance - Behind;
	if (History.IsEmpty() || Behind <= 0.f) return Body;
	if (Target <= History[0].Distance)
	{
		FTransform Result = History[0].Pose;
		Result.AddToTranslation(-Result.GetUnitAxis(EAxis::X) * (History[0].Distance - Target));
		return Result;
	}
	FTrailSample After{Distance, Body};
	for (int32 I = History.Num() - 1; I >= 0; --I)
	{
		const FTrailSample& Before = History[I];
		if (Before.Distance <= Target)
		{
			const float Alpha = float((Target - Before.Distance) / FMath::Max(.001, After.Distance - Before.Distance));
			FTransform Result;
			Result.Blend(Before.Pose, After.Pose, Alpha);
			return Result;
		}
		After = Before;
	}
	return Body;
}

FTransform FCoreMorphFlightMotion::PiecePose(const FCoreMorphVisualPiece& Piece, const FVector& Scale) const
{
	FVector Local = Piece.Position;
	FRotator Bend = FRotator::ZeroRotator;
	FTransform Frame = Body;
	if (Piece.Kind == TEXT("Wing"))
	{
		const float Side = Local.Y < 0 ? -1.f : 1.f;
		const float Span = FMath::Clamp(float(FMath::Abs(Local.Y) / 2540.), 0.f, 1.f);
		const float Wave = Phase - 1.8f * Span + .25f * float(Local.X / 1000.) + Side * .15f * Asymmetry;
		const float Gain = Activity * Stroke * (1.f + .2f * Noise + Side * .1f * Asymmetry);
		Local.Z += 780.f * Gain * FMath::Pow(Span, 1.7f) * FMath::Sin(Wave) - 300.f * Dive * Span * Span;
		Local.Z += 350.f * PowerStroke * Span * Span * (1.f - Dive);
		Local.Y -= Side * 180.f * Gain * Span * Span * FMath::Square(FMath::Sin(Wave));
		Local.X -= 600.f * Dive * Span * Span;
		Bend.Roll = Side * (32.f * Gain * Span * FMath::Sin(Wave) - 16.f * Dive * Span);
		Bend.Yaw = Side * 12.f * Dive * Span;
		Bend.Pitch = 6.f * Gain * Span * FMath::Cos(Wave);
	}
	else if (Piece.Kind == TEXT("Tail"))
	{
		// Sample actual distance history, including both horizontal and vertical
		// turns. No circle-radius assumption and no time-based turn/dive gates.
		const float Behind = FMath::Max(0.f, float(-Local.X * Scale.X));
		Frame = TrailPose(Behind);
		Local.X += Behind / FMath::Max(.001, Scale.X);
		const float S = FMath::Clamp(Piece.Order, 0.f, 1.f);
		const float Wave = Phase - 1.1f - 3.f * S;
		const float Gain = Activity * (1.f + .2f * Noise);
		Local.Y += (55.f + 185.f * Movement) * Gain * S * S * FMath::Sin(Wave);
		Local.Z += (130.f * Movement + 600.f * Climb) * Gain * S * S * FMath::Sin(Wave - .8f);
		Bend.Yaw = 8.f * Movement * Gain * S * FMath::Cos(Wave);
		Bend.Pitch = (5.f * Movement + 12.f * Climb) * Gain * S * FMath::Cos(Wave - .8f);
	}
	// Rigid core/body/attachments retain their assembly relationship, sharing
	// the same movement-derived body pose instead of receiving separate noise.
	return FTransform(Frame.GetRotation() * Bend.Quaternion(), Frame.TransformPosition(Local * Scale), Scale);
}
