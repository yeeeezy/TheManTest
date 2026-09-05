#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Enemy/_Shared/Effects/EnemyBloodSpray.h"
#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Camera/CameraActor.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "Weapons/ExplosionGun/Effects/ExplosionCameraShake.h"
#include "Engine/GameViewportClient.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Materials/Material.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Actors/DestructibleCube/ChaosDestructibleCube.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

namespace
{
struct FStickyTestState
{
 TWeakObjectPtr<AExplosionGunBullet> Bullet,ZeroBullet,OrphanBullet;
 TWeakObjectPtr<APhantom> Enemy;
 TWeakObjectPtr<AChaosDestructibleCube> EnemyNearbyCube;
 float Start=0;
 int Stage=0;
 bool bBloodCaptured=false;
};
class FStickyPIECommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 FStickyTestState State;
public:
 explicit FStickyPIECommand(FAutomationTestBase* InTest):Test(InTest){}
 void Capture(const TCHAR* Name)
 {
  FViewport* Viewport=GEditor->GetPIEViewport();
  if(!Viewport)return;
  TArray<FColor> Pixels;const FIntPoint Size=Viewport->GetSizeXY();
  if(Viewport->ReadPixels(Pixels)&&Pixels.Num()==Size.X*Size.Y)
  {
   TArray64<uint8> Png;FImageUtils::PNGCompressImageArray(Size.X,Size.Y,Pixels,Png);
   FFileHelper::SaveArrayToFile(Png,*(FPaths::ScreenShotDir()/Name));
  }
 }
 virtual bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  if(!World){Test->AddError(TEXT("Missing PIE world"));return true;}
  if(State.Stage==0)
  {
   AFPSCharacterBase* Player=Cast<AFPSCharacterBase>(World->GetFirstPlayerController()->GetPawn());
   if(!Player||!Player->GetAbilitySystemComponent()){Test->AddError(TEXT("Missing player ASC"));return true;}
   auto* BulletClass=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   if(!BulletClass){Test->AddError(TEXT("Sticky bullet Blueprint has wrong parent"));return true;}
   Test->TestEqual(TEXT("Designer default fuse is two seconds"),BulletClass->GetDefaultObject<AExplosionGunBullet>()->ExplosionDelay,2.f);
   Test->TestEqual(TEXT("Original impact tag unchanged"),BulletClass->GetDefaultObject<AExplosionGunBullet>()->ImpactCueTag,TAG_GameplayCue_Weapon_ExplosionGun_Impact.GetTag());
   FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   auto* EnemyClass=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   auto* Enemy=World->SpawnActor<APhantom>(EnemyClass,FVector(500,400,100),FRotator::ZeroRotator,Spawn);
   if(!Enemy){Test->AddError(TEXT("Enemy failed to spawn"));return true;}
   Enemy->SetCloaked(false);State.Enemy=Enemy;
   auto* CubeClass=LoadClass<AChaosDestructibleCube>(nullptr,TEXT("/Game/Actors/DestructibleCube/Blueprint/BP_ChaosDestructibleCube.BP_ChaosDestructibleCube_C"));
   State.EnemyNearbyCube=World->SpawnActor<AChaosDestructibleCube>(CubeClass,FVector(250,400,52),FRotator::ZeroRotator,Spawn);
   const FVector CameraLocation(270,440,145);
   auto* Camera=World->SpawnActor<ACameraActor>(CameraLocation,(FVector(500,480,90)-CameraLocation).Rotation(),Spawn);
   World->GetFirstPlayerController()->SetViewTarget(Camera);
   auto* Bullet=World->SpawnActor<AExplosionGunBullet>(BulletClass,FVector(460,400,100),FRotator::ZeroRotator,Spawn);
   State.Bullet=Bullet;Bullet->ExplosionDelay=0.65f;Bullet->InitBullet(Player,Player->GetAbilitySystemComponent());
   FHitResult Hit(Enemy,Enemy->GetCapsuleComponent(),FVector(470,400,100),FVector(-1,0,0));Hit.bBlockingHit=true;
   const float Before=Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute());
   Enemy->SetCloaked(true);Bullet->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
   Test->TestFalse(TEXT("Cloaked Phantom is not sticky"),Bullet->IsAttachedAndCountingDown());
   Enemy->SetCloaked(false);Bullet->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
   Test->TestTrue(TEXT("First hit starts countdown and keeps bullet"),IsValid(Bullet)&&Bullet->IsAttachedAndCountingDown());
   Test->TestTrue(TEXT("Enemy classification retained for delayed detonation"),Bullet->DidHitEnemy());
   Test->TestEqual(TEXT("Original five-point impact damage"),Before-Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),5.f);
   Bullet->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
   Test->TestEqual(TEXT("Repeated hit cannot duplicate damage"),Before-Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),5.f);
   Test->TestEqual(TEXT("Attached bullet collision disabled"),Bullet->CollisionSphere->GetCollisionEnabled(),ECollisionEnabled::NoCollision);
   Test->TestTrue(TEXT("Attached bullet stopped"),Bullet->ProjectileMovement->Velocity.IsNearlyZero());
   Test->TestTrue(TEXT("Adjustable fuse remains positive"),Bullet->GetRemainingExplosionTime()>0.5f);
   UDecalComponent* BodyDecal=nullptr;
   for(TObjectIterator<UDecalComponent> It;It;++It)if(It->GetWorld()==World&&It->GetAttachParent()==Enemy->GetMesh()){BodyDecal=*It;break;}
   const FVector OldStainLocation=BodyDecal?BodyDecal->GetComponentLocation():FVector::ZeroVector;
   const FVector OldLocation=Bullet->GetActorLocation();Enemy->AddActorWorldOffset(FVector(0,80,0),false);
   Test->TestTrue(TEXT("Body blood follows moving target"),BodyDecal&&BodyDecal->GetComponentLocation().Equals(OldStainLocation+FVector(0,80,0),.1));
   Test->TestTrue(TEXT("Attachment follows target movement"),Bullet->GetActorLocation().Equals(OldLocation+FVector(0,80,0),.1f));
   int32 Sprays=0;for(TActorIterator<AEnemyBloodSpray> It(World);It;++It)++Sprays;
   Test->TestTrue(TEXT("Actual damage invokes enemy blood Hit Cue"),Sprays>0);
   Test->TestEqual(TEXT("Positive hit produces exactly one blood spray"),Sprays,1);
   UClass* ElectricClass=LoadClass<ABulletBase>(nullptr,TEXT("/Game/Weapons/ElectricGun/Blueprint/BP_ElectricGunBullet.BP_ElectricGunBullet_C"));
   Test->TestNotNull(TEXT("Electric bullet loads"),ElectricClass);
   if(ElectricClass)
   {
    auto CountSprays=[World](){int32 Count=0;for(TActorIterator<AEnemyBloodSpray> It(World);It;++It)++Count;return Count;};
    auto* Electric=World->SpawnActor<ABulletBase>(ElectricClass,FVector(460,480,100),FRotator::ZeroRotator,Spawn);
    Test->TestEqual(TEXT("Electric damage remains zero"),Electric->Damage,0.f);
    Electric->bDestroyOnHit=false;
    Enemy->SetCloaked(true);Electric->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
    Test->TestEqual(TEXT("Zero-damage pass-through does not produce blood"),CountSprays(),Sprays);
    Enemy->SetCloaked(false);Electric->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
    Test->TestEqual(TEXT("Zero-damage electric hit produces one blood spray"),CountSprays(),Sprays+1);
    Electric->ProcessHit(Hit,Player,Player->GetAbilitySystemComponent());
    Test->TestEqual(TEXT("Duplicate zero hit does not duplicate blood"),CountSprays(),Sprays+1);
    Test->TestEqual(TEXT("Zero hit leaves target health unchanged"),Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),95.f);
    Electric->Destroy();
   }
   TArray<UDecalComponent*> Decals;Enemy->GetComponents(Decals);
   // SpawnDecalAttached components need not be owned by the attachment actor: inspect the world objects.
   int32 Stains=0,AttachedStains=0;
   for(TObjectIterator<UDecalComponent> It;It;++It)if(It->GetWorld()==World&&It->GetDecalMaterial()&&It->GetDecalMaterial()->GetMaterial()->GetName()==TEXT("M_Enemy_BloodStain"))
   {
    ++Stains;
    if(It->GetAttachParent()==Enemy->GetMesh()&&!It->GetAttachSocketName().IsNone())++AttachedStains;
   }
   Test->TestTrue(TEXT("Blood stain attaches to actual enemy mesh bone"),AttachedStains>0);
   Test->TestTrue(TEXT("Blood stain components are spawned"),Stains>0);
   auto* Zero=World->SpawnActor<AExplosionGunBullet>(BulletClass,FVector(900,400,100),FRotator::ZeroRotator,Spawn);
   State.ZeroBullet=Zero;Zero->ExplosionDelay=0;
   FHitResult Floor;Floor.ImpactPoint=FVector(900,400,100);Floor.ImpactNormal=FVector::UpVector;
   Zero->ProcessHit(Floor,nullptr,nullptr);
   Test->TestTrue(TEXT("Zero fuse stays safe until next tick"),IsValid(Zero)&&Zero->IsAttachedAndCountingDown());
   auto* Orphan=World->SpawnActor<AExplosionGunBullet>(BulletClass,FVector(900,500,100),FRotator::ZeroRotator,Spawn);
   State.OrphanBullet=Orphan;Orphan->ExplosionDelay=.2f;Orphan->ProcessHit(Floor,nullptr,nullptr);
   State.Start=World->GetTimeSeconds();State.Stage=1;return false;
  }
  const float Elapsed=World->GetTimeSeconds()-State.Start;
  if(!State.bBloodCaptured&&Elapsed>.08f)
  {
   Capture(TEXT("TMT_EnemyBloodHit.png"));State.bBloodCaptured=true;
   bool bFleshPlaying=false;
   for(TObjectIterator<UAudioComponent> It;It;++It)if(It->GetWorld()==World&&It->Sound&&It->Sound->GetName()==TEXT("S_Enemy_FleshHit")&&It->IsPlaying())bFleshPlaying=true;
   Test->TestTrue(TEXT("Actual enemy Hit Cue is playing supplied flesh sound"),bFleshPlaying);
   auto* Modifier=Cast<UCameraModifier_CameraShake>(World->GetFirstPlayerController()->PlayerCameraManager->FindCameraModifierByClass(UCameraModifier_CameraShake::StaticClass()));
   TArray<FActiveCameraShakeInfo> Active;
   if(Modifier)Modifier->GetActiveCameraShakes(Active);
   bool bShake=false;
   for(const auto& Entry:Active)if(Entry.ShakeInstance&&Entry.ShakeInstance->IsA<UExplosionCameraShake>())bShake=true;
   Test->TestTrue(TEXT("Detonation Cue starts directional shake on local player camera"),bShake);
  }
  if(Elapsed<1.f&&State.Stage!=3)return false;
  if(State.Stage==1)
  {
  Capture(TEXT("TMT_StickyExplosion.png"));
  Test->TestFalse(TEXT("Sticky bullet disappears after fuse"),State.Bullet.IsValid());
  Test->TestFalse(TEXT("Zero fuse detonates"),State.ZeroBullet.IsValid());
  Test->TestFalse(TEXT("Missing source ASC still detonates"),State.OrphanBullet.IsValid());
  if(State.EnemyNearbyCube.IsValid())
  {
   Test->TestFalse(TEXT("Enemy attachment explosion does not trigger nearby Chaos"),State.EnemyNearbyCube->GeometryCollection->IsRootBroken());
   State.EnemyNearbyCube->Destroy();
  }
  if(State.Enemy.IsValid())Test->TestEqual(TEXT("Explosion adds no second damage"),State.Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),95.f);
  int32 Sprays=0;for(TActorIterator<AEnemyBloodSpray> It(World);It;++It)++Sprays;
  Test->TestEqual(TEXT("Blood spray cleans itself up"),Sprays,0);
  bool bExplosion=false;for(TObjectIterator<UNiagaraComponent> It;It;++It)if(It->GetWorld()==World && It->GetAsset() && It->GetAsset()->GetName()==TEXT("NS_ExplosionGun_Detonation"))bExplosion=true;
  Test->TestTrue(TEXT("Explosion Cue spawns migrated Niagara after projectile destruction"),bExplosion);
  State.Stage=2;return false;
  }
  if(State.Stage==2)
  {
   if(Elapsed<14.f)return false;
   int32 ActiveBlasts=0;
   for(TObjectIterator<UNiagaraComponent> It;It;++It)
    if(It->GetWorld()==World&&It->IsActive()&&It->GetAsset()&&It->GetAsset()->GetName()==TEXT("NS_ExplosionGun_Detonation"))++ActiveBlasts;
   Test->TestEqual(TEXT("Explosion systems finish instead of accumulating"),ActiveBlasts,0);
   int32 RemainingStains=0;
   for(TObjectIterator<UDecalComponent> It;It;++It)
    if(It->GetWorld()==World&&It->IsRegistered()&&It->GetDecalMaterial()&&It->GetDecalMaterial()->GetMaterial()->GetName()==TEXT("M_Enemy_BloodStain"))++RemainingStains;
   Test->TestEqual(TEXT("Blood stains expire after configured lifetime"),RemainingStains,0);
   auto* Enemy=State.Enemy.Get();
   if(!Enemy)return true;
   FGameplayCueParameters Blood;
   Blood.Location=Enemy->GetActorLocation()+FVector(-30,0,0);Blood.Normal=FVector(-1,0,0);
   Enemy->GetAbilitySystemComponent()->InvokeGameplayCueEvent(TAG_GameplayCue_Character_Enemy_Hit,EGameplayCueEvent::Executed,Blood);
   State.Start=World->GetTimeSeconds();State.Stage=3;return false;
  }
  if(Elapsed<.12f)return false;
  Capture(TEXT("TMT_EnemyBloodHit_Isolated.png"));
  if(State.Enemy.IsValid())State.Enemy->Destroy();
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStickyExplosionPIETest,"TheManTest.Player.Weapons.StickyExplosionAndBlood",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FStickyExplosionPIETest::RunTest(const FString& Parameters)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FStickyPIECommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
 return true;
}
#endif
