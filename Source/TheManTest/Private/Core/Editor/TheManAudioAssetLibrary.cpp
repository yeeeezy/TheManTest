#include "Core/Editor/TheManAudioAssetLibrary.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"

#if WITH_EDITOR
#include "Sound/SoundNodeModulator.h"
#include "Sound/SoundNodeRandom.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "EdGraph/EdGraphNode.h"
#endif

bool UTheManAudioAssetLibrary::InitializeVariationCue(USoundCue* Cue, const TArray<USoundWave*>& Waves,
	float PitchMin, float PitchMax, float VolumeMin, float VolumeMax)
{
#if WITH_EDITOR
	if (!Cue || Cue->FirstNode || Waves.IsEmpty() || Waves.Contains(nullptr)
		|| PitchMin <= 0.f || PitchMax < PitchMin || VolumeMin < 0.f || VolumeMax < VolumeMin)
	{
		return false;
	}
	Cue->Modify();
	// SoundCue's engine default is 0.75; wrapping must not silently reduce existing gain.
	Cue->VolumeMultiplier = 1.f;
	Cue->PitchMultiplier = 1.f;
	auto* Modulator = Cue->ConstructSoundNode<USoundNodeModulator>();
	Modulator->PitchMin = PitchMin;
	Modulator->PitchMax = PitchMax;
	Modulator->VolumeMin = VolumeMin;
	Modulator->VolumeMax = VolumeMax;
	Cue->FirstNode = Modulator;
	Modulator->GraphNode->NodePosX = -200;
	USoundNode* Parent = Modulator;
	if (Waves.Num() > 1)
	{
		auto* Random = Cue->ConstructSoundNode<USoundNodeRandom>();
		Random->bRandomizeWithoutReplacement = true;
		Random->Weights.Init(1.f, Waves.Num());
		Random->HasBeenUsed.Init(false, Waves.Num());
		Random->ChildNodes.Reset();
		Random->GraphNode->NodePosX = -400;
		Modulator->ChildNodes = {Random};
		Parent = Random;
	}
	Parent->ChildNodes.Reset();
	for (int32 Index = 0; Index < Waves.Num(); ++Index)
	{
		auto* Player = Cue->ConstructSoundNode<USoundNodeWavePlayer>();
		Player->SetSoundWave(Waves[Index]);
		Player->bLooping = false;
		Player->GraphNode->NodePosX = Waves.Num() > 1 ? -600 : -400;
		Player->GraphNode->NodePosY = Index * 140;
		Parent->ChildNodes.Add(Player);
	}
	// Nodes are initially constructed with no children; rebuild editor pins after wiring.
	for (USoundNode* Node : Cue->AllNodes)
	{
		Node->GraphNode->ReconstructNode();
	}
	Cue->LinkGraphNodesFromSoundNodes();
	Cue->CompileSoundNodesFromGraphNodes();
	Cue->PostEditChange();
	Cue->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}
