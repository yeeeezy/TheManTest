#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionAnimInstance.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"

namespace
{
class FDamageReactionCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int Stage=0,Case=0;
 double Start=0;
 const FVector Origin=FVector(30000,30000,500);
 const FRotator Facing=FRotator(0,37,0);
 const FVector Sources[4]={FVector(1,0,0),FVector(-1,0,0),FVector(0,-1,0),FVector(0,1,0)};
 const TCHAR* Names[4]={TEXT("Front"),TEXT("Back"),TEXT("Left"),TEXT("Right")};
 TArray<TWeakObjectPtr<APhantom>> Enemies;
 TArray<TWeakObjectPtr<AExplosionGunBullet>> Bullets;

 AExplosionGunBullet* SpawnBullet(UWorld* World,const FVector& Point,const FVector& Direction)
 {
  FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  auto* Bullet=World->SpawnActor<AExplosionGunBullet>(Point,Direction.Rotation(),Spawn);
  Bullet->ChaosRadius=0;Bullet->PhysicsImpulseRadius=0;Bullet->BulletTime.bEnabled=false;
  Bullet->ExplosionCueTag=FGameplayTag();Bullets.Add(Bullet);return Bullet;
 }
 void Blast(UWorld* World)
 {
  auto* Bullet=SpawnBullet(World,Origin,FVector::ForwardVector);
  Bullet->ExplosionDelay=.05f;
  if(Case==1)Bullet->ExplosionDamageEffectClass=UGameplayEffect::StaticClass();
  if(Case==2)Bullet->ExplosionDamage=0;
  FHitResult Hit;Hit.ImpactPoint=Origin;Hit.ImpactNormal=FVector::UpVector;
  Bullet->ProcessHit(Hit,nullptr,nullptr);
 }
 void Shoot(UWorld* World,int Index)
 {
  auto* Enemy=Enemies[Index].Get();auto* Mesh=Enemy->GetMesh();
  const FVector Direction=-Facing.RotateVector(Sources[Index]);
  const FVector Point=Mesh->GetSocketLocation(TEXT("pelvis"));
  auto* Bullet=SpawnBullet(World,Point-Direction*100.f,Direction);
  Bullet->ExplosionDelay=10.f;Bullet->ExplosionDamage=0;Bullet->Damage=Case==4?0.f:5.f;
  if(Case==5)Bullet->HitEffectClass=UGameplayEffect::StaticClass();
  FHitResult Hit(Enemy,Mesh,Point,-Direction);Hit.BoneName=TEXT("pelvis");Hit.bBlockingHit=true;
  Bullet->ProcessHit(Hit,nullptr,Enemy->GetAbilitySystemComponent());
  // The same collision callback must not apply damage or restart a reaction twice.
  Bullet->ProcessHit(Hit,nullptr,Enemy->GetAbilitySystemComponent());
 }
public:
 explicit FDamageReactionCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
  if(!World){Test->AddError(TEXT("Missing PIE world"));return true;}
  const double Now=World->GetTimeSeconds();
  if(Stage==0)
  {
   auto* Class=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   if(!Class){Test->AddError(TEXT("Missing Phantom fixture"));return true;}
   FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   for(int Index=0;Index<4;++Index)
   {
    const FVector Point=Case<3?Origin-Facing.RotateVector(Sources[Index])*220.f:Origin+FVector(Index*1000.f,0,0);
    auto* Enemy=World->SpawnActor<APhantom>(Class,Point,Facing,Spawn);
    if(!Enemy){Test->AddError(TEXT("Failed to spawn reaction fixture"));return true;}
    Enemy->SetCloaked(false);Enemy->GetCharacterMovement()->DisableMovement();
    Enemy->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Enemy->ExplosionHitReaction->bApplyAnimationRootMotion=false;
    if(Case==6)Enemy->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),2.f);
    Enemies.Add(Enemy);
   }
   Start=Now;Stage=1;return false;
  }
  if(Stage==1)
  {
   if(Now-Start<.2)return false;
   if(Case<3)Blast(World);else for(int Index=0;Index<4;++Index)Shoot(World,Index);
   Start=Now;Stage=2;return false;
  }
  if(Now-Start<.3)return false;
  for(int Index=0;Index<4;++Index)
  {
   auto* Enemy=Enemies[Index].Get();
   UAnimSequence* Animation=nullptr;float Time=0,Alpha=0;
   Enemy->ExplosionHitReaction->SampleAnimation(Animation,Time,Alpha);
   const FString Label=FString::Printf(TEXT("Case %d %s"),Case,Names[Index]);
   const float Health=Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute());
   if(Case==0||Case==3)
   {
    const FString Expected=FString(TEXT("AS_Humanoid_BlastRifle_"))+Names[Index];
    Test->TestTrue(Label+TEXT(" selects direction in rotated target space"),Animation&&Animation->GetName()==Expected&&Alpha>.95f);
    Test->TestEqual(Label+TEXT(" applies damage once"),Health,Case==0?80.f:95.f);
    auto* Post=Cast<UEnemyHitReactionAnimInstance>(Enemy->GetMesh()->GetPostProcessInstance());
    Test->TestTrue(Label+TEXT(" reaches the live animation instance"),Post&&Post->ReactionAnimation==Animation&&Post->ReactionAlpha>.95f);
    if(Case==3)
    {
     Shoot(World,Index);
     float AfterTime=0;Enemy->ExplosionHitReaction->SampleAnimation(Animation,AfterTime,Alpha);
     Test->TestEqual(Label+TEXT(" new damaging hit still deducts health"),Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),90.f);
     Test->TestTrue(Label+TEXT(" new hit preserves current animation time"),AfterTime>=Time&&AfterTime>.2f);
    }
   }
   else if(Case==6)
    Test->TestTrue(Label+TEXT(" lethal direct hit uses ragdoll"),Enemy->IsDead()&&Health==0.f&&!Animation&&Enemy->GetMesh()->IsSimulatingPhysics(TEXT("pelvis")));
   else
   {
    Test->TestEqual(Label+TEXT(" zero damage or effect with no health modifier preserves health"),Health,100.f);
    Test->TestTrue(Label+TEXT(" no actual health loss means no reaction"),!Animation&&Alpha==0.f);
   }
  }
  for(auto Bullet:Bullets)if(Bullet.IsValid())Bullet->Destroy();
  for(auto Enemy:Enemies)if(Enemy.IsValid())Enemy->Destroy();
  Bullets.Reset();Enemies.Reset();Stage=0;return ++Case==7;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDamageReactionTest,"TheManTest.Player.Weapons.ExplosionDamageReactions",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FDamageReactionTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
 ADD_LATENT_AUTOMATION_COMMAND(FDamageReactionCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
