#include "Core/_Shared/Feedback/HitStopSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/PlatformTime.h"

float FHitStopSettings::GetStrength(float Distance) const
{
	const float Inner = FMath::Max(0.f, InnerRadius);
	const float Outer = FMath::Max(Inner + 1.f, OuterRadius);
	return bEnabled ? 1.f - FMath::Clamp((Distance - Inner) / (Outer - Inner), 0.f, 1.f) : 0.f;
}

bool UHitStopSubsystem::RequestHitStopAtLocation(FVector Location, const FHitStopSettings& Settings)
{
	float Strength = 0.f;
	for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		auto* PC = It->Get();
		if (PC && PC->IsLocalController() && PC->PlayerCameraManager)
			Strength = FMath::Max(Strength, Settings.GetStrength(FVector::Distance(Location, PC->PlayerCameraManager->GetCameraLocation())));
	}
	return Strength > UE_KINDA_SMALL_NUMBER && RequestHitStop(Settings.Duration * Strength,
		FMath::Lerp(1.f, Settings.TimeScale, Strength), Settings.MaxContinuousDuration);
}

bool UHitStopSubsystem::RequestHitStop(float Duration, float TimeScale, float MaxContinuousDuration)
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() != NM_Standalone || !FMath::IsFinite(Duration) || !FMath::IsFinite(TimeScale)
		|| !FMath::IsFinite(MaxContinuousDuration) || Duration <= 0.f || TimeScale >= 1.f || MaxContinuousDuration <= 0.f) return false;
	const double Now = FPlatformTime::Seconds();
	if (bActive) Tick(0.f); // Finish expired requests even when a new explosion precedes this frame's tick.
	if (Now < RecoveryUntil) return false;
	auto* WS = World->GetWorldSettings();
	if (!bActive)
	{
		OriginalDilation = WS->TimeDilation;
		StartRealTime = Now;
		CapRealTime = Now + FMath::Clamp(MaxContinuousDuration, 0.f, .5f);
		EndRealTime = FMath::Min(CapRealTime, Now + Duration);
		AppliedDilation = WS->SetTimeDilation(OriginalDilation * FMath::Clamp(TimeScale, .001f, 1.f));
		bActive = true;
	}
	else
	{
		// Strongest wins, extensions never move the beginning of the continuous interval.
		CapRealTime = FMath::Min(CapRealTime, StartRealTime + FMath::Clamp(MaxContinuousDuration, 0.f, .5f));
		EndRealTime = FMath::Min(CapRealTime, FMath::Max(EndRealTime, Now + Duration));
		AppliedDilation = WS->SetTimeDilation(FMath::Min(AppliedDilation, OriginalDilation * FMath::Clamp(TimeScale, .001f, 1.f)));
	}
	return true;
}

void UHitStopSubsystem::Tick(float DeltaTime)
{
	if (!bActive) return;
	const double Now = FPlatformTime::Seconds();
	// Do not overwrite another system's newer time-dilation setting.
	if (!FMath::IsNearlyEqual(GetWorld()->GetWorldSettings()->TimeDilation, AppliedDilation))
	{
		bActive = false;
		RecoveryUntil = Now + .05;
		return;
	}
	if (Now >= EndRealTime)
	{
		CancelHitStop();
		// A short real-time recovery gap prevents sustained explosions from holding the world slow forever.
		RecoveryUntil = Now + .05;
	}
}

void UHitStopSubsystem::CancelHitStop()
{
	if (bActive && GetWorld())
	{
		auto* WS = GetWorld()->GetWorldSettings();
		if (WS && FMath::IsNearlyEqual(WS->TimeDilation, AppliedDilation)) WS->SetTimeDilation(OriginalDilation);
	}
	bActive = false;
}

void UHitStopSubsystem::OnWorldEndPlay(UWorld& World) { CancelHitStop(); Super::OnWorldEndPlay(World); }
void UHitStopSubsystem::Deinitialize() { CancelHitStop(); Super::Deinitialize(); }
