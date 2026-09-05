#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraScript.h"
#include "NiagaraScriptSource.h"
#include "NiagaraGraph.h"
#include "NiagaraNodeFunctionCall.h"
#include "NiagaraNodeOutput.h"
#include "ViewModels/Stack/NiagaraStackGraphUtilities.h"
#include "Misc/CommandLine.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyExplosionScaleAudit,"TheManTest.Editor.ExplosionScaleAudit",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FEnemyExplosionScaleAudit::RunTest(const FString&)
{
 auto* System=LoadObject<UNiagaraSystem>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Effects/EnemyExplosion/Systems/NS_ExplosionGun_EnemyDetonation.NS_ExplosionGun_EnemyDetonation"));
 if(!TestNotNull(TEXT("Air system"),System))return false;
 auto* ScaleModule=LoadObject<UNiagaraScript>(nullptr,TEXT("/Niagara/Modules/Update/Utility/ApplyOwnerScaleToAttributes.ApplyOwnerScaleToAttributes"));
 if(!TestNotNull(TEXT("Engine owner scale module"),ScaleModule))return false;
 TestEqual(TEXT("All Air007 emitters retained"),System->GetEmitterHandles().Num(),22);
 const bool Install=FParse::Param(FCommandLine::Get(),TEXT("InstallEnemyScale"));
 if(Install)System->Modify();
 for(const auto& Handle:System->GetEmitterHandles())
 {
  auto* Data=Handle.GetEmitterData();
  if(!Data)continue;
  if(Install)
  {
   auto* Source=CastChecked<UNiagaraScriptSource>(Data->GraphSource);
   UNiagaraNodeOutput* Output=nullptr;
   for(auto Candidate:Source->NodeGraph->Nodes)
    if(auto* Out=Cast<UNiagaraNodeOutput>(Candidate);Out && Out->GetUsage()==ENiagaraScriptUsage::ParticleUpdateScript)Output=Out;
   if(!TestNotNull(TEXT("Particle update output"),Output))return false;
   TArray<UNiagaraNodeFunctionCall*> Ordered;
   UEdGraphNode* Node=Output;
   while(Node)
   {
    if(auto* Call=Cast<UNiagaraNodeFunctionCall>(Node))Ordered.Insert(Call,0);
    UEdGraphNode* Previous=nullptr;
    for(auto* Pin:Node->Pins)
     if(Pin->Direction==EGPD_Input && Pin->PinType.PinSubCategoryObject==FNiagaraTypeDefinition::GetParameterMapDef().GetStruct() && Pin->LinkedTo.Num())
      {Previous=Pin->LinkedTo[0]->GetOwningNode();break;}
    Node=Previous;
   }
   UNiagaraNodeFunctionCall* Call=nullptr;
   int32 Index=INDEX_NONE;
   for(int32 I=0;I<Ordered.Num();++I)
   {
    if(Ordered[I]->FunctionScript==ScaleModule)Call=Ordered[I];
    if(Ordered[I]->GetFunctionName().StartsWith(TEXT("SolveForcesAndVelocity")))Index=I;
   }
   if(!Call)Call=FNiagaraStackGraphUtilities::AddScriptModuleToStack(ScaleModule,*Output,Index,TEXT("ExplosionOwnerScale"));
   if(!TestNotNull(TEXT("Owner scale module created"),Call))return false;
   auto& Pin=FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(*Call,FNiagaraParameterHandle(FName(*(Call->GetFunctionName()+TEXT(".Owner Scale")))),FNiagaraTypeDefinition::GetVec3Def(),FGuid(),FGuid());
   if(Pin.LinkedTo.IsEmpty())FNiagaraStackGraphUtilities::SetLinkedParameterValueForFunctionInput(Pin,FNiagaraVariableBase(FNiagaraTypeDefinition::GetVec3Def(),TEXT("Engine.Owner.Scale")),{});
   for(auto* Input:Call->Pins)
   {
    const FName Name=Input->PinName;
    if(Name==TEXT("Scale Initial Sprite Size")||Name==TEXT("Scale Ribbon Width")||Name==TEXT("Scale Camera Offset"))Input->DefaultValue=TEXT("true");
    if(Name==TEXT("Scale Initial Mesh Scale")||Name==TEXT("Scale Initial Velocity")||Name==TEXT("Scale Forces"))Input->DefaultValue=Data->bLocalSpace?TEXT("false"):TEXT("true");
   }
   for(const TCHAR* Amount:{TEXT("Sprite Size Scale Amount"),TEXT("Mesh Particle Scale Amount"),TEXT("Initial Velocity Scale Amount"),TEXT("Forces Scale Amount"),TEXT("Ribbon Width Scale Amount"),TEXT("Camera Offset Scale Amount")})
   {
    auto& AmountPin=FNiagaraStackGraphUtilities::GetOrCreateStackFunctionInputOverridePin(*Call,FNiagaraParameterHandle(FName(*(Call->GetFunctionName()+TEXT(".")+Amount))),FNiagaraTypeDefinition::GetFloatDef(),FGuid(),FGuid());
    AmountPin.DefaultValue=TEXT("1.0");
   }
   Source->NodeGraph->NotifyGraphChanged();
  }
  AddInfo(FString::Printf(TEXT("AIR_EMITTER %s local=%d target=%d"),*Handle.GetName().ToString(),Data->bLocalSpace,int32(Data->SimTarget)));
  int32 ScaleModules=0;
  bool OwnerScaleLinked=false;
  if(auto* Source=Cast<UNiagaraScriptSource>(Data->GraphSource))
   for(auto Node:Source->NodeGraph->Nodes)
   {
    for(auto* Pin:Node->Pins)
     if(Pin->PinName==TEXT("Engine.Owner.Scale") && !Pin->LinkedTo.IsEmpty())OwnerScaleLinked=true;
    if(auto* Call=Cast<UNiagaraNodeFunctionCall>(Node))
     if(Call->FunctionScript==ScaleModule)
     {
      ++ScaleModules;
      for(auto* Pin:Call->Pins)
      {
       const FName Name=Pin->PinName;
       if(Name==TEXT("Scale Initial Sprite Size")||Name==TEXT("Scale Ribbon Width")||Name==TEXT("Scale Camera Offset"))
        TestEqual(TEXT("Visual dimensions follow owner scale"),Pin->DefaultValue,FString(TEXT("true")));
       if(Name==TEXT("Scale Initial Mesh Scale")||Name==TEXT("Scale Initial Velocity")||Name==TEXT("Scale Forces"))
        TestEqual(TEXT("Motion and mesh respect emitter space"),Pin->DefaultValue,FString(Data->bLocalSpace?TEXT("false"):TEXT("true")));
      }
     }
   }
  TestEqual(TEXT("Exactly one scale module per emitter"),ScaleModules,1);
  TestTrue(TEXT("Owner scale input is linked"),OwnerScaleLinked);
 }
 if(Install)
 {
  System->RequestCompile(true);System->WaitForCompilationComplete(true,false);
  TestTrue(TEXT("Niagara ready"),System->IsReadyToRun());
  FSavePackageArgs Args;Args.TopLevelFlags=RF_Public|RF_Standalone;Args.SaveFlags=SAVE_NoError;
  TestTrue(TEXT("Save scaled Air007"),UPackage::SavePackage(System->GetOutermost(),System,*FPackageName::LongPackageNameToFilename(System->GetOutermost()->GetName(),FPackageName::GetAssetPackageExtension()),Args));
 }
 return true;
}
#endif
