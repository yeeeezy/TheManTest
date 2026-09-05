#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Actors/DestructibleCube/ChaosDestructibleCube.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/WorldSettings.h"
#include "Camera/CameraActor.h"
#include "Components/BoxComponent.h"

namespace
{
class FExplosionOutcomeCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Case=0,Stage=0;
 double Start=0;
 bool bSawSlow=false,bPreviousSlow=false;
 int32 Starts=0;
 TWeakObjectPtr<AChaosDestructibleCube> Cube;
 TWeakObjectPtr<APhantom> Enemy;
 TWeakObjectPtr<AExplosionGunBullet> Bullet;
 TWeakObjectPtr<AActor> Wall;
 FVector Origin=FVector(18000,18000,150);
 const TCHAR* Names[12]={TEXT("Empty explosion"),TEXT("Nonlethal enemy hit"),TEXT("Enemy killed"),TEXT("Chaos resists strain"),TEXT("Chaos actually breaks"),TEXT("Loose fragments hit again"),TEXT("Kill and break together"),TEXT("Enemy behind wall"),TEXT("Bullet time disabled"),TEXT("Collateral Chaos breaks"),TEXT("Direct Chaos disabled"),TEXT("Direct Chaos without strain")};
public:
 explicit FExplosionOutcomeCommand(FAutomationTestBase* In):Test(In){}
 virtual bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
  if(!W){Test->AddError(TEXT("Missing outcome PIE world"));return true;}
  auto* Feedback=W->GetSubsystem<UBulletTimeSubsystem>();
  const double Now=W->GetTimeSeconds();
  if(Stage==0)
  {
   bSawSlow=false;bPreviousSlow=false;Starts=0;
   FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   if(Case==0)
   {
    auto* Camera=W->SpawnActor<ACameraActor>(Origin+FVector(-150,0,80),FRotator::ZeroRotator,Spawn);
    W->GetFirstPlayerController()->SetViewTarget(Camera);
   }
   if(Case==1||Case==2||Case==6||Case==7||Case==8)
   {
    auto* Class=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
    auto* E=W->SpawnActor<APhantom>(Class,Origin+FVector(150,0,0),FRotator::ZeroRotator,Spawn);Enemy=E;
    E->SetCloaked(false);E->GetCharacterMovement()->DisableMovement();
    E->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),Case==1?100.f:20.f);
   }
   if(Case==3||Case==4||Case==6||Case>=9)
   {
    auto* Class=LoadClass<AChaosDestructibleCube>(nullptr,TEXT("/Game/Actors/DestructibleCube/Blueprint/BP_ChaosDestructibleCube.BP_ChaosDestructibleCube_C"));
    const FTransform Transform(Origin+FVector(0,140,0));
    auto* C=W->SpawnActorDeferred<AChaosDestructibleCube>(Class,Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    C->Toughness=Case==3?1.e12f:100000.f;C->FinishSpawning(Transform);Cube=C;
    C->GeometryCollection->SetEnableGravity(false);
   }
   if(Case==7)
   {
    auto* A=W->SpawnActor<AActor>(Origin+FVector(70,0,0),FRotator::ZeroRotator,Spawn);Wall=A;
    auto* Box=NewObject<UBoxComponent>(A);A->SetRootComponent(Box);A->AddInstanceComponent(Box);
    Box->SetBoxExtent(FVector(10,100,200));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionObjectType(ECC_WorldStatic);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();Box->SetWorldLocation(Origin+FVector(70,0,0));
   }
   Start=Now;Stage=1;return false;
  }
  if(Stage==1)
  {
   if(Now-Start<.3)return false;
   FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   auto* Class=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   auto* Shot=W->SpawnActor<AExplosionGunBullet>(Class,Origin,FRotator::ZeroRotator,Spawn);Bullet=Shot;
   Shot->ExplosionDelay=.1f;Shot->ExplosionCueTag=FGameplayTag();
   Shot->BulletTime=FBulletTimeSettings();Shot->BulletTime.HoldDuration=.4f;Shot->BulletTime.InnerRadius=1000.f;
   Shot->BulletTime.bEnabled=Case!=8&&Case!=10;Shot->ChaosImpulse=0;Shot->ChaosAngularSpeed=0;
   FHitResult Hit;Hit.ImpactPoint=Origin;Hit.ImpactNormal=FVector::UpVector;
   if(Case==3||Case==4||Case==5||Case==10||Case==11)
    Hit=FHitResult(Cube.Get(),Cube->GeometryCollection,Cube->GetActorLocation()+FVector(0,-50,0),-FVector::RightVector);
   if(Case==11)Shot->ChaosRadius=0;
   Shot->ProcessHit(Hit,nullptr,nullptr);
   Test->TestFalse(FString(Names[Case])+TEXT(" no slow motion before fuse"),Feedback->IsBulletTimeActive());
   Start=Now;Stage=2;return false;
  }
  const bool Active=Feedback->IsBulletTimeActive();
  bSawSlow|=Active;if(Active&&!bPreviousSlow)++Starts;bPreviousSlow=Active;
  if(Now-Start<1.0)return false;
  const bool Expected=Case==2||Case==3||Case==4||Case==5||Case==6||Case==11;
  Test->TestEqual(FString(Names[Case])+TEXT(" bullet-time outcome"),bSawSlow,Expected);
  Test->TestEqual(FString(Names[Case])+TEXT(" starts only once"),Starts,Expected?1:0);
  Test->TestFalse(FString(Names[Case])+TEXT(" bullet destroyed"),Bullet.IsValid());
  Test->TestEqual(TEXT("World speed restored"),W->GetWorldSettings()->TimeDilation,1.f);
  if(Case==1)Test->TestEqual(TEXT("Surviving enemy loses 20 health"),Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),80.f);
  if(Case==2||Case==6||Case==8)Test->TestTrue(TEXT("Lethal blast really killed enemy"),Enemy.IsValid() && Enemy->IsDead());
  if(Case==7)Test->TestEqual(TEXT("Wall really blocked lethal blast"),Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),20.f);
  if(Cube.IsValid())
  {
   Test->TestEqual(TEXT("Observed actual Chaos state"),Cube->GeometryCollection->IsRootBroken(),Case!=3&&Case!=11);
   Test->TestFalse(TEXT("Temporary Chaos notifications restored"),Cube->GeometryCollection->bNotifyBreaks);
   if(Case!=4){Cube->Destroy();Cube.Reset();}
  }
  if(Enemy.IsValid())Enemy->Destroy();Enemy.Reset();
  if(Wall.IsValid())Wall->Destroy();Wall.Reset();
  ++Case;Stage=0;return Case==12;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplosionOutcomeTest,"TheManTest.Player.Weapons.ExplosionOutcomeBulletTime",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplosionOutcomeTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FExplosionOutcomeCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
