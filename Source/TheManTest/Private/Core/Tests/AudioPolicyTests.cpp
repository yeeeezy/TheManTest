#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNodeModulator.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundAttenuation.h"
#include "Core/Editor/TheManAudioAssetLibrary.h"
#include "Sound/SoundNodeRandom.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAudioPolicyTest,"TheManTest.Audio.AssetVariationPolicy",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAudioPolicyTest::RunTest(const FString&)
{
	TArray<FString> Paths;
	for (const TCHAR* Weapon : {TEXT("RepairGun"), TEXT("ElectricGun"), TEXT("ExplosionGun")})
		for (const TCHAR* Kind : {TEXT("Fire"), TEXT("DryFire"), TEXT("Impact")})
			Paths.Add(FString::Printf(TEXT("/Game/Weapons/%s/Audio/SCue_%s_%s"), Weapon, Weapon, Kind));
	Paths.Append({TEXT("/Game/Enemy/_Shared/Audio/SCue_Enemy_FleshHit"), TEXT("/Game/Weapons/ExplosionGun/Audio/SCue_ExplosionGun_Detonation"), TEXT("/Game/Weapons/TestGun/Audio/SCue_TestGun_Fire")});
	for (const FString& Path : Paths)
	{
		auto* Cue = LoadObject<USoundCue>(nullptr, *Path);
		if (!TestNotNull(*Path, Cue)) continue;
		auto* Mod = Cast<USoundNodeModulator>(Cue->FirstNode);
		if (!TestNotNull(TEXT("Random modulation is the connected root"), Mod)) continue;
		TestTrue(TEXT("Pitch varies gently around one"), Mod->PitchMin >= .9f && Mod->PitchMin < 1.f && Mod->PitchMax > 1.f && Mod->PitchMax <= 1.1f);
		TestTrue(TEXT("Random volume cannot amplify source peaks"), Mod->VolumeMin > 0.f && Mod->VolumeMin < Mod->VolumeMax && Mod->VolumeMax <= 1.f);
		TestEqual(TEXT("Wrapper preserves nominal gain"), Cue->VolumeMultiplier, 1.f);
		TestEqual(TEXT("One original source connected"), Mod->ChildNodes.Num(), 1);
		auto* Player = Mod->ChildNodes.Num() == 1 ? Cast<USoundNodeWavePlayer>(Mod->ChildNodes[0]) : nullptr;
		if (TestNotNull(TEXT("Modulator connects to Wave Player"), Player))
		{
			Player->LoadAsset(false);
			TestNotNull(TEXT("Original source wave is loadable"), Player->GetSoundWave());
			TestFalse(TEXT("One-shot never loops"), bool(Player->bLooping));
		}
		const auto* Att = Cue->GetAttenuationSettingsToApply();
		if (TestNotNull(TEXT("Purpose-specific attenuation assigned"), Att))
			TestTrue(TEXT("3D and distance enabled"), Att->bAttenuate && Att->bSpatialize);
		TestFalse(TEXT("Concurrency is explicit"), Cue->ConcurrencySet.IsEmpty());
	}
	// The authoring helper also supports future multi-sample sounds without replacement.
	auto* Wave = LoadObject<USoundWave>(nullptr, TEXT("/Game/Enemy/_Shared/Audio/S_Enemy_FleshHit"));
	auto* Scratch = NewObject<USoundCue>();
	TestTrue(TEXT("Future multi-sample Cue can be authored"), UTheManAudioAssetLibrary::InitializeVariationCue(Scratch, {Wave, Wave}));
	auto* Mod = Cast<USoundNodeModulator>(Scratch->FirstNode);
	auto* Random = Mod && Mod->ChildNodes.Num() == 1 ? Cast<USoundNodeRandom>(Mod->ChildNodes[0]) : nullptr;
	if (TestNotNull(TEXT("Multi-sample random selector connected"), Random))
	{
		TestEqual(TEXT("Both variants connected"), Random->ChildNodes.Num(), 2);
		TestTrue(TEXT("Variants play without replacement"), bool(Random->bRandomizeWithoutReplacement));
	}
	TestFalse(TEXT("Helper will not overwrite authored graphs"), UTheManAudioAssetLibrary::InitializeVariationCue(Scratch, {Wave}));
	return true;
}
#endif
