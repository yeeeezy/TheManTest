#include "Characters/CharacterBase/Lobby/LobbyCharacterAnimInstance.h"
#include "Characters/CharacterBase/Lobby/LobbyCharacterBase.h"
#include "Animation/AnimSingleNodeInstanceProxy.h"
#include "BonePose.h"
#include "TwoBoneIK.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{
struct FLobbyCharacterAnimInstanceProxy : FAnimSingleNodeInstanceProxy
{
	explicit FLobbyCharacterAnimInstanceProxy(UAnimInstance* Instance) : FAnimSingleNodeInstanceProxy(Instance) {}

	FTransform LeftTarget;
	FName RightHand;
	FName LeftHand;
	bool bApplyGrip = false;

	virtual void PreUpdate(UAnimInstance* Instance, float DeltaSeconds) override
	{
		FAnimSingleNodeInstanceProxy::PreUpdate(Instance, DeltaSeconds);
		const ALobbyCharacterBase* Actor = Cast<ALobbyCharacterBase>(Instance->GetOwningActor());
		bApplyGrip = Actor && Actor->GetLeftHandTarget(LeftTarget);
		if (Actor)
		{
			RightHand = Actor->DisplayMesh->GetSocketBoneName(Actor->WeaponAttachSocket);
			LeftHand = Actor->LeftHandBone;
		}
	}

	virtual bool Evaluate(FPoseContext& Output) override
	{
		const bool bEvaluated = FAnimSingleNodeInstanceProxy::Evaluate(Output);
		if (!bApplyGrip) return bEvaluated;

		const FBoneContainer& Bones = Output.Pose.GetBoneContainer();
		auto FindBone = [&Bones](FName Name)
		{
			FBoneReference Reference(Name);
			Reference.Initialize(Bones);
			return Reference.GetCompactPoseIndex(Bones);
		};
		const FCompactPoseBoneIndex Right = FindBone(RightHand);
		const FCompactPoseBoneIndex Hand = FindBone(LeftHand);
		if (Right == INDEX_NONE || Hand == INDEX_NONE) return bEvaluated;
		const FCompactPoseBoneIndex Elbow = Output.Pose.GetParentBoneIndex(Hand);
		if (Elbow == INDEX_NONE) return bEvaluated;
		const FCompactPoseBoneIndex Shoulder = Output.Pose.GetParentBoneIndex(Elbow);
		if (Shoulder == INDEX_NONE) return bEvaluated;

		FCSPose<FCompactPose> ComponentPose;
		ComponentPose.InitPose(Output.Pose);
		FTransform Upper = ComponentPose.GetComponentSpaceTransform(Shoulder);
		FTransform Lower = ComponentPose.GetComponentSpaceTransform(Elbow);
		FTransform Wrist = ComponentPose.GetComponentSpaceTransform(Hand);
		const FTransform Target = LeftTarget * ComponentPose.GetComponentSpaceTransform(Right);
		const FCompactPoseBoneIndex ShoulderParent = Output.Pose.GetParentBoneIndex(Shoulder);
		const FTransform Parent = ShoulderParent == INDEX_NONE ? FTransform::Identity : ComponentPose.GetComponentSpaceTransform(ShoulderParent);

		// The original elbow defines the bend plane. Never stretch the arm to reach a bad grip configuration.
		AnimationCore::SolveTwoBoneIK(Upper, Lower, Wrist, Lower.GetLocation(), Target.GetLocation(), false, 1.0, 1.0);
		Wrist.SetRotation(Target.GetRotation());
		Output.Pose[Shoulder] = Upper.GetRelativeTransform(Parent);
		Output.Pose[Elbow] = Lower.GetRelativeTransform(Upper);
		Output.Pose[Hand] = Wrist.GetRelativeTransform(Lower);
		Output.Pose.NormalizeRotations();
		return bEvaluated;
	}
};
}

FAnimInstanceProxy* ULobbyCharacterAnimInstance::CreateAnimInstanceProxy()
{
	return new FLobbyCharacterAnimInstanceProxy(this);
}
