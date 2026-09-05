#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Core/_Shared/Feedback/BulletTimeSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"

namespace
{
class FExplosionPhysicsCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;int32 Stage=0;double Start=0;
 TArray<TWeakObjectPtr<UStaticMeshComponent>> Bodies;
 TWeakObjectPtr<AActor> Wall;
 const FVector Origin=FVector(30000,30000,2000);
public:
 explicit FExplosionPhysicsCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;if(!W){Test->AddError(TEXT("Missing PIE"));return true;}
  if(Stage==0)
  {
   const FVector Offsets[]={FVector(100,0,0),FVector(0,250,0),FVector(-200,0,0),FVector(0,-600,0)};
   for(int32 I=0;I<4;++I)
   {
    auto* A=W->SpawnActor<AActor>();auto* C=NewObject<UStaticMeshComponent>(A);A->SetRootComponent(C);A->AddInstanceComponent(C);
    C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    C->SetWorldScale3D(FVector(.2));C->SetCollisionProfileName(TEXT("PhysicsActor"));C->RegisterComponent();A->SetActorLocation(Origin+Offsets[I]);
    C->SetEnableGravity(false);C->SetSimulatePhysics(true);Bodies.Add(C);
   }
   auto* A=W->SpawnActor<AActor>();Wall=A;auto* Box=NewObject<UBoxComponent>(A);A->SetRootComponent(Box);A->AddInstanceComponent(Box);
   Box->SetBoxExtent(FVector(5,80,80));Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();A->SetActorLocation(Origin+FVector(-100,0,0));
   Start=W->GetTimeSeconds();Stage=1;return false;
  }
  if(W->GetTimeSeconds()-Start<.15)return false;
  if(Stage==1)
  {
   auto* B=W->SpawnActor<AExplosionGunBullet>(Origin,FRotator::ZeroRotator);B->ExplosionDelay=0;B->ExplosionDamage=0;B->ChaosRadius=0;B->PhysicsImpulseStrength=800;B->ExplosionCueTag=FGameplayTag();
   FHitResult H;H.ImpactPoint=Origin;H.ImpactNormal=FVector::UpVector;B->ProcessHit(H,nullptr,nullptr);
   Start=W->GetTimeSeconds();Stage=2;return false;
  }
  const float Near=Bodies[0]->GetPhysicsLinearVelocity().Size(),Far=Bodies[1]->GetPhysicsLinearVelocity().Size();
  Test->TestTrue(TEXT("Near physics object receives outward velocity"),Bodies[0]->GetPhysicsLinearVelocity().X>300);
  Test->TestTrue(TEXT("Far object receives weaker blast"),Far>100 && Far<Near);
  Test->TestTrue(TEXT("Wall shields physics object"),Bodies[2]->GetPhysicsLinearVelocity().Size()<1);
  Test->TestTrue(TEXT("Outside radius remains still"),Bodies[3]->GetPhysicsLinearVelocity().Size()<1);
  Test->TestFalse(TEXT("Ordinary physics impulse does not trigger bullet time"),W->GetSubsystem<UBulletTimeSubsystem>()->IsBulletTimeActive());
  Test->AddInfo(FString::Printf(TEXT("PHYSICS_BLAST near=%.2f far=%.2f"),Near,Far));
  for(auto C:Bodies)C->GetOwner()->Destroy();Wall->Destroy();return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplosionPhysicsTest,"TheManTest.Player.Weapons.ExplosionSimulatedPhysics",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplosionPhysicsTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1));
 ADD_LATENT_AUTOMATION_COMMAND(FExplosionPhysicsCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
