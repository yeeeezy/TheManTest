#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Enemy/EnemyBase.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
class FEnemyExplosionRigCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int Stage=0;
 float Start=0;
 TArray<TWeakObjectPtr<AEnemyBase>> Enemies;
 TArray<FVector> Heads,Feet,Roots;
 FVector ArmBeforeL,ArmBeforeR;
 TArray<FVector> Directions={FVector(1,0,0),FVector(-1,0,0),FVector(0,1,0),FVector(0,-1,0)};
 void Capture(const TCHAR* Name)
 {
  if(auto* V=GEditor->GetPIEViewport())
  {
   TArray<FColor> Pixels;auto Size=V->GetSizeXY();
   if(V->ReadPixels(Pixels))
   {TArray64<uint8> PNG;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,PNG);FFileHelper::SaveArrayToFile(PNG,*(FPaths::ScreenShotDir()/Name));}
  }
 }
public:
 explicit FEnemyExplosionRigCommand(FAutomationTestBase* T):Test(T){}
 bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
  if(!W){Test->AddError(TEXT("Missing PIE"));return true;}
  if(Stage==0)
  {
   auto* Class=LoadClass<AEnemyBase>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   if(!Class){Test->AddError(TEXT("Missing Phantom"));return true;}
   FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   for(int I=0;I<4;++I)
   {
    auto* E=W->SpawnActor<AEnemyBase>(Class,FVector(-6000,I*180,100),FRotator::ZeroRotator,P);
    Enemies.Add(E);E->GetMesh()->GlobalAnimRateScale=0;
    Test->TestNull(TEXT("Stationary Phantom has no AI controller"),E->GetController());
    Test->TestEqual(TEXT("Stationary Phantom movement disabled"),E->GetCharacterMovement()->MovementMode.GetValue(),MOVE_None);
    auto* C=E->FindComponentByClass<UEnemyHitReactionComponent>();
    if(!C){Test->AddError(TEXT("Missing reaction component"));return true;}
    C->AttackDuration=.2f;C->RecoveryDuration=1.f;
   }
   auto* Camera=W->SpawnActor<ACameraActor>(FVector(-5500,270,240),FRotator(-10,180,0),P);
   W->GetFirstPlayerController()->SetViewTarget(Camera);
   auto* Light=W->SpawnActor<APointLight>(FVector(-5650,270,350),FRotator::ZeroRotator,P);
   auto* Lamp=Cast<UPointLightComponent>(Light->GetLightComponent());Lamp->SetIntensity(150000.f);Lamp->SetAttenuationRadius(2000);
   Start=W->GetTimeSeconds();Stage=1;return false;
  }
  if(Stage==1)
  {
   if(W->GetTimeSeconds()-Start<.3)return false;
   for(int I=0;I<Enemies.Num();++I)
   {
    auto* E=Enemies[I].Get();auto* M=E->GetMesh();
    Test->TestNotNull(TEXT("Actual skeletal mesh executes post-process reaction AnimBP"),Cast<UEnemyHitReactionAnimInstance>(M->GetPostProcessInstance()));
    Heads.Add(M->GetSocketLocation(TEXT("head")));Feet.Add(M->GetSocketLocation(TEXT("foot_l")));Roots.Add(E->GetActorLocation());
    auto* Shooter=W->SpawnActor<AActor>(E->GetActorLocation()-Directions[I]*200,FRotator::ZeroRotator);
    E->ReactToProjectileHit(Shooter);
    Test->TestTrue(TEXT("Projectile hit does not turn stationary Phantom"),E->GetActorRotation().Equals(FRotator::ZeroRotator,.01));
    Shooter->Destroy();
    auto* C=E->FindComponentByClass<UEnemyHitReactionComponent>();
    C->ReactToExplosion(M->GetSocketLocation(TEXT("spine_03"))-Directions[I]*100,Directions[I],1,TEXT("spine_03"));
   }
   Capture(TEXT("TMT_EnemyRig_Before.png"));Start=W->GetTimeSeconds();Stage=2;return false;
  }
  if(Stage==2)
  {
   if(W->GetTimeSeconds()-Start<.23)return false;
   for(int I=0;I<Enemies.Num();++I)
   {
    auto* E=Enemies[I].Get();auto* M=E->GetMesh();
    const FVector Delta=M->GetSocketLocation(TEXT("head"))-Heads[I];
    Test->TestTrue(FString::Printf(TEXT("Rendered skeleton bends away from blast direction %d: %s"),I,*Delta.ToString()),FVector::DotProduct(Delta,Directions[I])>2);
    Test->TestTrue(TEXT("Reaction does not move capsule"),E->GetActorLocation().Equals(Roots[I],.01));
    Test->TestTrue(TEXT("AI does not turn target after hit"),E->GetActorRotation().Equals(FRotator::ZeroRotator,.01));
    Test->TestTrue(TEXT("Reaction does not move support foot"),M->GetSocketLocation(TEXT("foot_l")).Equals(Feet[I],.1));
   }
   Capture(TEXT("TMT_EnemyRig_Directions.png"));Stage=3;return false;
  }
  if(Stage==3)
  {
   if(W->GetTimeSeconds()-Start<1.4)return false;
   for(int I=0;I<Enemies.Num();++I)
    Test->TestTrue(TEXT("Returns to incoming idle pose"),Enemies[I]->GetMesh()->GetSocketLocation(TEXT("head")).Equals(Heads[I],.1));
   auto* E=Enemies[0].Get();auto* M=E->GetMesh();
   ArmBeforeL=M->GetSocketTransform(TEXT("spine_03")).InverseTransformPosition(M->GetSocketLocation(TEXT("hand_l")));
   ArmBeforeR=M->GetSocketTransform(TEXT("spine_03")).InverseTransformPosition(M->GetSocketLocation(TEXT("hand_r")));
   E->FindComponentByClass<UEnemyHitReactionComponent>()->ReactToExplosion(M->GetSocketLocation(TEXT("upperarm_l"))-FVector(100,0,0),FVector::ForwardVector,1,TEXT("upperarm_l"));
   Stage=4;Start=W->GetTimeSeconds();return false;
  }
  if(W->GetTimeSeconds()-Start<.23)return false;
  auto* M=Enemies[0]->GetMesh();
  const auto Chest=M->GetSocketTransform(TEXT("spine_03"));
  const float LeftDelta=FVector::Distance(ArmBeforeL,Chest.InverseTransformPosition(M->GetSocketLocation(TEXT("hand_l"))));
  const float RightDelta=FVector::Distance(ArmBeforeR,Chest.InverseTransformPosition(M->GetSocketLocation(TEXT("hand_r"))));
  Test->TestTrue(TEXT("Left-arm hit adds localized left arm response"),LeftDelta>1.f&&LeftDelta>RightDelta+1.f);
  for(auto E:Enemies)if(E.IsValid())E->Destroy();
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyExplosionRigTest,"TheManTest.Player.Weapons.EnemyExplosionControlRig",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FEnemyExplosionRigTest::RunTest(const FString&)
{
 TestEqual(TEXT("Envelope starts neutral"),UEnemyHitReactionComponent::EvaluateEnvelope(0,.05,.5),0.f);
 TestEqual(TEXT("Envelope reaches impact peak"),UEnemyHitReactionComponent::EvaluateEnvelope(.05,.05,.5),1.f);
 TestTrue(TEXT("Envelope rebounds"),UEnemyHitReactionComponent::EvaluateEnvelope(.3,.05,.5)<0);
 TestEqual(TEXT("Envelope fully recovers"),UEnemyHitReactionComponent::EvaluateEnvelope(1,.05,.5),0.f);
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1));
 ADD_LATENT_AUTOMATION_COMMAND(FEnemyExplosionRigCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
