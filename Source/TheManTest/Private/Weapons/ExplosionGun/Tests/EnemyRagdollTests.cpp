#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

namespace
{
FVector RagdollMomentum(USkeletalMeshComponent* Mesh)
{
 FVector Momentum=FVector::ZeroVector;
 for(const auto* Body:Mesh->Bodies)if(Body&&Body->IsInstanceSimulatingPhysics())Momentum+=Body->GetUnrealWorldVelocity()*Body->GetBodyMass();
 return Momentum;
}
class FEnemyRagdollCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;
 int32 Case=0,Stage=0; double Start=0;
 TWeakObjectPtr<APhantom> Enemy;
 TWeakObjectPtr<ABulletBase> Flying;
 FVector BodyStart;
 const TCHAR* Paths[3]={
  TEXT("/Game/Weapons/RepairGun/Blueprint/BP_RepairGunBullet.BP_RepairGunBullet_C"),
  TEXT("/Game/Weapons/ElectricGun/Blueprint/BP_ElectricGunBullet.BP_ElectricGunBullet_C"),
  TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C")};
public:
 explicit FEnemyRagdollCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
  if(!W){Test->AddError(TEXT("Missing PIE world"));return true;}
  const double Now=W->GetTimeSeconds();
  if(Stage==0)
  {
   FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   auto* Class=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   Enemy=W->SpawnActor<APhantom>(Class,FVector(-12000,Case*1000,500),FRotator::ZeroRotator,Spawn);
   Enemy->CorpseLifetime=2.f;
   if(Case==4){Enemy->ProjectileKillKnockbackSpeed=0;Enemy->ProjectileKillUpwardSpeed=0;}
   Enemy->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
   Enemy->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),10);
   Start=Now;Stage=1;return false;
  }
  if(Stage==1)
  {
   if(Now-Start<.3)return false;
   auto* E=Enemy.Get();auto* Mesh=E->GetMesh();
   const FVector Point=Mesh->GetSocketLocation(TEXT("pelvis"));BodyStart=Point;
   if(Case!=3)
   {
    auto* Class=LoadClass<ABulletBase>(nullptr,Paths[Case%3]);
    if(!Test->TestNotNull(TEXT("Production projectile class loads"),Class))return true;
    auto* Shot=W->SpawnActor<ABulletBase>(Class,Point-FVector(50,0,0),FRotator::ZeroRotator);
    Shot->Damage=20;
    Test->AddInfo(FString::Printf(TEXT("RAGDOLL shot case=%d direction=%s strength=%.1f"),Case,*Shot->GetVelocity().GetSafeNormal(UE_SMALL_NUMBER,Shot->GetActorForwardVector()).ToString(),E->ProjectileHitImpulse));
    if(auto* Explosive=Cast<AExplosionGunBullet>(Shot)){Explosive->ExplosionDelay=10;Explosive->BulletTime.bEnabled=false;}
    FHitResult Hit(E,Mesh,Point+FVector(0,0,8),-FVector::ForwardVector);Hit.BoneName=TEXT("pelvis");Hit.bBlockingHit=true;
    APawn* Player=W->GetFirstPlayerController()->GetPawn();
    Shot->ProcessHit(Hit,Player,UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player));
    Test->AddInfo(FString::Printf(TEXT("RAGDOLL initial case=%d momentum=%s"),Case,*RagdollMomentum(Mesh).ToString()));
    Test->TestTrue(TEXT("Lethal shot synchronously starts ragdoll"),E->IsDead()&&Mesh->IsSimulatingPhysics(TEXT("pelvis")));
    Mesh->SetEnableGravity(false);
   }
   else
   {
    auto* Shot=W->SpawnActor<AExplosionGunBullet>(Point-FVector(150,0,0),FRotator::ZeroRotator);
    Shot->ExplosionDelay=0;Shot->ExplosionCueTag=FGameplayTag();Shot->BulletTime.bEnabled=false;Shot->ChaosRadius=0;
    FHitResult Hit;Hit.ImpactPoint=Point-FVector(150,0,0);Hit.ImpactNormal=FVector::UpVector;
    Shot->ProcessHit(Hit,nullptr,nullptr);
   }
   Start=Now;Stage=2;return false;
  }
  if(Stage==2)
  {
   if(Now-Start<.15)return false;
   if(!Test->TestTrue(TEXT("Corpse remains during configured lifetime"),Enemy.IsValid()))return true;
   auto* E=Enemy.Get();auto* Mesh=E->GetMesh();
   Test->TestTrue(TEXT("Dead state and physics active"),E->IsDead()&&Mesh->IsSimulatingPhysics(TEXT("pelvis")));
   Test->TestFalse(TEXT("Death hides health bar"),E->GetEnemyHealthBarComponent()->IsVisible());
   Test->TestTrue(TEXT("Death disables capsule and movement"),E->GetCapsuleComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&E->GetCharacterMovement()->MovementMode==MOVE_None);
   const FVector Velocity=Mesh->GetPhysicsLinearVelocity(TEXT("pelvis"));
   Test->AddInfo(FString::Printf(TEXT("RAGDOLL case=%d pelvisVelocity=%s displacement=%s"),Case,*Velocity.ToString(),*(Mesh->GetSocketLocation(TEXT("pelvis"))-BodyStart).ToString()));
   const FVector Momentum=RagdollMomentum(Mesh);
   Test->AddInfo(FString::Printf(TEXT("RAGDOLL settled case=%d momentum=%s"),Case,*Momentum.ToString()));
   Test->TestTrue(TEXT("Lethal projectile/blast adds forward total body momentum"),Momentum.X>100);
   if(Case<3)
   {
    Test->TestTrue(TEXT("Lethal shot launches whole body forward"),Velocity.X>180.f);
    Test->TestTrue(TEXT("Lethal shot lifts whole body"),Velocity.Z>50.f);
   }
   if(Case==4)Test->TestTrue(TEXT("Zero launch settings preserve only local impact"),FMath::Abs(Velocity.Z)<40.f);
   if(Case==3){Stage=4;return false;}
   Mesh->SetAllPhysicsLinearVelocity(FVector::ZeroVector);Mesh->SetAllPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
   const FVector Target=Mesh->GetSocketLocation(TEXT("pelvis"));
   auto* Shot=W->SpawnActor<ABulletBase>(LoadClass<ABulletBase>(nullptr,Paths[Case%3]),Target-FVector(150,0,0),FRotator::ZeroRotator);Flying=Shot;
   Shot->Damage=0;
   if(auto* Explosive=Cast<AExplosionGunBullet>(Shot)){Explosive->ExplosionDelay=10;Explosive->BulletTime.bEnabled=false;}
   APawn* Player=W->GetFirstPlayerController()->GetPawn();Shot->InitBullet(Player,UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player));
   Stage=3;return false;
  }
  if(Stage==3)
  {
   if(Now-Start<.4)return false;
   auto* Mesh=Enemy->GetMesh();const FVector Velocity=Mesh->GetPhysicsLinearVelocity(TEXT("pelvis"));
   Test->AddInfo(FString::Printf(TEXT("RAGDOLL corpse shot case=%d velocity=%s"),Case,*Velocity.ToString()));
   Test->TestTrue(TEXT("Actual flying zero-damage projectile pushes corpse"),Velocity.X>1);
   Test->TestTrue(TEXT("Corpse hit does not repeat lethal upward launch"),FMath::Abs(Velocity.Z)<40.f);
   if(Case==2)Test->TestTrue(TEXT("Explosive attaches to ragdoll bone"),Flying.IsValid()&&Flying->GetAttachParentActor()==Enemy.Get()&&!Flying->GetRootComponent()->GetAttachSocketName().IsNone());
   else Test->TestFalse(TEXT("Normal projectile consumed by corpse hit"),Flying.IsValid());
   Test->TestEqual(TEXT("Corpse health stays zero"),Enemy->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),0.f);
   Stage=4;return false;
  }
  if(Now-Start<2.4)return false;
  Test->TestFalse(TEXT("Corpse expires without hits extending its lifetime"),Enemy.IsValid());
  Test->TestFalse(TEXT("Pending explosive removed with expired corpse"),Flying.IsValid());
  ++Case;Stage=0;return Case==5;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEnemyRagdollTest,"TheManTest.Player.Weapons.EnemyDeathRagdoll",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FEnemyRagdollTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1));
 ADD_LATENT_AUTOMATION_COMMAND(FEnemyRagdollCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
