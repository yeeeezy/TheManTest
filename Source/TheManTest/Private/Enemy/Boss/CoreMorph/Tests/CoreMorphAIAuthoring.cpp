#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Enemy/Boss/CoreMorph/AI/BTTask_CoreMorphAction.h"
#include "Enemy/Boss/CoreMorph/AI/BTService_CoreMorphTarget.h"
#include "Enemy/Boss/CoreMorph/AI/BTDecorator_CoreMorphBool.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Composites/BTComposite_SimpleParallel.h"
#include "BehaviorTreeGraph.h"
#include "EdGraphSchema_BehaviorTree.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
namespace
{
bool SaveCoreMorphAI(UObject* O){O->MarkPackageDirty();FSavePackageArgs A;A.TopLevelFlags=RF_Public|RF_Standalone;return UPackage::SavePackage(O->GetOutermost(),O,*FPackageName::LongPackageNameToFilename(O->GetOutermost()->GetName(),FPackageName::GetAssetPackageExtension()),A);}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphAuthorAI,"TheManTest.Authoring.CoreMorphAI",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCoreMorphAuthorAI::RunTest(const FString&)
{
 const TCHAR* BBPath=TEXT("/Game/Enemy/Boss/CoreMorph/AI/BB_CoreMorphBoss");const TCHAR* TreePath=TEXT("/Game/Enemy/Boss/CoreMorph/AI/BT_CoreMorphBoss");
 auto* BB=FPackageName::DoesPackageExist(BBPath)?LoadObject<UBlackboardData>(nullptr,BBPath):NewObject<UBlackboardData>(CreatePackage(BBPath),TEXT("BB_CoreMorphBoss"),RF_Public|RF_Standalone);
 BB->Keys.Reset();
 for(FName Key:{FName("Manta"),FName("CanBombard"),FName("CombatReady"),FName("CanStrike"),FName("NeedsApproach")}){FBlackboardEntry E;E.EntryName=Key;E.KeyType=NewObject<UBlackboardKeyType_Bool>(BB);BB->Keys.Add(E);}
 {FBlackboardEntry E;E.EntryName=TEXT("TargetActor");E.KeyType=NewObject<UBlackboardKeyType_Object>(BB);BB->Keys.Add(E);}
 {FBlackboardEntry E;E.EntryName=TEXT("LockedStrikeLocation");E.KeyType=NewObject<UBlackboardKeyType_Vector>(BB);BB->Keys.Add(E);}
 FAssetRegistryModule::AssetCreated(BB);TestTrue(TEXT("Blackboard saved"),SaveCoreMorphAI(BB));
 auto* Tree=FPackageName::DoesPackageExist(TreePath)?LoadObject<UBehaviorTree>(nullptr,TreePath):NewObject<UBehaviorTree>(CreatePackage(TreePath),TEXT("BT_CoreMorphBoss"),RF_Public|RF_Standalone);Tree->BlackboardAsset=BB;
 if(Tree->BTGraph){Tree->BTGraph->Rename(nullptr,GetTransientPackage());Tree->BTGraph=nullptr;}
 auto* Root=NewObject<UBTComposite_Selector>(Tree);Root->NodeName=TEXT("CoreMorph boss - form and combat decisions");Tree->RootNode=Root;Root->Services.Add(NewObject<UBTService_CoreMorphTarget>(Tree));
 auto AddComposite=[&](UBTCompositeNode* Parent,UBTCompositeNode* Child){FBTCompositeChild L;L.ChildComposite=Child;Parent->Children.Add(L);};
 auto AddTask=[&](UBTCompositeNode* Parent,ECoreMorphTreeAction Action,const TCHAR* Label){auto* T=NewObject<UBTTask_CoreMorphAction>(Tree);T->Action=Action;T->NodeName=Label;FBTCompositeChild L;L.ChildTask=T;Parent->Children.Add(L);};
 auto Guard=[&](FBTCompositeChild& Child,FName Key,EBTFlowAbortMode::Type Mode){auto* D=NewObject<UBTDecorator_CoreMorphBool>(Tree);D->Configure(Key,Mode);Child.Decorators.Add(D);};
 auto* Manta=NewObject<UBTComposite_Sequence>(Tree);Manta->NodeName=TEXT("Manta flight and reassembly");AddComposite(Root,Manta);Guard(Root->Children.Last(),TEXT("Manta"),EBTFlowAbortMode::Both);
 auto* Air=NewObject<UBTComposite_SimpleParallel>(Tree);Air->NodeName=TEXT("Fly while selecting ranged phase skills");Air->FinishMode=EBTParallelMode::WaitForBackground;AddComposite(Manta,Air);
 AddTask(Air,ECoreMorphTreeAction::Flight,TEXT("Flight GA"));auto* Ranged=NewObject<UBTComposite_Selector>(Tree);Ranged->NodeName=TEXT("Manta ranged decisions");AddComposite(Air,Ranged);
 AddTask(Ranged,ECoreMorphTreeAction::FarPhaseSkill,TEXT("Current phase far-range GA"));Guard(Ranged->Children.Last(),TEXT("CanBombard"),EBTFlowAbortMode::LowerPriority);AddTask(Ranged,ECoreMorphTreeAction::Idle,TEXT("Wait for barrage cooldown"));
 AddTask(Manta,ECoreMorphTreeAction::Reassemble,TEXT("Reassemble GA"));
 auto* Active=NewObject<UBTComposite_Selector>(Tree);Active->NodeName=TEXT("Scorpion engagement");AddComposite(Root,Active);Guard(Root->Children.Last(),TEXT("CombatReady"),EBTFlowAbortMode::Both);
 auto* Attack=NewObject<UBTComposite_Sequence>(Tree);Attack->NodeName=TEXT("Face and commit a phase skill");AddComposite(Active,Attack);Guard(Active->Children.Last(),TEXT("CanStrike"),EBTFlowAbortMode::LowerPriority);
 AddTask(Attack,ECoreMorphTreeAction::Face,TEXT("Face target and plant all feet"));AddTask(Attack,ECoreMorphTreeAction::PhaseSkill,TEXT("Current phase near-range GA"));
 AddTask(Active,ECoreMorphTreeAction::Approach,TEXT("Eight-foot approach / clearance steering"));Guard(Active->Children.Last(),TEXT("NeedsApproach"),EBTFlowAbortMode::Both);
 AddTask(Active,ECoreMorphTreeAction::Idle,TEXT("Hold range / cooldown"));AddTask(Root,ECoreMorphTreeAction::Idle,TEXT("Wait for target or assembly"));
        auto* Graph=NewObject<UBehaviorTreeGraph>(Tree,TEXT("Behavior Tree"));Tree->BTGraph=Graph;Graph->Schema=UEdGraphSchema_BehaviorTree::StaticClass();
        Graph->LockUpdates();
        Graph->GetSchema()->CreateDefaultNodesForGraph(*Graph);Graph->OnCreated();Graph->Initialize();
        // Engine AutoArrange requires live Slate widgets; author deterministic graph positions offscreen.
        int32 NextX=0;TFunction<int32(UEdGraphNode*,int32)> Arrange;
        Arrange=[&](UEdGraphNode* Node,int32 Depth)
        {
            TArray<int32> ChildX;for(auto* Pin:Node->Pins)if(Pin->Direction==EGPD_Output)for(auto* Linked:Pin->LinkedTo)ChildX.Add(Arrange(Linked->GetOwningNode(),Depth+1));
            Node->NodePosY=Depth*300;Node->NodePosX=ChildX.IsEmpty()?NextX:((ChildX[0]+ChildX.Last())/2);if(ChildX.IsEmpty())NextX+=400;return Node->NodePosX;
        };
        Arrange(Graph->Nodes[0],0);Graph->UpdateClassData();Graph->UnlockUpdates();Graph->UpdateAsset();
        FAssetRegistryModule::AssetCreated(Tree);TestTrue(TEXT("Behavior tree and editor graph saved"),SaveCoreMorphAI(Tree));

 return true;
}
#endif
