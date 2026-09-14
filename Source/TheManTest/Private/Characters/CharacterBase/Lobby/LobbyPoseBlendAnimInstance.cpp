#include "Characters/CharacterBase/Lobby/LobbyPoseBlendAnimInstance.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "Components/SkeletalMeshComponent.h"

struct FLobbyPoseBlendProxy : public FAnimSingleNodeInstanceProxy
{
	explicit FLobbyPoseBlendProxy(UAnimInstance* Instance) : FAnimSingleNodeInstanceProxy(Instance) {}
	TArray<FTransform> SourcePose;
	float Alpha = 1.f;

	virtual bool Evaluate(FPoseContext& Output) override
	{
		const bool bEvaluated = FAnimSingleNodeInstanceProxy::Evaluate(Output);
		if (bEvaluated && Alpha < 1.f)
		{
			const FBoneContainer& Bones = Output.Pose.GetBoneContainer();
			for (FCompactPoseBoneIndex Index : Output.Pose.ForEachBoneIndex())
			{
				const int32 MeshIndex = Bones.MakeMeshPoseIndex(Index).GetInt();
				if (SourcePose.IsValidIndex(MeshIndex))
				{
					FTransform Blended;
					Blended.Blend(SourcePose[MeshIndex], Output.Pose[Index], Alpha);
					Output.Pose[Index] = Blended;
				}
			}
		}
		return bEvaluated;
	}
};

FAnimInstanceProxy* ULobbyPoseBlendAnimInstance::CreateAnimInstanceProxy()
{
	return new FLobbyPoseBlendProxy(this);
}

bool ULobbyPoseBlendAnimInstance::PlayPose(UAnimationAsset* Animation, bool bBlend)
{
	if (GetCurrentAsset() == Animation) return false;
	FLobbyPoseBlendProxy& Proxy = GetProxyOnGameThread<FLobbyPoseBlendProxy>();
	bBlend = bBlend && GetCurrentAsset() && Animation && GetSkelMeshComponent();
	Proxy.SourcePose = bBlend ? GetSkelMeshComponent()->GetBoneSpaceTransforms() : TArray<FTransform>();
	Proxy.Alpha = bBlend ? 0.f : 1.f;
	SetAnimationAsset(Animation, true);
	SetPosition(0.f, false);
	SetPlaying(true);
	return bBlend;
}

void ULobbyPoseBlendAnimInstance::SetPoseBlendAlpha(float Alpha)
{
	FLobbyPoseBlendProxy& Proxy = GetProxyOnGameThread<FLobbyPoseBlendProxy>();
	Proxy.Alpha = Alpha;
	if (Alpha >= 1.f) Proxy.SourcePose.Reset();
}
