#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

namespace
{
class FStickySurfaceCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Stage=0;
 double Start=0;
 TArray<TWeakObjectPtr<APhantom>> Enemies;
 TArray<TWeakObjectPtr<AExplosionGunBullet>> Bullets;
 TArray<FVector> LocalPoints,WorldPoints;
 TArray<FHitResult> Expected;
 const TArray<FVector> Directions={FVector(1,0,0),FVector(-1,0,0),FVector(1,1,0).GetSafeNormal(),FVector(-1,1,0).GetSafeNormal(),FVector(-1,-1,0).GetSafeNormal(),FVector(1,-1,0).GetSafeNormal()};
public:
 explicit FStickySurfaceCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  if(!World){Test->AddError(TEXT("Missing PIE world"));return true;}
  FActorSpawnParameters Params;Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  if(Stage==0)
  {
   auto* Class=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   if(!Test->TestNotNull(TEXT("Phantom class"),Class))return true;
   for(int32 I=0;I<Directions.Num();++I)
   {
    auto* Enemy=World->SpawnActor<APhantom>(Class,FVector(-8000,I*500,150),FRotator::ZeroRotator,Params);
    Enemies.Add(Enemy);Enemy->GetMesh()->GlobalAnimRateScale=0;
    Enemy->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
   }
   Start=World->GetTimeSeconds();Stage=1;return false;
  }
  if(World->GetTimeSeconds()-Start<.3)return false;
  if(Stage==1)
  {
   auto* Class=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   if(!Test->TestNotNull(TEXT("Sticky projectile class"),Class))return true;
   for(int32 I=0;I<Directions.Num();++I)
   {
    auto* Mesh=Enemies[I]->GetMesh();const FVector Center=Mesh->GetSocketLocation(TEXT("spine_03"));
    const FVector From=Center-Directions[I]*180;
    FHitResult Surface;FCollisionQueryParams Query(SCENE_QUERY_STAT(StickySurfaceTest),false);
    Test->TestTrue(TEXT("Entry-side body surface exists"),Mesh->LineTraceComponent(Surface,From,Center+Directions[I]*180,Query));Expected.Add(Surface);
    auto* Bullet=World->SpawnActor<AExplosionGunBullet>(Class,From,Directions[I].Rotation(),Params);
    Bullet->ExplosionDelay=10;Bullet->Damage=0;Bullets.Add(Bullet);
   }
   Start=World->GetTimeSeconds();Stage=2;return false;
  }
  if(Stage==2)
  {
   for(int32 I=0;I<Bullets.Num();++I)
   {
    auto* Bullet=Bullets[I].Get();auto* Mesh=Enemies[I]->GetMesh();
    if(!Test->TestNotNull(TEXT("Flying bullet survives impact"),Bullet))return true;
    Test->TestTrue(TEXT("Actual physics collision starts sticky fuse"),Bullet->IsAttachedAndCountingDown());
    Test->TestTrue(TEXT("Bullet attaches to skeletal mesh, never capsule"),Bullet->GetRootComponent()->GetAttachParent()==Mesh);
    const FName Bone=Bullet->GetRootComponent()->GetAttachSocketName();
    Test->TestFalse(TEXT("Attachment has a valid bone"),Bone.IsNone());
    const FVector Desired=Expected[I].ImpactPoint+Expected[I].ImpactNormal*Bullet->AttachmentOffset;
    Test->TestTrue(FString::Printf(TEXT("Direction %d remains at entry surface (error %.3f cm)"),I,FVector::Distance(Bullet->GetActorLocation(),Desired)),Bullet->GetActorLocation().Equals(Desired,.5));
    Test->TestTrue(TEXT("Resolved projectile mesh remains visible"),Bullet->FindComponentByClass<UStaticMeshComponent>()->IsVisible());
    LocalPoints.Add(Mesh->GetSocketTransform(Bone).InverseTransformPosition(Bullet->GetActorLocation()));WorldPoints.Add(Bullet->GetActorLocation());
    auto* Reaction=Enemies[I]->FindComponentByClass<UEnemyHitReactionComponent>();
    Reaction->ReactToExplosion(Expected[I].ImpactPoint-Directions[I]*100,Directions[I],1,TEXT("spine_03"));
   }
   Start=World->GetTimeSeconds();Stage=3;return false;
  }
  for(int32 I=0;I<Bullets.Num();++I)
  {
   auto* Bullet=Bullets[I].Get();auto* Mesh=Enemies[I]->GetMesh();
   const FVector ExpectedPoint=Mesh->GetSocketTransform(Bullet->GetRootComponent()->GetAttachSocketName()).TransformPosition(LocalPoints[I]);
   Test->TestTrue(TEXT("Projectile follows animated attachment bone"),Bullet->GetActorLocation().Equals(ExpectedPoint,.1));
   Test->TestTrue(FString::Printf(TEXT("Reaction moves direction %d attached projectile on %s (%.3f cm)"),I,*Bullet->GetRootComponent()->GetAttachSocketName().ToString(),FVector::Distance(Bullet->GetActorLocation(),WorldPoints[I])),FVector::Distance(Bullet->GetActorLocation(),WorldPoints[I])>.1);
   Bullet->Destroy();Enemies[I]->Destroy();
  }
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStickySurfaceTest,"TheManTest.Player.Weapons.StickyBodySurfaces",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStickySurfaceTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FStickySurfaceCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
