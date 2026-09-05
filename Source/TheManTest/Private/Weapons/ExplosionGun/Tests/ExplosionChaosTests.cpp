#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Actors/DestructibleCube/ChaosDestructibleCube.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Camera/CameraActor.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace
{
class FExplosionChaosCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int Stage=0;
 float Start=0;
 TWeakObjectPtr<AChaosDestructibleCube> Cube,FarCube;
 TWeakObjectPtr<AExplosionGunBullet> Bullet;
 TWeakObjectPtr<AStaticMeshActor> Floor,Block;
 FVector Origin=FVector(-4000,0,0);
 void Capture(const TCHAR* Name)
 {
  if(FViewport* Viewport=GEditor->GetPIEViewport())
  {
   TArray<FColor> Pixels;const FIntPoint Size=Viewport->GetSizeXY();
   if(Viewport->ReadPixels(Pixels)&&Pixels.Num()==Size.X*Size.Y)
   {
    TArray64<uint8> Png;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,Png);
    FFileHelper::SaveArrayToFile(Png,*(FPaths::ScreenShotDir()/Name));
   }
  }
 }
public:
 explicit FExplosionChaosCommand(FAutomationTestBase* T):Test(T){}
 virtual bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  if(!World){Test->AddError(TEXT("Missing Chaos PIE world"));return true;}
  if(Stage==0)
  {
   auto* CubeClass=LoadClass<AChaosDestructibleCube>(nullptr,TEXT("/Game/Actors/DestructibleCube/Blueprint/BP_ChaosDestructibleCube.BP_ChaosDestructibleCube_C"));
   auto* BulletClass=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   if(!CubeClass||!BulletClass){Test->AddError(TEXT("Missing configured blueprints"));return true;}
   auto MakeBox=[&](FVector Location,FVector Scale)
   {
    auto* Box=World->SpawnActor<AStaticMeshActor>(Location,FRotator::ZeroRotator);
    Box->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
    Box->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
    Box->SetActorScale3D(Scale);return Box;
   };
   Floor=MakeBox(Origin-FVector(0,0,25),FVector(25,15,.5));
   Floor->Tags.Add(AExplosionGunBullet::ExplosionGroundTag);
   Block=MakeBox(Origin+FVector(0,300,100),FVector(1,1,2));
   Cube=World->SpawnActor<AChaosDestructibleCube>(CubeClass,Origin+FVector(0,0,52),FRotator::ZeroRotator);
   const FVector CameraLocation=Origin+FVector(-450,-550,280);
   auto* Camera=World->SpawnActor<ACameraActor>(CameraLocation,(Origin+FVector(0,0,65)-CameraLocation).Rotation());
   World->GetFirstPlayerController()->SetViewTarget(Camera);
   FarCube=World->SpawnActor<AChaosDestructibleCube>(CubeClass,Origin+FVector(900,0,52),FRotator::ZeroRotator);
   Bullet=World->SpawnActor<AExplosionGunBullet>(BulletClass,Origin+FVector(-250,0,55),FRotator::ZeroRotator);
   Bullet->ExplosionDelay=.8f;
   // Pause while testing projection, then fly into a real Chaos collision.
   Bullet->FindComponentByClass<UProjectileMovementComponent>()->Deactivate();
   FHitResult Hit;
   Test->TestTrue(TEXT("Find marked floor through blocking static cube"),Bullet->FindExplosionGround(Origin+FVector(0,300,250),Hit));
   Test->TestTrue(TEXT("Projection picks floor not cube top"),Hit.GetActor()==Floor.Get());
   Test->TestTrue(TEXT("Ground is floor height"),FMath::Abs(Hit.ImpactPoint.Z)<1.f);
   Floor->Tags.Reset();
   Test->TestFalse(TEXT("Untagged floor rejected"),Bullet->FindExplosionGround(Origin+FVector(0,300,250),Hit));
   Floor->Tags.Add(AExplosionGunBullet::ExplosionGroundTag);
   Bullet->GroundSearchDistance=30;
   Test->TestFalse(TEXT("Search distance is bounded"),Bullet->FindExplosionGround(Origin+FVector(0,300,250),Hit));
   Bullet->GroundSearchDistance=2000;
   Cube->Tags.Add(AExplosionGunBullet::ExplosionGroundTag);
   Test->TestTrue(TEXT("Projection through tagged GeometryCollection"),Bullet->FindExplosionGround(Origin+FVector(0,0,250),Hit));
   Test->TestTrue(TEXT("GeometryCollection never qualifies as ground"),Hit.GetActor()==Floor.Get());
   Cube->Tags.Reset();
   Block->Tags.Add(AExplosionGunBullet::ExplosionGroundTag);
   Block->SetActorScale3D(FVector(3,3,.01));
   Block->SetActorRotation(FRotator(60,0,0));
   Test->TestTrue(TEXT("Slope query finds underlying floor"),Bullet->FindExplosionGround(Origin+FVector(0,300,250),Hit));
   Test->TestTrue(TEXT("Steep tagged face rejected"),Hit.GetActor()==Floor.Get());
   Block->Destroy();
   Test->TestFalse(TEXT("Initially intact"),Cube->GeometryCollection->IsRootBroken());
   auto* CueClass=LoadClass<UGCN_ExplosionGunExplosion>(nullptr,TEXT("/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion.GC_Weapon_ExplosionGun_Explosion_C"));
   FGameplayCueParameters NoGround;NoGround.Location=Origin;
   CueClass->GetDefaultObject<UGCN_ExplosionGunExplosion>()->OnExecute_Implementation(Bullet.Get(),NoGround);
   int Count=0;
   for(TObjectIterator<UNiagaraComponent> It;It;++It)if(It->GetWorld()==World&&It->GetAsset()&&It->GetAsset()->GetName()==TEXT("NS_ExplosionGun_Detonation"))++Count;
   Test->TestEqual(TEXT("Missing ground hit suppresses Ground Niagara"),Count,0);
   Start=World->GetTimeSeconds();Stage=3;return false;
  }
  float Elapsed=World->GetTimeSeconds()-Start;
  if(Stage==3)
  {
   if(Elapsed<.3f)return false;
   Capture(TEXT("TMT_ChaosCube_Intact.png"));
   Test->AddInfo(FString::Printf(TEXT("GC type=%d enabled=%d physics=%d rest=%s bounds=%s/%s; bullet response=%d"),int(Cube->GeometryCollection->GetCollisionObjectType()),int(Cube->GeometryCollection->GetCollisionEnabled()),Cube->GeometryCollection->IsPhysicsStateCreated(),*GetNameSafe(Cube->FractureAsset),*Cube->GeometryCollection->Bounds.Origin.ToString(),*Cube->GeometryCollection->Bounds.BoxExtent.ToString(),int(Bullet->FindComponentByClass<USphereComponent>()->GetCollisionResponseToChannel(Cube->GeometryCollection->GetCollisionObjectType()))));
   auto* Movement=Bullet->FindComponentByClass<UProjectileMovementComponent>();
   Movement->Velocity=FVector(2000,0,0);
   Movement->Activate();
   Start=World->GetTimeSeconds();Stage=1;return false;
  }
  if(Stage==1)
  {
   if(Elapsed<.2f)return false;
   Test->AddInfo(FString::Printf(TEXT("Bullet after flight %s, cube bounds %s"),*Bullet->GetActorLocation().ToString(),*Cube->GeometryCollection->Bounds.Origin.ToString()));
   Test->TestTrue(TEXT("Real projectile sweep attaches to Chaos cube"),Bullet.IsValid()&&Bullet->IsAttachedAndCountingDown());
   if(Bullet.IsValid())Test->TestFalse(TEXT("Cube hit classified non-enemy"),Bullet->DidHitEnemy());
   Test->TestFalse(TEXT("No fracture before fuse"),Cube->GeometryCollection->IsRootBroken());
   Stage=2;return false;
  }
  if(Elapsed<2.f)return false;
  Capture(TEXT("TMT_ChaosCube_Detonated.png"));
  Test->TestFalse(TEXT("Fuse consumes projectile"),Bullet.IsValid());
  Test->TestTrue(TEXT("Actual Chaos root fractured"),Cube->GeometryCollection->IsRootBroken());
  Test->TestFalse(TEXT("Outside blast radius remains intact"),FarCube->GeometryCollection->IsRootBroken());
  Test->TestTrue(TEXT("Released pieces dispersed by impulse"),Cube->GeometryCollection->Bounds.BoxExtent.GetMax()>65.f);
  bool bGroundEffect=false;
  for(TObjectIterator<UNiagaraComponent> It;It;++It)
  {
   if(It->GetWorld()!=World||!It->GetAsset()||It->GetAsset()->GetName()!=TEXT("NS_ExplosionGun_Detonation"))continue;
   if(FMath::Abs(It->GetComponentLocation().X-Origin.X)<300.f)
   { bGroundEffect=true;Test->TestTrue(TEXT("Ground VFX stays on floor, not attached cube surface"),FMath::Abs(It->GetComponentLocation().Z-1.f)<2.f); }
  }
  Test->TestTrue(TEXT("Detonation played ground Niagara"),bGroundEffect);
  for(AActor* Actor:{static_cast<AActor*>(Cube.Get()),static_cast<AActor*>(FarCube.Get()),static_cast<AActor*>(Floor.Get()),static_cast<AActor*>(Bullet.Get())})if(IsValid(Actor))Actor->Destroy();
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplosionChaosTest,"TheManTest.Player.Weapons.ExplosionChaosGround",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplosionChaosTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FExplosionChaosCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
