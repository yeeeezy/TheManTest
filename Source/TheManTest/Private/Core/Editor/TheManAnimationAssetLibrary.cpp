#include "Core/Editor/TheManAnimationAssetLibrary.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"

#if WITH_EDITOR
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "Engine/InheritableComponentHandler.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimGraphNode_ControlRig.h"
#include "AnimGraphNode_CopyPoseFromMesh.h"
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
#include "K2Node_VariableGet.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorTabs.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"
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

#if WITH_EDITOR
static UActorComponent* GetOrCreateInheritedComponentTemplate(
	UBlueprint* Blueprint,
	FName ComponentVariableName)
{
	if (!Blueprint || ComponentVariableName.IsNone())
	{
		return nullptr;
	}
	UInheritableComponentHandler* Handler = Blueprint->GetInheritableComponentHandler(true);
	if (!Handler)
	{
		return nullptr;
	}
	const FComponentKey Key = Handler->FindKey(ComponentVariableName);
	if (Key.IsValid())
	{
		if (UActorComponent* Existing = Handler->GetOverridenComponentTemplate(Key))
		{
			return Existing;
		}
		return Handler->CreateOverridenComponentTemplate(Key);
	}
	if (!Blueprint->GeneratedClass)
	{
		return nullptr;
	}
	AActor* DefaultActor = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());
	if (!DefaultActor)
	{
		return nullptr;
	}
	for (UActorComponent* Component : DefaultActor->GetComponents())
	{
		if (Component && Component->GetFName() == ComponentVariableName)
		{
			return Component;
		}
	}
	return nullptr;
}
#endif

bool UTheManAnimationAssetLibrary::SetInheritedSceneComponentTransform(
	UBlueprint* Blueprint,
	FName ComponentVariableName,
	FVector RelativeLocation,
	FRotator RelativeRotation,
	FVector RelativeScale)
{
#if WITH_EDITOR
	USceneComponent* SceneTemplate = Cast<USceneComponent>(
		GetOrCreateInheritedComponentTemplate(Blueprint, ComponentVariableName));
	if (!SceneTemplate)
	{
		return false;
	}
	SceneTemplate->Modify();
	SceneTemplate->SetRelativeLocation(RelativeLocation);
	SceneTemplate->SetRelativeRotation(RelativeRotation);
	SceneTemplate->SetRelativeScale3D(RelativeScale);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::SetInheritedSkeletalMesh(
	UBlueprint* Blueprint,
	FName ComponentVariableName,
	USkeletalMesh* SkeletalMesh)
{
#if WITH_EDITOR
	USkeletalMeshComponent* MeshTemplate = Cast<USkeletalMeshComponent>(
		GetOrCreateInheritedComponentTemplate(Blueprint, ComponentVariableName));
	if (!MeshTemplate)
	{
		return false;
	}
	MeshTemplate->Modify();
	MeshTemplate->SetSkeletalMeshAsset(SkeletalMesh);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::SetInheritedAnimClass(
	UBlueprint* Blueprint,
	FName ComponentVariableName,
	TSubclassOf<UAnimInstance> AnimClass)
{
#if WITH_EDITOR
	USkeletalMeshComponent* MeshTemplate = Cast<USkeletalMeshComponent>(
		GetOrCreateInheritedComponentTemplate(Blueprint, ComponentVariableName));
	if (!MeshTemplate || !AnimClass)
	{
		return false;
	}
	MeshTemplate->Modify();
	MeshTemplate->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	MeshTemplate->SetAnimInstanceClass(AnimClass);
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	Blueprint->MarkPackageDirty();
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::SetBlueprintForceFullEditor(
	UBlueprint* Blueprint,
	bool bForceFullEditor)
{
#if WITH_EDITOR
	if (!Blueprint)
	{
		return false;
	}
	Blueprint->bForceFullEditor = bForceFullEditor;
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::FocusBlueprintComponentViewport(UBlueprint* Blueprint)
{
#if WITH_EDITOR
	if (!Blueprint || !GEditor)
	{
		return false;
	}
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem
		? AssetEditorSubsystem->FindEditorForAsset(Blueprint, false)
		: nullptr;
	if (!EditorInstance || EditorInstance->GetEditorName() != TEXT("BlueprintEditor"))
	{
		return false;
	}
	FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(EditorInstance);
	TSharedPtr<SDockTab> ViewportTab = BlueprintEditor->GetTabManager()->TryInvokeTab(
		FBlueprintEditorTabs::SCSViewportID);
	if (!ViewportTab.IsValid())
	{
		return false;
	}
	FSlateApplication::Get().SetKeyboardFocus(ViewportTab->GetContent(), EFocusCause::SetDirectly);
	return true;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::SetBlueprintComponentViewportView(
	UBlueprint* Blueprint,
	const FString& ViewName)
{
#if WITH_EDITOR
	if (!FocusBlueprintComponentViewport(Blueprint) || !GEditor)
	{
		return false;
	}
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	IAssetEditorInstance* EditorInstance = AssetEditorSubsystem
		? AssetEditorSubsystem->FindEditorForAsset(Blueprint, false)
		: nullptr;
	if (!EditorInstance || EditorInstance->GetEditorName() != TEXT("BlueprintEditor"))
	{
		return false;
	}
	FBlueprintEditor* BlueprintEditor = static_cast<FBlueprintEditor*>(EditorInstance);
	TSharedPtr<SDockTab> ViewportTab = BlueprintEditor->GetTabManager()->FindExistingLiveTab(
		FBlueprintEditorTabs::SCSViewportID);
	if (!ViewportTab.IsValid())
	{
		return false;
	}
	TSharedRef<SEditorViewport> EditorViewport = StaticCastSharedRef<SEditorViewport>(
		ViewportTab->GetContent());
	TSharedPtr<FEditorViewportClient> ViewportClient = EditorViewport->GetViewportClient();
	if (!ViewportClient.IsValid())
	{
		return false;
	}
	if (ViewName.Equals(TEXT("Front"), ESearchCase::IgnoreCase))
	{
		ViewportClient->SetViewportType(LVT_OrthoYZ);
	}
	else if (ViewName.Equals(TEXT("Side"), ESearchCase::IgnoreCase))
	{
		ViewportClient->SetViewportType(LVT_OrthoXZ);
	}
	else if (ViewName.Equals(TEXT("Top"), ESearchCase::IgnoreCase))
	{
		ViewportClient->SetViewportType(LVT_OrthoXY);
	}
	else
	{
		return false;
	}
	FBox PreviewBounds(ForceInit);
	if (FPreviewScene* PreviewScene = ViewportClient->GetPreviewScene())
	{
		if (UWorld* PreviewWorld = PreviewScene->GetWorld())
		{
			for (TActorIterator<AActor> It(PreviewWorld); It; ++It)
			{
				TInlineComponentArray<USkeletalMeshComponent*> MeshComponents(*It);
				for (USkeletalMeshComponent* MeshComponent : MeshComponents)
				{
					if (MeshComponent && MeshComponent->GetSkeletalMeshAsset())
					{
						PreviewBounds += MeshComponent->Bounds.GetBox();
					}
				}
			}
		}
	}
	if (PreviewBounds.IsValid)
	{
		ViewportClient->FocusViewportOnBox(PreviewBounds, true);
	}
	ViewportClient->Invalidate();
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

bool UTheManAnimationAssetLibrary::ConfigureFirstPersonUpperBodyCopy(
	UAnimBlueprint* AnimBlueprint,
	FName SourcePropertyName,
	FName BlendBoneName)
{
#if WITH_EDITOR
	if (!AnimBlueprint || SourcePropertyName.IsNone() || BlendBoneName.IsNone())
	{
		return false;
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
		return false;
	}

	// Idempotence: an existing Copy Pose node means the composition was already authored.
	for (UEdGraphNode* GraphNode : AnimGraph->Nodes)
	{
		if (Cast<UAnimGraphNode_CopyPoseFromMesh>(GraphNode))
		{
			return true;
		}
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
	UEdGraphPin* RootResult = RootNode ? RootNode->FindPin(TEXT("Result")) : nullptr;
	if (!RootResult || RootResult->LinkedTo.Num() != 1)
	{
		return false;
	}
	UEdGraphPin* ExistingBodyPose = RootResult->LinkedTo[0];

	FGraphNodeCreator<UAnimGraphNode_CopyPoseFromMesh> CopyCreator(*AnimGraph);
	UAnimGraphNode_CopyPoseFromMesh* CopyNode = CopyCreator.CreateNode();
	CopyNode->Node.bUseAttachedParent = false;
	CopyNode->Node.bCopyCurves = true;
	CopyNode->Node.bCopyCustomAttributes = true;
	CopyNode->Node.bUseMeshPose = false;
	CopyNode->Node.RootBoneToCopy = BlendBoneName;
	CopyNode->NodePosX = RootNode->NodePosX - 450;
	CopyNode->NodePosY = RootNode->NodePosY + 250;
	CopyCreator.Finalize();

	FGraphNodeCreator<UK2Node_VariableGet> VariableCreator(*AnimGraph);
	UK2Node_VariableGet* SourceVariable = VariableCreator.CreateNode();
	SourceVariable->VariableReference.SetSelfMember(SourcePropertyName);
	SourceVariable->NodePosX = CopyNode->NodePosX - 300;
	SourceVariable->NodePosY = CopyNode->NodePosY + 100;
	VariableCreator.Finalize();

	FGraphNodeCreator<UAnimGraphNode_LayeredBoneBlend> BlendCreator(*AnimGraph);
	UAnimGraphNode_LayeredBoneBlend* BlendNode = BlendCreator.CreateNode();
	BlendNode->NodePosX = RootNode->NodePosX - 200;
	BlendNode->NodePosY = RootNode->NodePosY;
	BlendCreator.Finalize();
	BlendNode->Node.LayerSetup.SetNum(1);
	FBranchFilter& UpperBodyFilter = BlendNode->Node.LayerSetup[0].BranchFilters.AddDefaulted_GetRef();
	UpperBodyFilter.BoneName = BlendBoneName;
	UpperBodyFilter.BlendDepth = 0;
	BlendNode->Node.bMeshSpaceRotationBlend = true;
	BlendNode->Node.bMeshSpaceScaleBlend = true;

	const UEdGraphSchema* Schema = AnimGraph->GetSchema();
	UEdGraphPin* SourceValue = SourceVariable->GetValuePin();
	UEdGraphPin* SourceMesh = CopyNode->FindPin(TEXT("SourceMeshComponent"));
	UEdGraphPin* CopyPose = CopyNode->FindPin(TEXT("Pose"));
	UEdGraphPin* BlendBase = BlendNode->FindPin(TEXT("BasePose"));
	UEdGraphPin* BlendUpper = BlendNode->FindPin(TEXT("BlendPoses_0"));
	UEdGraphPin* BlendWeight = BlendNode->FindPin(TEXT("BlendWeights_0"));
	UEdGraphPin* BlendPose = BlendNode->FindPin(TEXT("Pose"));
	if (!Schema || !SourceValue || !SourceMesh || !CopyPose || !BlendBase || !BlendUpper
		|| !BlendWeight || !BlendPose)
	{
		return false;
	}

	RootResult->BreakAllPinLinks();
	BlendWeight->DefaultValue = TEXT("1.0");
	if (!Schema->TryCreateConnection(SourceValue, SourceMesh)
		|| !Schema->TryCreateConnection(ExistingBodyPose, BlendBase)
		|| !Schema->TryCreateConnection(CopyPose, BlendUpper)
		|| !Schema->TryCreateConnection(BlendPose, RootResult))
	{
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(AnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(AnimBlueprint);
	AnimBlueprint->MarkPackageDirty();
	return AnimBlueprint->Status != BS_Error;
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
