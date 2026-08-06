#include "Core/Editor/TheManAnimationAssetLibrary.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/SceneComponent.h"

#if WITH_EDITOR
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "Engine/InheritableComponentHandler.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimGraphNode_ControlRig.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimationGraph.h"
#include "AnimationGraphSchema.h"
#include "AnimationBlueprintLibrary.h"
#include "AssetToolsModule.h"
#include "ControlRigBlueprintLegacy.h"
#include "Factories/AnimBlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#endif

bool UTheManAnimationAssetLibrary::AddAnimationAssetOverride(
	UAnimBlueprint* AnimBlueprint,
	UAnimationAsset* Target,
	UAnimationAsset* Override)
{
#if WITH_EDITOR
	if (!AnimBlueprint || !Target || !Override)
	{
		return false;
	}
	UAnimationBlueprintLibrary::AddNodeAssetOverride(AnimBlueprint, Target, Override, true);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(AnimBlueprint);
	AnimBlueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::SetInheritedSceneComponentRotation(
	UBlueprint* Blueprint,
	FName ComponentVariableName,
	FRotator RelativeRotation)
{
#if WITH_EDITOR
	if (!Blueprint || ComponentVariableName.IsNone())
	{
		return false;
	}
	UInheritableComponentHandler* Handler = Blueprint->GetInheritableComponentHandler(true);
	if (!Handler)
	{
		return false;
	}
	const FComponentKey Key = Handler->FindKey(ComponentVariableName);
	if (!Key.IsValid())
	{
		if (!Blueprint->GeneratedClass)
		{
			return false;
		}
		AActor* DefaultActor = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());
		if (!DefaultActor)
		{
			return false;
		}
		USceneComponent* DefaultComponent = nullptr;
		for (UActorComponent* Component : DefaultActor->GetComponents())
		{
			if (Component && Component->GetFName() == ComponentVariableName)
			{
				DefaultComponent = Cast<USceneComponent>(Component);
				break;
			}
		}
		if (!DefaultComponent)
		{
			return false;
		}
		DefaultComponent->Modify();
		DefaultComponent->SetRelativeRotation(RelativeRotation);
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		Blueprint->MarkPackageDirty();
		return true;
	}
	UActorComponent* Template = Handler->GetOverridenComponentTemplate(Key);
	if (!Template)
	{
		Template = Handler->CreateOverridenComponentTemplate(Key);
	}
	USceneComponent* SceneTemplate = Cast<USceneComponent>(Template);
	if (!SceneTemplate)
	{
		return false;
	}
	SceneTemplate->Modify();
	SceneTemplate->SetRelativeRotation(RelativeRotation);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::AssignAnimationSkeleton(UAnimationAsset* AnimationAsset, USkeleton* Skeleton)
{
#if WITH_EDITOR
	if (!AnimationAsset || !Skeleton) return false;
	AnimationAsset->Modify();
	AnimationAsset->SetSkeleton(Skeleton);
	AnimationAsset->ValidateSkeleton();
	AnimationAsset->MarkPackageDirty();
	return AnimationAsset->GetSkeleton() == Skeleton;
#else
	return false;
#endif
}

UAnimBlueprint* UTheManAnimationAssetLibrary::CreateControlRigAnimBlueprint(
	const FString& PackagePath,
	const FString& AssetName,
	UAnimSequence* SourceAnimation,
	UControlRigBlueprint* ControlRigBlueprint)
{
#if WITH_EDITOR
	if (!SourceAnimation || !SourceAnimation->GetSkeleton() || !ControlRigBlueprint)
	{
		return nullptr;
	}

	UAnimBlueprintFactory* Factory = NewObject<UAnimBlueprintFactory>();
	Factory->TargetSkeleton = SourceAnimation->GetSkeleton();
	Factory->ParentClass = UAnimInstance::StaticClass();
	UAnimBlueprint* AnimBlueprint = Cast<UAnimBlueprint>(
		FAssetToolsModule::GetModule().Get().CreateAsset(AssetName, PackagePath, UAnimBlueprint::StaticClass(), Factory));
	if (!AnimBlueprint)
	{
		return nullptr;
	}

	UEdGraph* AnimGraph = nullptr;
	for (UEdGraph* Graph : AnimBlueprint->FunctionGraphs)
	{
		if (Graph && Graph->GetFName() == UEdGraphSchema_K2::GN_AnimGraph)
		{
			AnimGraph = Graph;
			break;
		}
	}
	if (!AnimGraph)
	{
		return nullptr;
	}

	UAnimGraphNode_Root* RootNode = nullptr;
	for (UEdGraphNode* GraphNode : AnimGraph->Nodes)
	{
		if (UAnimGraphNode_Root* Candidate = Cast<UAnimGraphNode_Root>(GraphNode))
		{
			RootNode = Candidate;
			break;
		}
	}
	if (!RootNode)
	{
		return nullptr;
	}

	FGraphNodeCreator<UAnimGraphNode_SequencePlayer> SequenceCreator(*AnimGraph);
	UAnimGraphNode_SequencePlayer* SequenceNode = SequenceCreator.CreateNode();
	SequenceNode->SetAnimationAsset(SourceAnimation);
	SequenceNode->Node.SetLoopAnimation(true);
	SequenceNode->NodePosX = -750;
	SequenceNode->NodePosY = -100;
	SequenceCreator.Finalize();

	// The authored walk already contains a full stepping cycle. Feeding those animated
	// leg chains into Locomotor makes both systems lift the same feet and produces the
	// crossed/high-leg pose. Keep the walk for body/head motion, but replace the six
	// support-leg chains with the walk's all-feet-down frame before procedural stepping.
	FGraphNodeCreator<UAnimGraphNode_SequencePlayer> GroundPoseCreator(*AnimGraph);
	UAnimGraphNode_SequencePlayer* GroundPoseNode = GroundPoseCreator.CreateNode();
	GroundPoseNode->SetAnimationAsset(SourceAnimation);
	GroundPoseNode->Node.SetLoopAnimation(false);
	GroundPoseNode->Node.SetStartPosition(0.0f);
	GroundPoseNode->Node.SetPlayRate(0.0f);
	GroundPoseNode->NodePosX = -750;
	GroundPoseNode->NodePosY = 150;
	GroundPoseCreator.Finalize();

	FGraphNodeCreator<UAnimGraphNode_LayeredBoneBlend> BlendCreator(*AnimGraph);
	UAnimGraphNode_LayeredBoneBlend* BlendNode = BlendCreator.CreateNode();
	BlendNode->NodePosX = -500;
	BlendNode->NodePosY = 0;
	BlendCreator.Finalize();
	BlendNode->Node.LayerSetup.SetNum(1);
	for (const TCHAR* LegRoot : {
		TEXT("tent_large_forward3_left1"), TEXT("tent_large_forward3_right1"),
		TEXT("tent_large_back2_left1"), TEXT("tent_large_back2_right1"),
		TEXT("tent_large_back_left1"), TEXT("tent_large_back_right1")})
	{
		FBranchFilter& Filter = BlendNode->Node.LayerSetup[0].BranchFilters.AddDefaulted_GetRef();
		Filter.BoneName = LegRoot;
		Filter.BlendDepth = 0;
	}

	FGraphNodeCreator<UAnimGraphNode_ControlRig> RigCreator(*AnimGraph);
	UAnimGraphNode_ControlRig* RigNode = RigCreator.CreateNode();
	RigNode->Node.SetControlRigClass(TSubclassOf<UControlRig>(ControlRigBlueprint->GeneratedClass.Get()));
	RigNode->NodePosX = -200;
	RigNode->NodePosY = 0;
	RigCreator.Finalize();
	if (UEdGraphPin* AlphaPin = RigNode->FindPin(TEXT("Alpha")))
	{
		AlphaPin->DefaultValue = TEXT("1.0");
	}

	const UEdGraphSchema* Schema = AnimGraph->GetSchema();
	UEdGraphPin* SequencePose = SequenceNode->FindPin(TEXT("Pose"));
	UEdGraphPin* GroundPose = GroundPoseNode->FindPin(TEXT("Pose"));
	UEdGraphPin* BlendBase = BlendNode->FindPin(TEXT("BasePose"));
	UEdGraphPin* BlendLegs = BlendNode->FindPin(TEXT("BlendPoses_0"));
	UEdGraphPin* BlendWeight = BlendNode->FindPin(TEXT("BlendWeights_0"));
	UEdGraphPin* BlendPose = BlendNode->FindPin(TEXT("Pose"));
	UEdGraphPin* RigSource = RigNode->FindPin(TEXT("Source"));
	UEdGraphPin* RigPose = RigNode->FindPin(TEXT("Pose"));
	UEdGraphPin* RootResult = RootNode->FindPin(TEXT("Result"));
	if (BlendWeight)
	{
		BlendWeight->DefaultValue = TEXT("1.0");
	}
	if (!SequencePose || !GroundPose || !BlendBase || !BlendLegs || !BlendPose
		|| !RigSource || !RigPose || !RootResult
		|| !Schema->TryCreateConnection(SequencePose, BlendBase)
		|| !Schema->TryCreateConnection(GroundPose, BlendLegs)
		|| !Schema->TryCreateConnection(BlendPose, RigSource)
		|| !Schema->TryCreateConnection(RigPose, RootResult))
	{
		return nullptr;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(AnimBlueprint);
	AnimBlueprint->MarkPackageDirty();
	return AnimBlueprint;
#else
	return nullptr;
#endif
}
