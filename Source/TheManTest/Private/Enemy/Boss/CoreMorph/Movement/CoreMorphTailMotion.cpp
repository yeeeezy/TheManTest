#include "Enemy/Boss/CoreMorph/Movement/CoreMorphTailMotion.h"

void FCoreMorphTailMotion::Update(const FVector& WorldVelocity, float DeltaSeconds)
{
	if (DeltaSeconds <= 0.f || !FMath::IsFinite(DeltaSeconds) || WorldVelocity.ContainsNaN()) return;
	// Positive world Z means climbing even when the body is banked. A time constant
	// prevents chatter at the crest; tiny vertical drift cannot produce a full stroke.
	const float Blend = 1.f - FMath::Exp(-DeltaSeconds / .3f);
	Movement = FMath::Lerp(Movement, FMath::Clamp(float(WorldVelocity.Size() / 11000.), 0.f, 1.f), Blend);
	Climb = FMath::Lerp(Climb, FMath::Clamp(float(WorldVelocity.Z / 8000.), 0.f, 1.f), Blend);
	Phase = FMath::Fmod(Phase + DeltaSeconds * 2.f * PI * .85f, 2.f * PI);
}

void FCoreMorphTailMotion::Apply(float TailOrder, FVector& LocalPosition, FRotator& LocalBend) const
{
	const float S = FMath::Clamp(TailOrder, 0.f, 1.f);
	const float Wave = Phase - 1.1f - 3.f * S;
	LocalPosition.Y += 240.f * Movement * S * S * FMath::Sin(Wave);
	// Climbing strengthens the travelling vertical wave; level/descent returns
	// smoothly to the small cruising stroke. The root stays anchored at S == 0.
	LocalPosition.Z += (130.f * Movement + 600.f * Climb) * S * S * FMath::Sin(Wave - .8f);
	LocalBend.Yaw += 8.f * Movement * S * FMath::Cos(Wave);
	LocalBend.Pitch += (5.f * Movement + 12.f * Climb) * S * FMath::Cos(Wave - .8f);
}
