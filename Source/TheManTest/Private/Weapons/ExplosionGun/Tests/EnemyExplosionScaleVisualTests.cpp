#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraDataSet.h"
#include "NiagaraDataSetAccessor.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
class FEnemyScaleVisualCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Stage=0,Case=0;
 double Start=0;
 float Scales[3]={1.f,.25f,2.f};
 float FireSize[3]={};
 float ChunkRadius[3]={};
 int32 BrightPixels[3]={};
 TWeakObjectPtr<UNiagaraComponent> Effect;
 TWeakObjectPtr<ACameraActor> Camera;
 FVector Origin=FVector(15000,15000,1000);
 TArray<TPair<FVersionedNiagaraEmitterData*,bool>> Determinism;
public:
 explicit FEnemyScaleVisualCommand(FAutomationTestBase* In):Test(In){}
 virtual ~FEnemyScaleVisualCommand(){for(auto& Entry:Determinism)Entry.Key->bDeterminism=Entry.Value;}
 virtual bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;if(!W){Test->AddError(TEXT("No PIE world"));return true;}
  const double Now=W->GetTimeSeconds();
  auto* Class=LoadClass<UGCN_ExplosionGunExplosion>(nullptr,TEXT("/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion.GC_Weapon_ExplosionGun_Explosion_C"));
  if(Stage==0)
  {
   if(Case==0)
   {
    auto* System=Class->GetDefaultObject<UGCN_ExplosionGunExplosion>()->EnemyExplosionEffect.Get();
    for(auto& Handle:System->GetEmitterHandles())if(auto* Data=Handle.GetEmitterData()){Determinism.Emplace(Data,Data->bDeterminism);Data->bDeterminism=true;}
    Camera=W->SpawnActor<ACameraActor>(Origin-FVector(2400,0,0),FRotator::ZeroRotator);
    W->GetFirstPlayerController()->SetViewTarget(Camera.Get());
   }
   auto* Cue=NewObject<UGCN_ExplosionGunExplosion>(GetTransientPackage(),Class);
   Cue->EnemyEffectScale=Scales[Case];Cue->EnemyExplosionSound=nullptr;Cue->CameraShakeClass=nullptr;
   FGameplayCueParameters P;P.Location=Origin;P.Normal=FVector::UpVector;P.AggregatedTargetTags.AddTag(TAG_Data_Explosion_EnemyImpact);
   Test->TestTrue(TEXT("Enemy Cue works without any ground hit"),Cue->OnExecute_Implementation(Camera.Get(),P));
   for(TObjectIterator<UNiagaraComponent> It;It;++It)
    if(It->GetWorld()==W && It->GetAsset()==Cue->EnemyExplosionEffect && It->IsActive())Effect=*It;
   if(!Test->TestTrue(TEXT("Real Cue spawned Air007"),Effect.IsValid()))return true;
   Test->TestTrue(TEXT("Cue passes requested scale"),Effect->GetComponentScale().Equals(FVector(Scales[Case]),.001f));
   Start=Now;Stage=1;return false;
  }
  if(Stage==1)
  {
   if(Now-Start<.3)return false;
   // Sample every scale at the same simulation age, independent of render frame time.
   Effect->ResetSystem();
   Effect->AdvanceSimulation(18,1.f/60.f);
   Effect->SetPaused(true);
   int32 SpriteCount=0;
   if(auto Controller=Effect->GetSystemInstanceController())
    if(auto* Instance=Controller->GetSystemInstance_Unsafe())
     for(auto& Emitter:Instance->GetEmitters())
     {
      if(Emitter->GetEmitterHandle().GetEmitterData()->SimTarget!=ENiagaraSimTarget::CPUSim || Emitter->GetNumParticles()==0)continue;
      auto& Data=Emitter->GetParticleData();auto Reader=FNiagaraDataSetAccessor<FVector2f>(Data,TEXT("Particles.SpriteSize")).GetReader(Data);
      if(!Reader.IsValid())Reader=FNiagaraDataSetAccessor<FVector2f>(Data,TEXT("SpriteSize")).GetReader(Data);
      Test->AddInfo(FString::Printf(TEXT("AIR_PARTICLES %s count=%d spriteReader=%d"),*Emitter->GetEmitterHandle().GetName().ToString(),Emitter->GetNumParticles(),Reader.IsValid()));
      if(Reader.IsValid()&&Emitter->GetEmitterHandle().GetName()==TEXT("NE_Fireball01"))
      {
       for(int32 I=0;I<Emitter->GetNumParticles();++I)FireSize[Case]+=Reader.Get(I).Size();
       SpriteCount+=Emitter->GetNumParticles();
      }
      if(Emitter->GetEmitterHandle().GetName()==TEXT("NE_Chunks_MOD"))
      {
       auto Positions=FNiagaraDataSetAccessor<FNiagaraPosition>(Data,TEXT("Position")).GetReader(Data);
       if(Positions.IsValid())
       {
        for(int32 I=0;I<Emitter->GetNumParticles();++I)ChunkRadius[Case]+=FVector::Distance(FVector(Positions.Get(I)),Origin);
        ChunkRadius[Case]/=Emitter->GetNumParticles();
       }
      }
     }
   if(SpriteCount)FireSize[Case]/=SpriteCount;
   Start=Now;Stage=2;return false;
  }
  if(Now-Start<.1)return false;
  if(FViewport* View=GEditor->GetPIEViewport())
  {
   TArray<FColor> Pixels;const FIntPoint Size=View->GetSizeXY();
   if(View->ReadPixels(Pixels)&&Pixels.Num()==Size.X*Size.Y)
   {
    for(const auto& P:Pixels)if(P.R>100 && P.R>P.B*1.2f)++BrightPixels[Case];
    TArray64<uint8> Png;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,Png);
    FFileHelper::SaveArrayToFile(Png,*(FPaths::ScreenShotDir()/FString::Printf(TEXT("TMT_EnemyExplosionScale_%d.png"),Case)));
   }
  }
  Test->AddInfo(FString::Printf(TEXT("AIR_SCALE scale=%.2f meanSprite=%.3f chunkRadius=%.3f brightPixels=%d"),Scales[Case],FireSize[Case],ChunkRadius[Case],BrightPixels[Case]));
  Effect->DestroyComponent();Effect.Reset();
  if(++Case<3){Stage=0;return false;}
  Test->TestTrue(TEXT("Live sprite particles are present"),FireSize[0]>1.f);
  Test->TestTrue(TEXT("Quarter scale really reduces particle dimensions"),FireSize[1]<FireSize[0]*.4f);
  Test->TestTrue(TEXT("Double scale really increases particle dimensions"),FireSize[2]>FireSize[0]*1.5f);
  Test->TestTrue(TEXT("Quarter scale reduces world-space fragment travel"),ChunkRadius[1]>0.f && ChunkRadius[1]<ChunkRadius[0]*.6f);
  Test->TestTrue(TEXT("Double scale increases world-space fragment travel"),ChunkRadius[2]>ChunkRadius[0]*1.4f);
  Test->TestTrue(TEXT("Quarter scale visibly reduces rendered explosion"),BrightPixels[1]>0 && BrightPixels[1]<BrightPixels[0]*.5f);
  Test->TestTrue(TEXT("Double scale visibly increases rendered explosion"),BrightPixels[2]>BrightPixels[0]*1.3f);
  Camera->Destroy();return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyExplosionScaleVisualTest,"TheManTest.Player.Weapons.EnemyExplosionScaleVisual",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FEnemyExplosionScaleVisualTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FEnemyScaleVisualCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
