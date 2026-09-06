#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/Humanoid/Animation/EnemyHitReactionComponent.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
namespace {
class FAttachedReactionCommand : public IAutomationLatentCommand {
 FAutomationTestBase* Test;int Stage=0;double Start=0;int Case=0;
 TWeakObjectPtr<APhantom> Target,Neighbor;
 TWeakObjectPtr<AExplosionGunBullet> Bullet;
public:
 explicit FAttachedReactionCommand(FAutomationTestBase* T):Test(T){}
 bool Update() override {
  UWorld* W=GEditor?GEditor->PlayWorld:nullptr;if(!W){Test->AddError(TEXT("No PIE world"));return true;}
  const double Now=W->GetTimeSeconds();
  if(Stage==0){
   auto* C=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
   FActorSpawnParameters S;S.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   Target=W->SpawnActor<APhantom>(C,FVector(20000,20000,500),FRotator::ZeroRotator,S);
   Neighbor=W->SpawnActor<APhantom>(C,FVector(20000,20180,500),FRotator::ZeroRotator,S);
   for(auto E:{Target,Neighbor}){E->SetCloaked(false);E->GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;}
   if(Case==1)Target->GetAbilitySystemComponent()->SetNumericAttributeBase(UEnemyAttributeSetBase::GetHealthAttribute(),10.f);
   Start=Now;Stage=1;return false;
  }
  if(Stage==1){
   if(Now-Start<.3)return false;
   auto* M=Target->GetMesh();const FVector P=M->GetSocketLocation(TEXT("calf_l"));
   auto* B=W->SpawnActor<AExplosionGunBullet>(P-FVector(100,0,0),FRotator::ZeroRotator);Bullet=B;
   B->ExplosionDelay=.2f;B->ChaosRadius=0;B->BulletTime.bEnabled=false;B->ExplosionCueTag=FGameplayTag();B->Damage=5;
   FHitResult H(Target.Get(),M,P,-FVector::ForwardVector);H.BoneName=TEXT("calf_l");H.bBlockingHit=true;
   B->ProcessHit(H,nullptr,Target->GetAbilitySystemComponent());
   Test->TestTrue(TEXT("Projectile really attached to target"),B->GetAttachParentActor()==Target.Get());
   const FName Bone=B->GetRootComponent()->GetAttachSocketName();
   Test->TestTrue(TEXT("Resolved actual physics surface is left leg"),Bone==TEXT("thigh_l") || M->BoneIsChildOf(Bone,TEXT("thigh_l")));
   UAnimSequence* Before=nullptr;float BeforeTime=0,BeforeAlpha=0;Target->ExplosionHitReaction->SampleAnimation(Before,BeforeTime,BeforeAlpha);
   Test->TestEqual(TEXT("First impact deals five damage before fuse"),Target->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),Case==0?95.f:5.f);
   Test->TestTrue(TEXT("No animation before fuse despite first-impact damage"),!Before&&BeforeAlpha==0.f);
   Target->SetActorRotation(FRotator(0,90,0));
   Start=Now;Stage=2;return false;
  }
  if(Now-Start<.4)return false;
  UAnimSequence* A=nullptr;float Time=0,Alpha=0;
  Target->ExplosionHitReaction->SampleAnimation(A,Time,Alpha);
  if(Case==0){
   Test->TestTrue(TEXT("Attached damaged survivor gets full-strength directional animation"),A&&Alpha>.95f);
   Test->TestEqual(TEXT("Attached target takes blast damage after initial hit"),Target->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),75.f);
  }else Test->TestTrue(TEXT("Lethal explosion uses ragdoll instead of living animation"),Target->IsDead()&&!A&&Target->GetMesh()->IsSimulatingPhysics(TEXT("pelvis")));
  Neighbor->ExplosionHitReaction->SampleAnimation(A,Time,Alpha);
  Test->TestTrue(TEXT("Collateral damaged neighbor reacts from its left toward the blast"),A&&A->GetName()==TEXT("AS_Humanoid_BlastRifle_Left")&&Alpha>.95f);
  Test->TestEqual(TEXT("Collateral neighbor still takes blast damage"),Neighbor->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),80.f);
  Target->Destroy();Neighbor->Destroy();if(Bullet.IsValid())Bullet->Destroy();
  ++Case;Stage=0;return Case==2;
 }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttachedReactionTest,"TheManTest.Player.Weapons.AttachedLimbReaction",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FAttachedReactionTest::RunTest(const FString&){
 AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1));
 ADD_LATENT_AUTOMATION_COMMAND(FAttachedReactionCommand(this));ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());return true;
}
#endif
