#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"

namespace
{
class FStickyUpAimCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Stage=0,Case=0;
 double Start=0;
 TWeakObjectPtr<APhantom> Enemy;
 TWeakObjectPtr<ACameraActor> Camera;
 TWeakObjectPtr<AExplosionGunBullet> Bullet;
 FVector View,Forward,Up,Right,Expected;
public:
 explicit FStickyUpAimCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;auto* PC=W?W->GetFirstPlayerController():nullptr;
  auto* Player=PC?Cast<AFPSCharacterBase>(PC->GetPawn()):nullptr;
  auto* Weapon=Player?Cast<AFirearm>(Player->GetEquipmentManager()->GetCurrentEquipment()):nullptr;
  if(!Weapon){Test->AddError(TEXT("Missing player weapon"));return true;}
  if(Stage==0)
  {
   auto* Class=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   Enemy=W->SpawnActor<APhantom>(Class,FVector(25000,25000,2000),FRotator::ZeroRotator);
   Enemy->GetMesh()->GlobalAnimRateScale=0;
   Enemy->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
   Camera=W->SpawnActor<ACameraActor>();PC->SetViewTarget(Camera.Get());
   Weapon->BulletClass=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   Weapon->bIsHitscan=false;Weapon->MuzzleSocketName=NAME_None;
   Weapon->FireCameraShake=nullptr;Weapon->FireMontage=nullptr;Weapon->MuzzleEffect=nullptr;
   Start=W->GetTimeSeconds();Stage=1;return false;
  }
  if(W->GetTimeSeconds()-Start<.25)return false;
  if(Stage==1)
  {
   const FRotator Rotation((Case%3)*30.f,0,0);
   Forward=Rotation.Vector();Up=Rotation.RotateVector(FVector::UpVector);Right=Rotation.RotateVector(FVector::RightVector);
   const FVector Center=Enemy->GetMesh()->GetSocketLocation(TEXT("spine_03"));
   View=Center-Forward*(Case<3?100.f:200.f);
   Camera->SetActorLocationAndRotation(View,Rotation);
   FHitResult Hit;FCollisionQueryParams Q(SCENE_QUERY_STAT(StickyUpAim),false);
   if(!Test->TestTrue(TEXT("Crosshair has a skeletal surface target"),Enemy->GetMesh()->LineTraceComponent(Hit,View,Center+Forward*200,Q)))return true;
   Expected=Hit.ImpactPoint;
   Start=W->GetTimeSeconds();Stage=2;return false;
  }
  if(Stage==2)
  {
   const FVector Muzzle=View+Forward*60+Right*25-Up*20;
   Weapon->MuzzleLocalTransform=FTransform(Muzzle).GetRelativeTransform(Weapon->GetActorTransform());
   Player->PrimaryFire();
   for(TObjectIterator<AExplosionGunBullet> It;It;++It)
    if(It->GetWorld()==W&&It->GetOwner()==Player&&!It->IsActorBeingDestroyed())Bullet=*It;
   if(!Test->TestTrue(TEXT("Actual player shot spawned"),Bullet.IsValid()))return true;
   Bullet->ExplosionDelay=10;
   Test->AddInfo(FString::Printf(TEXT("UP_LAUNCH case=%d immediate=%d muzzle=%s bullet=%s direction=%s"),Case,Bullet->IsAttachedAndCountingDown(),*Muzzle.ToString(),*Bullet->GetActorLocation().ToString(),*Bullet->GetActorForwardVector().ToString()));
   Start=W->GetTimeSeconds();Stage=3;return false;
  }
  Test->TestTrue(TEXT("Upward shot attaches to enemy mesh"),Bullet->GetRootComponent()->GetAttachParent()==Enemy->GetMesh());
  const FVector Offset=Bullet->GetActorLocation()-Expected;
  const float Vertical=FVector::DotProduct(Offset,Up),Horizontal=FVector::DotProduct(Offset,Right);
  Test->AddInfo(FString::Printf(TEXT("UP_RESULT case=%d vertical=%.3f horizontal=%.3f expected=%s actual=%s"),Case,Vertical,Horizontal,*Expected.ToString(),*Bullet->GetActorLocation().ToString()));
  Test->TestTrue(TEXT("Attached projectile remains within 6 cm of crosshair vertically"),FMath::Abs(Vertical)<6.f);
  Bullet->Destroy();Bullet.Reset();
  if(++Case<6){Stage=1;Start=W->GetTimeSeconds();return false;}
  Enemy->Destroy();Camera->Destroy();return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStickyUpAimTest,"TheManTest.Player.Weapons.StickyUpwardAim",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStickyUpAimTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
 ADD_LATENT_AUTOMATION_COMMAND(FStickyUpAimCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
