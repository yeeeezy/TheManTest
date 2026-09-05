#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Components/BoxComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"

namespace
{
class FProjectileAimCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Stage=0,Case=0;
 double Start=0;
 const FVector Origin=FVector(20000,20000,2000);
 TWeakObjectPtr<ACameraActor> Camera;
 TWeakObjectPtr<AActor> Target;
 TWeakObjectPtr<AExplosionGunBullet> Bullet;
public:
 explicit FProjectileAimCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  auto* PC=World?World->GetFirstPlayerController():nullptr;
  auto* Player=PC?Cast<AFPSCharacterBase>(PC->GetPawn()):nullptr;
  auto* Weapon=Player?Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()):nullptr;
  if(!Weapon){Test->AddError(TEXT("Missing equipped player weapon"));return true;}
  if(Stage==0)
  {
   Camera=World->SpawnActor<ACameraActor>(Origin,FRotator::ZeroRotator);
   PC->SetViewTarget(Camera.Get());
   // Transient test tuning: exercise the actual player ability with a sticky projectile.
   Weapon->BulletClass=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   Weapon->bIsHitscan=false;Weapon->MuzzleSocketName=NAME_None;
   Weapon->FireCameraShake=nullptr;Weapon->FireMontage=nullptr;Weapon->MuzzleEffect=nullptr;
   Start=World->GetTimeSeconds();Stage=1;return false;
  }
  if(Stage==1)
  {
   if(World->GetTimeSeconds()-Start<.2)return false;
   const float Distances[]={150.f,300.f,1000.f,20.f};
   const FVector Muzzle=Origin+FVector(60,25,-20);
   Weapon->MuzzleLocalTransform=FTransform(Muzzle).GetRelativeTransform(Weapon->GetActorTransform());
   auto* Wall=World->SpawnActor<AActor>();Target=Wall;
   auto* Box=NewObject<UBoxComponent>(Wall);Wall->SetRootComponent(Box);Wall->AddInstanceComponent(Box);
   Box->SetBoxExtent(FVector(2,200,200));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
   Box->SetCollisionObjectType(ECC_WorldStatic);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();
   Wall->SetActorLocation(Origin+FVector(Distances[Case],0,0));
   const int32 Ammo=Weapon->GetCurrentAmmo();Player->PrimaryFire();
   Test->TestEqual(TEXT("Actual PrimaryFire consumed one round"),Weapon->GetCurrentAmmo(),Ammo-1);
   for(TObjectIterator<AExplosionGunBullet> It;It;++It)
    if(It->GetWorld()==World&&It->GetOwner()==Player&&!It->IsActorBeingDestroyed())Bullet=*It;
   if(!Test->TestTrue(TEXT("Actual player ability spawned explosion bullet"),Bullet.IsValid()))return true;
   if(Case<3)
   {
    Test->TestTrue(TEXT("Clear shot still originates at muzzle"),Bullet->GetActorLocation().Equals(Muzzle,.1));
    const FVector Direction=Bullet->GetActorForwardVector();
    const FVector Crossing=Muzzle+Direction*((Distances[Case]-2-60)/Direction.X);
    Test->TestTrue(TEXT("Trajectory crosses the crosshair target within 1 mm"),
     FVector::Dist2D(FVector(Crossing.Y,Crossing.Z,0),FVector(Origin.Y,Origin.Z,0))<.1);
   }
   else
   {
    Test->TestTrue(TEXT("Muzzle beyond near wall immediately registers its obstruction"),Bullet->IsAttachedAndCountingDown());
    Test->TestTrue(TEXT("Near-wall bullet stays on camera side"),Bullet->GetActorLocation().X<Origin.X+20);
    Test->TestTrue(TEXT("Near-wall hit never fires backwards"),Bullet->GetActorForwardVector().X>0);
   }
   Start=World->GetTimeSeconds();Stage=2;return false;
  }
  if(World->GetTimeSeconds()-Start<.35)return false;
  if(Test->TestTrue(TEXT("Projectile survives to sticky impact"),Bullet.IsValid()))
  {
   Test->TestTrue(TEXT("Projectile really hit and attached"),Bullet->IsAttachedAndCountingDown());
   Test->TestTrue(TEXT("Projectile attached to the intended blocking surface"),Bullet->GetAttachParentActor()==Target.Get());
   Bullet->Destroy();
  }
  Target->Destroy();Bullet.Reset();
  if(++Case<4){Stage=1;Start=World->GetTimeSeconds();return false;}
  Camera->Destroy();return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FProjectileAimTest,"TheManTest.Player.Weapons.ProjectileCrosshairAim",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FProjectileAimTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 ADD_LATENT_AUTOMATION_COMMAND(FProjectileAimCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
