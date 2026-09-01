#include "Core/Editor/TheManAnimationAssetLibrary.h"
#include "Algo/NoneOf.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Characters/CharacterBase/Animation/CharacterBaseAnimInstance.h"
#include "Weapons/_Shared/Firearms/FirearmAnimInstance.h"

#if WITH_EDITOR
#include "Animation/AnimBlueprint.h"
#include "Engine/Blueprint.h"
#include "Engine/InheritableComponentHandler.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "AnimGraphNode_ControlRig.h"
#include "AnimGraphNode_CopyPoseFromMesh.h"
#include "AnimGraphNode_BlendListByBool.h"
#include "AnimGraphNode_LayeredBoneBlend.h"
#include "AnimGraphNode_LinkedAnimLayer.h"
#include "AnimGraphNode_LinkedInputPose.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_SequencePlayer.h"
#include "AnimGraphNode_AssetPlayerBase.h"
#include "AnimGraphNode_BlendSpacePlayer.h"
#include "AnimGraphNode_StateMachine.h"
#include "AnimGraphNode_StateResult.h"
#include "AnimGraphNode_TransitionResult.h"
#include "AnimStateTransitionNode.h"
#include "AnimationGraph.h"
#include "AnimationGraphSchema.h"
#include "AnimationBlueprintLibrary.h"
#include "AssetToolsModule.h"
#include "ControlRigBlueprintLegacy.h"
#include "Factories/AnimBlueprintFactory.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TransitionRuleGetter.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "EdGraphUtilities.h"
#include "KismetCompiler.h"
#include "BlueprintEditor.h"
#include "BlueprintEditorTabs.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Framework/Application/SlateApplication.h"
#include "SEditorViewport.h"
#include "EditorViewportClient.h"
#include "EngineUtils.h"
#endif

bool UTheManAnimationAssetLibrary::ConfigureFirstPersonFirearmLinkedLayer(
	UAnimBlueprint* HostAnimBlueprint,
	UAnimBlueprint* FirearmTemplateAnimBlueprint,
	UAnimBlueprint* ConcreteFirearmAnimBlueprint,
	UAnimBlueprint* AnimLayerInterface,
	FName LayerName)
{
#if WITH_EDITOR
	if (!HostAnimBlueprint || !FirearmTemplateAnimBlueprint || !ConcreteFirearmAnimBlueprint || !AnimLayerInterface || LayerName.IsNone()
		|| !AnimLayerInterface->GeneratedClass)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: invalid inputs host=%d template=%d interface=%d generated=%d layer=%s"), HostAnimBlueprint != nullptr, FirearmTemplateAnimBlueprint != nullptr, AnimLayerInterface != nullptr, AnimLayerInterface && AnimLayerInterface->GeneratedClass != nullptr, *LayerName.ToString());
		return false;
	}

	auto FindGraph = [](UAnimBlueprint* Blueprint, FName GraphName) -> UEdGraph*
	{
		TArray<UEdGraph*> AllGraphs;
		Blueprint->GetAllGraphs(AllGraphs);
		for (UEdGraph* Graph : AllGraphs)
		{
			if (Graph && Graph->GetFName() == GraphName)
			{
				return Graph;
			}
		}
		return nullptr;
	};

	UEdGraph* HostGraph = FindGraph(HostAnimBlueprint, UEdGraphSchema_K2::GN_AnimGraph);
	UEdGraph* TemplateLayerGraph = FindGraph(FirearmTemplateAnimBlueprint, LayerName);
	if (!HostGraph)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: missing host AnimGraph"));
		return false;
	}

	UClass* InterfaceClass = AnimLayerInterface->GeneratedClass;
	// The existing firearm template already owns the shared weapon locomotion and
	// implements ALI_WeaponAnim. Convert only the first-person host into a thin
	// router and keep its authored post-layer sway chain intact.
	if (TemplateLayerGraph)
	{
		HostAnimBlueprint->ParentClass = UCharacterBaseAnimInstance::StaticClass();
		FirearmTemplateAnimBlueprint->ParentClass = UFirearmAnimInstance::StaticClass();
		const TSet<FName> DriverNames = {
			TEXT("Is_Moving"), TEXT("Is_InAir"), TEXT("Character_Speed"),
			TEXT("Lean_Sides_Amount"), TEXT("Look_Up_Amount") };
		HostAnimBlueprint->NewVariables.RemoveAll([&DriverNames](const FBPVariableDescription& Variable)
		{
			return DriverNames.Contains(Variable.VarName);
		});
		for (UEdGraph* EventGraph : HostAnimBlueprint->UbergraphPages)
		{
			for (int32 Index = EventGraph->Nodes.Num() - 1; Index >= 0; --Index)
			{
				EventGraph->Nodes[Index]->DestroyNode();
			}
		}
		if (!FBlueprintEditorUtils::ImplementsInterface(HostAnimBlueprint, true, InterfaceClass)
			&& !FBlueprintEditorUtils::ImplementNewInterface(HostAnimBlueprint, InterfaceClass->GetClassPathName()))
		{
			return false;
		}

		UAnimGraphNode_StateMachine* StateMachine = nullptr;
		for (UEdGraphNode* Node : HostGraph->Nodes)
		{
			if (UAnimGraphNode_StateMachine* Candidate = Cast<UAnimGraphNode_StateMachine>(Node))
			{
				StateMachine = Candidate;
				break;
			}
		}
		UEdGraphPin* StatePose = StateMachine ? StateMachine->FindPin(TEXT("Pose")) : nullptr;
		if (!StatePose)
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked jump fallback: state machine pose missing"));
			return false;
		}
		for (UEdGraphNode* Node : HostGraph->Nodes)
		{
			if (Cast<UAnimGraphNode_BlendListByBool>(Node))
			{
				return true;
			}
		}
		UAnimGraphNode_LinkedAnimLayer* LayerNode = nullptr;
		for (UEdGraphNode* Node : HostGraph->Nodes)
		{
			if (UAnimGraphNode_LinkedAnimLayer* Candidate = Cast<UAnimGraphNode_LinkedAnimLayer>(Node))
			{
				if (Candidate->Node.Layer == LayerName)
				{
					LayerNode = Candidate;
					break;
				}
			}
		}
		if (!LayerNode)
		{
			FGraphNodeCreator<UAnimGraphNode_LinkedAnimLayer> Creator(*HostGraph);
			LayerNode = Creator.CreateNode();
			LayerNode->Node.Interface = InterfaceClass;
			LayerNode->Node.Layer = LayerName;
			FGuid LayerGuid;
			FBlueprintEditorUtils::GetFunctionGuidFromClassByFieldName(
				FBlueprintEditorUtils::GetMostUpToDateClass(InterfaceClass), LayerName, LayerGuid);
			if (FStructProperty* RefProperty = FindFProperty<FStructProperty>(LayerNode->GetClass(), TEXT("FunctionReference")))
			{
				RefProperty->ContainerPtrToValuePtr<FMemberReference>(LayerNode)->SetExternalMember(
					LayerName, InterfaceClass, LayerGuid);
			}
			LayerNode->NodePosX = StateMachine->NodePosX;
			LayerNode->NodePosY = StateMachine->NodePosY;
			Creator.Finalize();
			LayerNode->ReconstructNode();
		}
		UEdGraphPin* LayerPose = LayerNode->FindPin(TEXT("Pose"));
		if (!LayerPose)
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked jump fallback: layer pose missing"));
			return false;
		}
		TArray<UEdGraphPin*> Consumers = StatePose->LinkedTo;
		if (Consumers.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked jump fallback: no downstream consumers"));
			Consumers = LayerPose->LinkedTo;
		}
		if (Consumers.IsEmpty())
		{
			return false;
		}
		StatePose->BreakAllPinLinks();
		LayerPose->BreakAllPinLinks();

		FGraphNodeCreator<UAnimGraphNode_BlendListByBool> BlendCreator(*HostGraph);
		UAnimGraphNode_BlendListByBool* AirBlend = BlendCreator.CreateNode();
		AirBlend->NodePosX = LayerNode->NodePosX + 220;
		AirBlend->NodePosY = LayerNode->NodePosY;
		BlendCreator.Finalize();
		FGraphNodeCreator<UK2Node_VariableGet> FallingCreator(*HostGraph);
		UK2Node_VariableGet* FallingVariable = FallingCreator.CreateNode();
		FallingVariable->VariableReference.SetSelfMember(TEXT("bIsFalling"));
		FallingVariable->NodePosX = AirBlend->NodePosX - 220;
		FallingVariable->NodePosY = AirBlend->NodePosY + 220;
		FallingCreator.Finalize();
		UEdGraphPin* GroundPose = AirBlend->FindPin(TEXT("BlendPose_0"));
		UEdGraphPin* AirPose = AirBlend->FindPin(TEXT("BlendPose_1"));
		UEdGraphPin* ActiveValue = AirBlend->FindPin(TEXT("bActiveValue"));
		UEdGraphPin* BlendedPose = AirBlend->FindPin(TEXT("Pose"));
		if (!GroundPose || !AirPose || !ActiveValue || !BlendedPose
			|| !HostGraph->GetSchema()->TryCreateConnection(LayerPose, GroundPose)
			|| !HostGraph->GetSchema()->TryCreateConnection(StatePose, AirPose)
			|| !HostGraph->GetSchema()->TryCreateConnection(FallingVariable->GetValuePin(), ActiveValue))
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked jump fallback: blend pins/connections failed ground=%d air=%d active=%d output=%d"), GroundPose != nullptr, AirPose != nullptr, ActiveValue != nullptr, BlendedPose != nullptr);
			return false;
		}
		for (UEdGraphPin* Consumer : Consumers)
		{
			if (!HostGraph->GetSchema()->TryCreateConnection(BlendedPose, Consumer))
			{
				UE_LOG(LogTemp, Error, TEXT("FP linked jump fallback: downstream connection failed"));
				return false;
			}
		}

		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(FirearmTemplateAnimBlueprint);
		FKismetEditorUtilities::CompileBlueprint(FirearmTemplateAnimBlueprint);
		ConcreteFirearmAnimBlueprint->ParentClass = FirearmTemplateAnimBlueprint->GeneratedClass;
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ConcreteFirearmAnimBlueprint);
		FKismetEditorUtilities::CompileBlueprint(ConcreteFirearmAnimBlueprint);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(HostAnimBlueprint);
		FKismetEditorUtilities::CompileBlueprint(HostAnimBlueprint);
		HostAnimBlueprint->MarkPackageDirty();
		FirearmTemplateAnimBlueprint->MarkPackageDirty();
		ConcreteFirearmAnimBlueprint->MarkPackageDirty();
		return HostAnimBlueprint->Status != BS_Error
			&& FirearmTemplateAnimBlueprint->Status != BS_Error
			&& ConcreteFirearmAnimBlueprint->Status != BS_Error;
	}
	HostAnimBlueprint->ParentClass = UCharacterBaseAnimInstance::StaticClass();
	FirearmTemplateAnimBlueprint->ParentClass = UFirearmAnimInstance::StaticClass();
	if (!FBlueprintEditorUtils::ImplementsInterface(FirearmTemplateAnimBlueprint, true, InterfaceClass))
	{
		if (!FBlueprintEditorUtils::ImplementNewInterface(
			FirearmTemplateAnimBlueprint, InterfaceClass->GetClassPathName()))
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked setup: firearm template cannot implement interface"));
			return false;
		}
	}
	TemplateLayerGraph = FindGraph(FirearmTemplateAnimBlueprint, LayerName);
	if (!TemplateLayerGraph)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: new firearm template layer is missing"));
		return false;
	}

	const TSet<FName> NativeDriverNames = {
		TEXT("Is_Moving"), TEXT("Is_InAir"), TEXT("Character_Speed"),
		TEXT("Lean_Sides_Amount"), TEXT("Look_Up_Amount") };
	HostAnimBlueprint->NewVariables.RemoveAll([&NativeDriverNames](const FBPVariableDescription& Variable)
	{
		return NativeDriverNames.Contains(Variable.VarName);
	});
	for (const FBPVariableDescription& Variable : HostAnimBlueprint->NewVariables)
	{
		if (NativeDriverNames.Contains(Variable.VarName)
			|| FirearmTemplateAnimBlueprint->NewVariables.ContainsByPredicate(
				[&Variable](const FBPVariableDescription& Existing) { return Existing.VarName == Variable.VarName; }))
		{
			continue;
		}
		FirearmTemplateAnimBlueprint->NewVariables.Add(Variable);
	}
	// The imported vendor EventGraph only writes locomotion variables. The native
	// UCharacterBaseAnimInstance/UFirearmAnimInstance hierarchy is now the sole driver.
	for (UEdGraph* EventGraph : FirearmTemplateAnimBlueprint->UbergraphPages)
	{
		for (int32 Index = EventGraph->Nodes.Num() - 1; Index >= 0; --Index)
		{
			EventGraph->Nodes[Index]->DestroyNode();
		}
	}

	// Preserve the interface graph terminators, but replace its old implementation.
	UAnimGraphNode_Root* TemplateRoot = nullptr;
	for (UEdGraphNode* Node : TemplateLayerGraph->Nodes)
	{
		if (UAnimGraphNode_Root* Root = Cast<UAnimGraphNode_Root>(Node))
		{
			TemplateRoot = Root;
		}
	}
	if (!TemplateRoot)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: template layer has no root"));
		return false;
	}
	for (int32 Index = TemplateLayerGraph->Nodes.Num() - 1; Index >= 0; --Index)
	{
		UEdGraphNode* Node = TemplateLayerGraph->Nodes[Index];
		if (Node != TemplateRoot && !Node->IsA<UAnimGraphNode_LinkedInputPose>())
		{
			Node->DestroyNode();
		}
	}

	FCompilerResultsLog CloneLog;
	TArray<UEdGraphNode*> ClonedNodes;
	FEdGraphUtilities::CloneAndMergeGraphIn(
		TemplateLayerGraph, HostGraph, CloneLog, true, false, &ClonedNodes);
	TArray<UEdGraph*> TemplateGraphsAfterClone;
	FirearmTemplateAnimBlueprint->GetAllGraphs(TemplateGraphsAfterClone);
	TArray<UEdGraphNode*> NodesToRepair;
	for (UEdGraph* Graph : TemplateGraphsAfterClone)
	{
		NodesToRepair.Append(Graph->Nodes);
	}
	for (UEdGraphNode* Node : NodesToRepair)
	{
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (UClass* PinClass = Cast<UClass>(Pin->PinType.PinSubCategoryObject.Get()))
			{
				if (PinClass->ClassGeneratedBy == HostAnimBlueprint)
				{
					UE_LOG(LogTemp, Warning, TEXT("FP linked repair: node=%s class=%s pin=%s host-typed=%s"), *Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString(), *Node->GetClass()->GetName(), *Pin->GetName(), *PinClass->GetName());
					Pin->PinType.PinSubCategoryObject = FirearmTemplateAnimBlueprint->SkeletonGeneratedClass;
				}
			}
		}
		if (UK2Node_VariableGet* VariableGet = Cast<UK2Node_VariableGet>(Node))
		{
			UClass* MemberParent = VariableGet->VariableReference.GetMemberParentClass();
			if (MemberParent && (MemberParent == HostAnimBlueprint->GeneratedClass
				|| MemberParent == HostAnimBlueprint->SkeletonGeneratedClass
				|| MemberParent->ClassGeneratedBy == HostAnimBlueprint))
			{
				VariableGet->VariableReference.SetSelfMember(VariableGet->VariableReference.GetMemberName());
				VariableGet->ReconstructNode();
			}
		}
		else if (UK2Node_VariableSet* VariableSet = Cast<UK2Node_VariableSet>(Node))
		{
			UClass* MemberParent = VariableSet->VariableReference.GetMemberParentClass();
			if (MemberParent && (MemberParent == HostAnimBlueprint->GeneratedClass
				|| MemberParent == HostAnimBlueprint->SkeletonGeneratedClass
				|| MemberParent->ClassGeneratedBy == HostAnimBlueprint))
			{
				VariableSet->VariableReference.SetSelfMember(VariableSet->VariableReference.GetMemberName());
				VariableSet->ReconstructNode();
			}
		}
		else if (UK2Node_CallFunction* CallFunction = Cast<UK2Node_CallFunction>(Node))
		{
			UClass* MemberParent = CallFunction->FunctionReference.GetMemberParentClass();
			if (MemberParent && (MemberParent == HostAnimBlueprint->GeneratedClass
				|| MemberParent == HostAnimBlueprint->SkeletonGeneratedClass
				|| MemberParent->ClassGeneratedBy == HostAnimBlueprint))
			{
				CallFunction->FunctionReference.SetSelfMember(CallFunction->FunctionReference.GetMemberName());
				CallFunction->ReconstructNode();
			}
		}
		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (UClass* PinClass = Cast<UClass>(Pin->PinType.PinSubCategoryObject.Get()))
			{
				if (PinClass->ClassGeneratedBy == HostAnimBlueprint)
				{
					Pin->PinType.PinSubCategoryObject = FirearmTemplateAnimBlueprint->SkeletonGeneratedClass;
				}
			}
		}
	}

	UAnimGraphNode_Root* ClonedRoot = nullptr;
	UAnimGraphNode_StateMachine* ClonedStateMachine = nullptr;
	for (UEdGraphNode* Node : ClonedNodes)
	{
		if (UAnimGraphNode_Root* Root = Cast<UAnimGraphNode_Root>(Node))
		{
			ClonedRoot = Root;
		}
		if (UAnimGraphNode_StateMachine* StateMachine = Cast<UAnimGraphNode_StateMachine>(Node))
		{
			ClonedStateMachine = StateMachine;
		}
	}
	UEdGraphPin* ClonedResult = ClonedRoot ? ClonedRoot->FindPin(TEXT("Result")) : nullptr;
	UEdGraphPin* TemplateResult = TemplateRoot->FindPin(TEXT("Result"));
	UEdGraphPin* ClonedFinalPose = ClonedStateMachine ? ClonedStateMachine->FindPin(TEXT("Pose")) : nullptr;
	if (!ClonedResult || !ClonedFinalPose || !TemplateResult)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: cloned final pose invalid, root=%d links=%d template=%d"), ClonedResult != nullptr, ClonedResult ? ClonedResult->LinkedTo.Num() : -1, TemplateResult != nullptr);
		return false;
	}
	ClonedResult->BreakAllPinLinks();
	TemplateResult->BreakAllPinLinks();
	if (!TemplateLayerGraph->GetSchema()->TryCreateConnection(ClonedFinalPose, TemplateResult))
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: cannot connect cloned pose to template root"));
		return false;
	}
	ClonedRoot->DestroyNode();
	for (int32 Index = TemplateLayerGraph->Nodes.Num() - 1; Index >= 0; --Index)
	{
		UEdGraphNode* Node = TemplateLayerGraph->Nodes[Index];
		if (Node != TemplateRoot && Node != ClonedStateMachine && !Node->IsA<UAnimGraphNode_LinkedInputPose>())
		{
			Node->DestroyNode();
		}
	}
	if (UEdGraph* TemplateMainGraph = FindGraph(FirearmTemplateAnimBlueprint, UEdGraphSchema_K2::GN_AnimGraph))
	{
		for (int32 Index = TemplateMainGraph->Nodes.Num() - 1; Index >= 0; --Index)
		{
			if (!TemplateMainGraph->Nodes[Index]->IsA<UAnimGraphNode_Root>())
			{
				TemplateMainGraph->Nodes[Index]->DestroyNode();
			}
		}
	}

	if (!FBlueprintEditorUtils::ImplementsInterface(HostAnimBlueprint, true, InterfaceClass))
	{
		if (!FBlueprintEditorUtils::ImplementNewInterface(
			HostAnimBlueprint, InterfaceClass->GetClassPathName()))
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked setup: host cannot implement interface"));
			return false;
		}
	}

	UAnimGraphNode_StateMachine* HostStateMachine = nullptr;
	for (UEdGraphNode* Node : HostGraph->Nodes)
	{
		if (UAnimGraphNode_StateMachine* StateMachine = Cast<UAnimGraphNode_StateMachine>(Node))
		{
			HostStateMachine = StateMachine;
			break;
		}
	}
	UEdGraphPin* StateMachinePose = HostStateMachine ? HostStateMachine->FindPin(TEXT("Pose")) : nullptr;
	if (!StateMachinePose || StateMachinePose->LinkedTo.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: host state machine output missing or unused"));
		return false;
	}
	TArray<UEdGraphPin*> ExistingConsumers = StateMachinePose->LinkedTo;
	StateMachinePose->BreakAllPinLinks();

	FGraphNodeCreator<UAnimGraphNode_LinkedAnimLayer> LayerCreator(*HostGraph);
	UAnimGraphNode_LinkedAnimLayer* LayerNode = LayerCreator.CreateNode();
	LayerNode->Node.Interface = InterfaceClass;
	LayerNode->Node.Layer = LayerName;
	FGuid FunctionGuid;
	FBlueprintEditorUtils::GetFunctionGuidFromClassByFieldName(
		FBlueprintEditorUtils::GetMostUpToDateClass(InterfaceClass), LayerName, FunctionGuid);
	if (FStructProperty* FunctionReferenceProperty = FindFProperty<FStructProperty>(
		LayerNode->GetClass(), TEXT("FunctionReference")))
	{
		FMemberReference* FunctionReference = FunctionReferenceProperty->ContainerPtrToValuePtr<FMemberReference>(LayerNode);
		FunctionReference->SetExternalMember(LayerName, InterfaceClass, FunctionGuid);
	}
	LayerNode->NodePosX = HostStateMachine->NodePosX;
	LayerNode->NodePosY = HostStateMachine->NodePosY;
	LayerCreator.Finalize();
	LayerNode->ReconstructNode();
	UEdGraphPin* LayerPose = LayerNode->FindPin(TEXT("Pose"));
	if (!LayerPose)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: linked layer has no Pose pin"));
		return false;
	}
	for (UEdGraphPin* Consumer : ExistingConsumers)
	{
		if (!HostGraph->GetSchema()->TryCreateConnection(LayerPose, Consumer))
		{
			UE_LOG(LogTemp, Error, TEXT("FP linked setup: cannot connect linked layer to old consumer"));
			return false;
		}
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(FirearmTemplateAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(FirearmTemplateAnimBlueprint);
	ConcreteFirearmAnimBlueprint->ParentClass = FirearmTemplateAnimBlueprint->GeneratedClass;
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ConcreteFirearmAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(ConcreteFirearmAnimBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(HostAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(HostAnimBlueprint);
	HostAnimBlueprint->MarkPackageDirty();
	FirearmTemplateAnimBlueprint->MarkPackageDirty();
	ConcreteFirearmAnimBlueprint->MarkPackageDirty();
	const bool bSucceeded = HostAnimBlueprint->Status != BS_Error
		&& FirearmTemplateAnimBlueprint->Status != BS_Error
		&& ConcreteFirearmAnimBlueprint->Status != BS_Error;
	if (!bSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("FP linked setup: compile failed host=%d template=%d"), static_cast<int32>(HostAnimBlueprint->Status), static_cast<int32>(FirearmTemplateAnimBlueprint->Status));
	}
	return bSucceeded;
#else
	return false;
#endif
}

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

UAnimBlueprint* UTheManAnimationAssetLibrary::CreateFirstPersonHostTemplate(
	UAnimBlueprint* HostAnimBlueprint,
	const FString& PackagePath,
	const FString& AssetName,
	bool bReparentHost)
{
#if WITH_EDITOR
	if (!HostAnimBlueprint || PackagePath.IsEmpty() || AssetName.IsEmpty())
	{
		return nullptr;
	}

	const FString TemplateObjectPath = FString::Printf(
		TEXT("%s/%s.%s"), *PackagePath, *AssetName, *AssetName);
	UAnimBlueprint* TemplateAnimBlueprint = LoadObject<UAnimBlueprint>(nullptr, *TemplateObjectPath);
	if (!TemplateAnimBlueprint)
	{
		TemplateAnimBlueprint = Cast<UAnimBlueprint>(
			FAssetToolsModule::GetModule().Get().DuplicateAsset(
				AssetName, PackagePath, HostAnimBlueprint));
	}
	if (!TemplateAnimBlueprint)
	{
		return nullptr;
	}

	TemplateAnimBlueprint->TargetSkeleton = nullptr;
	TemplateAnimBlueprint->bIsTemplate = true;
	TemplateAnimBlueprint->ParentClass = UCharacterBaseAnimInstance::StaticClass();
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TemplateAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(TemplateAnimBlueprint);
	TemplateAnimBlueprint->MarkPackageDirty();
	if (TemplateAnimBlueprint->Status == BS_Error || !bReparentHost)
	{
		return TemplateAnimBlueprint;
	}

	HostAnimBlueprint->ParentClass = TemplateAnimBlueprint->GeneratedClass;
	TArray<UEdGraph*> OwnedGraphs;
	OwnedGraphs.Append(HostAnimBlueprint->UbergraphPages);
	OwnedGraphs.Append(HostAnimBlueprint->FunctionGraphs);
	OwnedGraphs.Append(HostAnimBlueprint->MacroGraphs);
	for (UEdGraph* Graph : OwnedGraphs)
	{
		if (Graph)
		{
			FBlueprintEditorUtils::RemoveGraph(HostAnimBlueprint, Graph);
		}
	}
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(HostAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(HostAnimBlueprint);
	HostAnimBlueprint->MarkPackageDirty();
	return HostAnimBlueprint->Status == BS_Error ? nullptr : TemplateAnimBlueprint;
#else
	return nullptr;
#endif
}

bool UTheManAnimationAssetLibrary::MoveTemplateAnimationAssetsToChild(
	UAnimBlueprint* TemplateAnimBlueprint,
	UAnimBlueprint* ChildAnimBlueprint,
	FName StateMachineOldName,
	FName StateMachineNewName)
{
#if WITH_EDITOR
	if (!TemplateAnimBlueprint || !ChildAnimBlueprint
		|| ChildAnimBlueprint->ParentClass != TemplateAnimBlueprint->GeneratedClass)
	{
		return false;
	}

	TArray<UEdGraph*> Graphs;
	TemplateAnimBlueprint->GetAllGraphs(Graphs);
	TArray<UAnimGraphNode_AssetPlayerBase*> AssetPlayers;
	TArray<UAnimGraphNode_AssetPlayerBase*> DisconnectedAssetPlayers;
	UAnimGraphNode_StateMachineBase* StateMachineToRename = nullptr;
	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* GraphNode : Graph->Nodes)
		{
			if (UAnimGraphNode_AssetPlayerBase* AssetPlayer = Cast<UAnimGraphNode_AssetPlayerBase>(GraphNode))
			{
				const UEdGraphPin* PosePin = AssetPlayer->FindPin(TEXT("Pose"));
				if (PosePin && PosePin->LinkedTo.IsEmpty())
				{
					DisconnectedAssetPlayers.Add(AssetPlayer);
					continue;
				}
				if (AssetPlayer->GetAnimationAsset())
				{
					if (!AssetPlayer->IsA<UAnimGraphNode_SequencePlayer>()
						&& !AssetPlayer->IsA<UAnimGraphNode_BlendSpacePlayer>())
					{
						UE_LOG(LogTemp, Error, TEXT("Template asset extraction: unsupported player %s (%s)"),
							*GraphNode->GetName(), *GraphNode->GetClass()->GetName());
						return false;
					}
					AssetPlayers.Add(AssetPlayer);
				}
			}
			if (!StateMachineOldName.IsNone())
			{
				if (UAnimGraphNode_StateMachineBase* StateMachine = Cast<UAnimGraphNode_StateMachineBase>(GraphNode))
				{
					if (StateMachine->GetStateMachineName() == StateMachineOldName.ToString())
					{
						StateMachineToRename = StateMachine;
					}
				}
			}
		}
	}

	int32 RemovedDisconnectedNodeCount = 0;
	for (UAnimGraphNode_AssetPlayerBase* AssetPlayer : DisconnectedAssetPlayers)
	{
		TSet<UEdGraphNode*> InputNodes;
		for (UEdGraphPin* Pin : AssetPlayer->Pins)
		{
			if (Pin && Pin->Direction == EGPD_Input)
			{
				for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
				{
					if (LinkedPin && LinkedPin->GetOwningNode())
					{
						InputNodes.Add(LinkedPin->GetOwningNode());
					}
				}
			}
		}
		AssetPlayer->DestroyNode();
		++RemovedDisconnectedNodeCount;
		for (UEdGraphNode* InputNode : InputNodes)
		{
			if (InputNode->IsA<UK2Node_VariableGet>()
				&& Algo::NoneOf(InputNode->Pins, [](const UEdGraphPin* Pin)
				{
					return Pin && !Pin->LinkedTo.IsEmpty();
				}))
			{
				InputNode->DestroyNode();
				++RemovedDisconnectedNodeCount;
			}
		}
	}

	for (UAnimGraphNode_AssetPlayerBase* AssetPlayer : AssetPlayers)
	{
		UAnimationAsset* ConcreteAsset = AssetPlayer->GetAnimationAsset();
		ChildAnimBlueprint->ParentAssetOverrides.RemoveAll(
			[AssetPlayer](const FAnimParentNodeAssetOverride& Override)
			{
				return Override.ParentNodeGuid == AssetPlayer->NodeGuid;
			});
		ChildAnimBlueprint->ParentAssetOverrides.Emplace(AssetPlayer->NodeGuid, ConcreteAsset);
		UE_LOG(LogTemp, Warning, TEXT("Template asset extraction: %s %s -> child override %s"),
			*AssetPlayer->GetClass()->GetName(), *AssetPlayer->NodeGuid.ToString(), *ConcreteAsset->GetPathName());
		if (UAnimGraphNode_SequencePlayer* SequencePlayer = Cast<UAnimGraphNode_SequencePlayer>(AssetPlayer))
		{
			SequencePlayer->Node.SetSequence(nullptr);
		}
		else if (UAnimGraphNode_BlendSpacePlayer* BlendSpacePlayer = Cast<UAnimGraphNode_BlendSpacePlayer>(AssetPlayer))
		{
			BlendSpacePlayer->Node.SetBlendSpace(nullptr);
		}
	}
	TemplateAnimBlueprint->ParentAssetOverrides.Empty();
	int32 AutomaticTransitionCount = 0;
	for (UEdGraph* Graph : Graphs)
	{
		bool bNeedsRelevantTimeRule = false;
		for (UEdGraphNode* GraphNode : Graph->Nodes)
		{
			if (UK2Node_TransitionRuleGetter* Getter = Cast<UK2Node_TransitionRuleGetter>(GraphNode))
			{
				bNeedsRelevantTimeRule |= Getter->GetterType == ETransitionGetter::AnimationAsset_GetTimeFromEnd
					|| Getter->GetterType == ETransitionGetter::AnimationAsset_GetTimeFromEndFraction;
			}
		}
		if (bNeedsRelevantTimeRule)
		{
			if (UAnimStateTransitionNode* Transition = Cast<UAnimStateTransitionNode>(Graph->GetOuter()))
			{
				Transition->bAutomaticRuleBasedOnSequencePlayerInState = true;
				for (int32 Index = Graph->Nodes.Num() - 1; Index >= 0; --Index)
				{
					if (!Graph->Nodes[Index]->IsA<UAnimGraphNode_TransitionResult>())
					{
						Graph->Nodes[Index]->DestroyNode();
					}
				}
				for (UEdGraphNode* GraphNode : Graph->Nodes)
				{
					if (UAnimGraphNode_TransitionResult* Result = Cast<UAnimGraphNode_TransitionResult>(GraphNode))
					{
						if (UEdGraphPin* CanEnter = Result->FindPin(TEXT("bCanEnterTransition")))
						{
							CanEnter->DefaultValue = TEXT("true");
						}
					}
				}
				++AutomaticTransitionCount;
			}
		}
		for (UEdGraphNode* GraphNode : Graph->Nodes)
		{
			for (UEdGraphPin* Pin : GraphNode->Pins)
			{
				if (Pin && Cast<UAnimationAsset>(Pin->DefaultObject))
				{
					Pin->DefaultObject = nullptr;
					Pin->DefaultValue.Reset();
				}
			}
		}
	}

	if (StateMachineToRename && !StateMachineNewName.IsNone()
		&& StateMachineOldName != StateMachineNewName)
	{
		StateMachineToRename->OnRenameNode(StateMachineNewName.ToString());
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TemplateAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(TemplateAnimBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ChildAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(ChildAnimBlueprint);
	TemplateAnimBlueprint->MarkPackageDirty();
	ChildAnimBlueprint->MarkPackageDirty();
	UE_LOG(LogTemp, Warning, TEXT("Template asset extraction complete: template=%s moved=%d removed_disconnected_nodes=%d automatic_transitions=%d state_machine_renamed=%d"),
		*TemplateAnimBlueprint->GetPathName(), AssetPlayers.Num(), RemovedDisconnectedNodeCount,
		AutomaticTransitionCount, StateMachineToRename != nullptr);
	return TemplateAnimBlueprint->Status != BS_Error
		&& ChildAnimBlueprint->Status != BS_Error;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::RestoreTemplateBlendSpaceState(
	UAnimBlueprint* TemplateAnimBlueprint,
	UAnimBlueprint* ChildAnimBlueprint,
	UBlendSpace* ConcreteBlendSpace,
	FName StateGraphName,
	FName CoordinatePropertyName)
{
#if WITH_EDITOR
	if (!TemplateAnimBlueprint || !ChildAnimBlueprint || !ConcreteBlendSpace
		|| ChildAnimBlueprint->ParentClass != TemplateAnimBlueprint->GeneratedClass
		|| StateGraphName.IsNone() || CoordinatePropertyName.IsNone())
	{
		return false;
	}

	TArray<UEdGraph*> Graphs;
	TemplateAnimBlueprint->GetAllGraphs(Graphs);
	UEdGraph** StateGraphMatch = Graphs.FindByPredicate(
		[StateGraphName](const UEdGraph* Graph)
		{
			return Graph && Graph->GetFName() == StateGraphName;
		});
	UEdGraph* StateGraph = StateGraphMatch ? *StateGraphMatch : nullptr;
	if (!StateGraph)
	{
		UE_LOG(LogTemp, Error, TEXT("Restore template BlendSpace: state graph %s not found"), *StateGraphName.ToString());
		return false;
	}

	UAnimGraphNode_StateResult* ResultNode = nullptr;
	UAnimGraphNode_SequencePlayer* SequencePlayer = nullptr;
	for (UEdGraphNode* Node : StateGraph->Nodes)
	{
		ResultNode = ResultNode ? ResultNode : Cast<UAnimGraphNode_StateResult>(Node);
		if (UAnimGraphNode_SequencePlayer* Candidate = Cast<UAnimGraphNode_SequencePlayer>(Node))
		{
			if (UEdGraphPin* PosePin = Candidate->FindPin(TEXT("Pose")); PosePin && !PosePin->LinkedTo.IsEmpty())
			{
				SequencePlayer = Candidate;
			}
		}
	}
	if (!ResultNode || !SequencePlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("Restore template BlendSpace: active Sequence Player or result missing in %s"), *StateGraphName.ToString());
		return false;
	}

	const FGuid OldSequenceGuid = SequencePlayer->NodeGuid;
	const int32 NodePosX = SequencePlayer->NodePosX;
	const int32 NodePosY = SequencePlayer->NodePosY;
	SequencePlayer->DestroyNode();

	FGraphNodeCreator<UAnimGraphNode_BlendSpacePlayer> BlendCreator(*StateGraph);
	UAnimGraphNode_BlendSpacePlayer* BlendPlayer = BlendCreator.CreateNode();
	BlendPlayer->Node.SetBlendSpace(nullptr);
	BlendPlayer->NodePosX = NodePosX;
	BlendPlayer->NodePosY = NodePosY;
	BlendCreator.Finalize();
	BlendPlayer->ReconstructNode();

	FGraphNodeCreator<UK2Node_VariableGet> CoordinateCreator(*StateGraph);
	UK2Node_VariableGet* CoordinateNode = CoordinateCreator.CreateNode();
	CoordinateNode->VariableReference.SetSelfMember(CoordinatePropertyName);
	CoordinateNode->NodePosX = NodePosX - 220;
	CoordinateNode->NodePosY = NodePosY + 80;
	CoordinateCreator.Finalize();
	CoordinateNode->ReconstructNode();

	UEdGraphPin* PosePin = BlendPlayer->FindPin(TEXT("Pose"));
	UEdGraphPin* ResultPin = ResultNode->FindPin(TEXT("Result"));
	UEdGraphPin* CoordinatePin = BlendPlayer->FindPin(TEXT("X"));
	UEdGraphPin* ValuePin = CoordinateNode->GetValuePin();
	if (!PosePin || !ResultPin || !CoordinatePin || !ValuePin
		|| !StateGraph->GetSchema()->TryCreateConnection(PosePin, ResultPin)
		|| !StateGraph->GetSchema()->TryCreateConnection(ValuePin, CoordinatePin))
	{
		UE_LOG(LogTemp, Error, TEXT("Restore template BlendSpace: failed to connect nodes in %s"), *StateGraphName.ToString());
		return false;
	}

	ChildAnimBlueprint->ParentAssetOverrides.RemoveAll(
		[OldSequenceGuid, BlendPlayer](const FAnimParentNodeAssetOverride& Override)
		{
			return Override.ParentNodeGuid == OldSequenceGuid || Override.ParentNodeGuid == BlendPlayer->NodeGuid;
		});
	ChildAnimBlueprint->ParentAssetOverrides.Emplace(BlendPlayer->NodeGuid, ConcreteBlendSpace);

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(TemplateAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(TemplateAnimBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ChildAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(ChildAnimBlueprint);
	TemplateAnimBlueprint->MarkPackageDirty();
	ChildAnimBlueprint->MarkPackageDirty();
	UE_LOG(LogTemp, Warning, TEXT("Restored template BlendSpace state: template=%s state=%s child_override=%s"),
		*TemplateAnimBlueprint->GetPathName(), *StateGraphName.ToString(), *ConcreteBlendSpace->GetPathName());
	return TemplateAnimBlueprint->Status != BS_Error && ChildAnimBlueprint->Status != BS_Error;
#else
	return false;
#endif
}

bool UTheManAnimationAssetLibrary::ConfigureFirearmUpperBodyAirbornePassThrough(
	UAnimBlueprint* FirearmTemplateAnimBlueprint,
	UAnimBlueprint* ConcreteFirearmAnimBlueprint,
	FName LayerName)
{
#if WITH_EDITOR
	if (!FirearmTemplateAnimBlueprint || !ConcreteFirearmAnimBlueprint
		|| ConcreteFirearmAnimBlueprint->ParentClass != FirearmTemplateAnimBlueprint->GeneratedClass)
	{
		return false;
	}

	UEdGraph* LayerGraph = nullptr;
	TArray<UEdGraph*> Graphs;
	FirearmTemplateAnimBlueprint->GetAllGraphs(Graphs);
	for (UEdGraph* Graph : Graphs)
	{
		if (Graph && Graph->GetFName() == LayerName)
		{
			LayerGraph = Graph;
			break;
		}
	}
	if (!LayerGraph)
	{
		return false;
	}

	UAnimGraphNode_Root* Root = nullptr;
	UAnimGraphNode_StateMachine* GroundStateMachine = nullptr;
	UAnimGraphNode_LinkedInputPose* InputPose = nullptr;
	for (UEdGraphNode* Node : LayerGraph->Nodes)
	{
		Root = Root ? Root : Cast<UAnimGraphNode_Root>(Node);
		GroundStateMachine = GroundStateMachine ? GroundStateMachine : Cast<UAnimGraphNode_StateMachine>(Node);
		InputPose = InputPose ? InputPose : Cast<UAnimGraphNode_LinkedInputPose>(Node);
		if (Cast<UAnimGraphNode_BlendListByBool>(Node))
		{
			return true;
		}
	}
	if (!Root || !GroundStateMachine || !InputPose)
	{
		return false;
	}

	UEdGraphPin* RootResult = Root->FindPin(TEXT("Result"));
	UEdGraphPin* GroundPose = GroundStateMachine->FindPin(TEXT("Pose"));
	UEdGraphPin* AirPose = InputPose->FindPin(TEXT("Pose"));
	if (!RootResult || !GroundPose || !AirPose)
	{
		return false;
	}
	RootResult->BreakAllPinLinks();

	FGraphNodeCreator<UAnimGraphNode_BlendListByBool> BlendCreator(*LayerGraph);
	UAnimGraphNode_BlendListByBool* AirBlend = BlendCreator.CreateNode();
	AirBlend->NodePosX = Root->NodePosX - 260;
	AirBlend->NodePosY = Root->NodePosY;
	BlendCreator.Finalize();

	FGraphNodeCreator<UK2Node_VariableGet> FallingCreator(*LayerGraph);
	UK2Node_VariableGet* FallingVariable = FallingCreator.CreateNode();
	FallingVariable->VariableReference.SetSelfMember(TEXT("bIsFalling"));
	FallingVariable->NodePosX = AirBlend->NodePosX - 220;
	FallingVariable->NodePosY = AirBlend->NodePosY + 220;
	FallingCreator.Finalize();

	UEdGraphPin* BlendGround = AirBlend->FindPin(TEXT("BlendPose_0"));
	UEdGraphPin* BlendAir = AirBlend->FindPin(TEXT("BlendPose_1"));
	UEdGraphPin* ActiveValue = AirBlend->FindPin(TEXT("bActiveValue"));
	UEdGraphPin* BlendOutput = AirBlend->FindPin(TEXT("Pose"));
	const UEdGraphSchema* Schema = LayerGraph->GetSchema();
	if (!BlendGround || !BlendAir || !ActiveValue || !BlendOutput
		|| !Schema->TryCreateConnection(GroundPose, BlendGround)
		|| !Schema->TryCreateConnection(AirPose, BlendAir)
		|| !Schema->TryCreateConnection(FallingVariable->GetValuePin(), ActiveValue)
		|| !Schema->TryCreateConnection(BlendOutput, RootResult))
	{
		return false;
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(FirearmTemplateAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(FirearmTemplateAnimBlueprint);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(ConcreteFirearmAnimBlueprint);
	FKismetEditorUtilities::CompileBlueprint(ConcreteFirearmAnimBlueprint);
	FirearmTemplateAnimBlueprint->MarkPackageDirty();
	ConcreteFirearmAnimBlueprint->MarkPackageDirty();
	return FirearmTemplateAnimBlueprint->Status != BS_Error
		&& ConcreteFirearmAnimBlueprint->Status != BS_Error;
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
