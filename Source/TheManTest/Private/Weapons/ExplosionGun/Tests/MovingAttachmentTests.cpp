#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionAnimInstance.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemGlobals.h"

namespace
{
class FMovingAttachmentCommand : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;int32 Stage=0;double Start=0;bool Reacted=false;
 TWeakObjectPtr<AHumanoidEnemy> Enemy;TWeakObjectPtr<AExplosionGunBullet> Bullet;
 TWeakObjectPtr<UDecalComponent> Stain;TWeakObjectPtr<AActor> Floor;
 FVector StartPosition,BulletLocal,StainLocal;
 bool BlastDeath=false;
public:
 explicit FMovingAttachmentCommand(FAutomationTestBase* In):Test(In){}
 bool Update() override
 {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;if(!W){Test->AddError(TEXT("Missing PIE"));return true;}
  if(Stage==0)
  {
   auto* A=W->SpawnActor<AActor>();Floor=A;auto* Box=NewObject<UBoxComponent>(A);A->SetRootComponent(Box);A->AddInstanceComponent(Box);
   Box->SetBoxExtent(FVector(2000,2000,10));Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();A->SetActorLocation(FVector(-15000,0,-10));
   auto* Fixture=LoadClass<AHumanoidEnemy>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"))->GetDefaultObject<AHumanoidEnemy>();
   auto* E=W->SpawnActorDeferred<AHumanoidEnemy>(AHumanoidEnemy::StaticClass(),FTransform(FVector(-15000,0,100)),nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Enemy=E;
   E->AutoPossessAI=EAutoPossessAI::Disabled;
   E->GetMesh()->SetSkeletalMeshAsset(Fixture->GetMesh()->GetSkeletalMeshAsset());E->GetMesh()->SetRelativeTransform(Fixture->GetMesh()->GetRelativeTransform());
   E->GetMesh()->SetAnimInstanceClass(Fixture->GetMesh()->GetAnimClass());
   E->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
   E->FinishSpawning(FTransform(FVector(-15000,0,100)));
   E->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetMaxHealthAttribute(),100);
   E->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),100);
   E->GetCharacterMovement()->bRunPhysicsWithNoController=true;E->GetCharacterMovement()->MaxWalkSpeed=150;E->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
   Start=W->GetTimeSeconds();Stage=1;return false;
  }
  if(Stage==1)
  {
   if(W->GetTimeSeconds()-Start<.4)return false;
   Test->TestNotNull(TEXT("Non-Phantom humanoid automatically uses shared reaction ABP"),Cast<UEnemyHitReactionAnimInstance>(Enemy->GetMesh()->GetPostProcessInstance()));
   const FVector Center=Enemy->GetMesh()->GetSocketLocation(TEXT("spine_03"));
   auto* Class=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
   auto* B=W->SpawnActor<AExplosionGunBullet>(Class,Center-FVector(180,0,0),FRotator::ZeroRotator);Bullet=B;B->ExplosionDelay=10;
   APawn* Player=W->GetFirstPlayerController()->GetPawn();B->InitBullet(Player,UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player));
   Start=W->GetTimeSeconds();Stage=2;return false;
  }
  if(Stage==2)
  {
   if(W->GetTimeSeconds()-Start<.2)return false;
   Test->TestTrue(TEXT("Actual flying projectile attached before walking"),Bullet.IsValid()&&Bullet->IsAttachedAndCountingDown());
   for(TObjectIterator<UDecalComponent> It;It;++It)if(It->GetWorld()==W&&It->GetAttachParent()==Enemy->GetMesh())Stain=*It;
   if(!Test->TestTrue(TEXT("Damage created a body stain"),Stain.IsValid()))return true;
   Test->TestTrue(TEXT("Body stain is owned by the enemy"),Stain->GetOwner()==Enemy.Get());
   BulletLocal=Bullet->GetRootComponent()->GetRelativeLocation();StainLocal=Stain->GetRelativeLocation();StartPosition=Enemy->GetActorLocation();
   Start=W->GetTimeSeconds();Stage=3;return false;
  }
  if(Stage==3)
  {
   Enemy->AddMovementInput(FVector(1,0,0),1,true);
   if(!Reacted&&W->GetTimeSeconds()-Start>.5)
   {
    Enemy->SetActorRotation(FRotator(0,45,0));
    Enemy->FindComponentByClass<UEnemyHitReactionComponent>()->ReactToExplosion(Bullet->GetActorLocation()-FVector(80,0,0),FVector::ForwardVector,1,TEXT("spine_03"));Reacted=true;
   }
   Test->TestTrue(TEXT("Moving/turning/reacting bullet keeps its bone-local attachment"),Bullet->GetRootComponent()->GetRelativeLocation().Equals(BulletLocal,.01));
   Test->TestTrue(TEXT("Moving/turning/reacting stain keeps its bone-local attachment"),Stain->GetRelativeLocation().Equals(StainLocal,.01));
   if(W->GetTimeSeconds()-Start<1.5)return false;
   Test->AddInfo(FString::Printf(TEXT("MOVING_ATTACHMENT start=%s end=%s velocity=%s mode=%d speed=%.1f"),*StartPosition.ToString(),*Enemy->GetActorLocation().ToString(),*Enemy->GetVelocity().ToString(),int32(Enemy->GetCharacterMovement()->MovementMode.GetValue()),Enemy->GetCharacterMovement()->MaxWalkSpeed));
   Test->TestTrue(TEXT("CharacterMovement actually walked over 100cm"),Enemy->GetActorLocation().X-StartPosition.X>100);
   Test->TestTrue(TEXT("Locomotion animation is running"),Enemy->GetMesh()->GetAnimInstance()!=nullptr);
   if(!BlastDeath)Enemy->OnDeath();
   else
   {
    Enemy->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),10);
    const FVector Point=Enemy->GetMesh()->GetSocketLocation(TEXT("spine_03"));
    auto* Detonator=W->SpawnActor<AExplosionGunBullet>(Point,FRotator::ZeroRotator);
    Detonator->ExplosionDelay=0;Detonator->ExplosionCueTag=FGameplayTag();
    FHitResult Hit(Enemy.Get(),Enemy->GetCapsuleComponent(),Point,FVector::UpVector);Hit.bBlockingHit=true;
    APawn* Player=W->GetFirstPlayerController()->GetPawn();
    Detonator->ProcessHit(Hit,Player,UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Player));
   }
   Start=W->GetTimeSeconds();Stage=4;return false;
  }
  if(W->GetTimeSeconds()-Start<.1)return false;
  Test->TestFalse(TEXT("Enemy death destroys pending attached bullet"),Bullet.IsValid());
  Test->TestFalse(TEXT("Enemy actually died"),Enemy.IsValid());
  Test->TestTrue(TEXT("Enemy death destroys owned body decal"),!Stain.IsValid()||!Stain->IsRegistered());
  Floor->Destroy();
  if(!BlastDeath){BlastDeath=true;Reacted=false;Stage=0;return false;}
  return true;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMovingAttachmentTest,"TheManTest.Player.Weapons.MovingEnemyAttachmentCleanup",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FMovingAttachmentTest::RunTest(const FString&)
{
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
 ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1));
 ADD_LATENT_AUTOMATION_COMMAND(FMovingAttachmentCommand(this));
 ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
