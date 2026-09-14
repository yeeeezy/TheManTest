#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Components/PointLightComponent.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphTailEffects.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphScorpionMovement.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphTailStrike.h"
#include "Enemy/Boss/CoreMorph/AI/CoreMorphAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AbilitySystemComponent.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
namespace
{
ACoreMorphBoss* ScorpionBoss(){if(GEditor->PlayWorld)for(TActorIterator<ACoreMorphBoss> I(GEditor->PlayWorld);I;++I)return *I;return nullptr;}
TWeakObjectPtr<ACoreMorphBoss> ExitOwner;
double TailMaxBend=0,TailMaxEndBend=0,TailLengthError=0;
void ObserveTail(UCoreMorphScorpionCombat* C)
{
 const auto& N=C->GetTailNodes();const auto& L=C->GetTailLengths();
 for(int32 J=0;J<L.Num();++J)
 {
  TailLengthError=FMath::Max(TailLengthError,FMath::Abs(FVector::Dist(N[J],N[J+1])-L[J]));
  if(J==0)continue;
  const double Bend=FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct((N[J]-N[J-1]).GetSafeNormal(),(N[J+1]-N[J]).GetSafeNormal()),-1.0,1.0)));
  TailMaxBend=FMath::Max(TailMaxBend,Bend);if(J==15)TailMaxEndBend=FMath::Max(TailMaxEndBend,Bend);
 }
}
void AdvanceScorpion(UCoreMorphScorpionCombat* C,float Seconds)
{
 for(float Left=Seconds;Left>KINDA_SMALL_NUMBER;Left-=1.f/120)
 {const float Dt=FMath::Min(Left,1.f/120);C->TickComponent(Dt,LEVELTICK_All,nullptr);ObserveTail(C);CastChecked<ACoreMorphBoss>(C->GetOwner())->TailEffects->TickComponent(Dt,LEVELTICK_All,nullptr);}
}

void SetHealth(AEnemyBase* B,float Value)
{
 auto* GE=NewObject<UGameplayEffect>();GE->DurationPolicy=EGameplayEffectDurationType::Instant;
 for(auto A:{UEnemyAttributeSetBase::GetMaxHealthAttribute(),UEnemyAttributeSetBase::GetHealthAttribute()}){auto& M=GE->Modifiers.AddDefaulted_GetRef();M.Attribute=A;M.ModifierOp=EGameplayModOp::Override;M.ModifierMagnitude=FScalableFloat(Value);}
 auto* ASC=B->GetAbilitySystemComponent();ASC->ApplyGameplayEffectToSelf(GE,1,ASC->MakeEffectContext());
}
void ClearCooldown(ACoreMorphBoss* B){B->GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_TailCooldown));}
bool PrepareScorpion(ACoreMorphBoss* B)
{
 B->ResetFlightPreview();if(!B->StartReassembly())return false;
 for(int32 I=0;I<318;++I)B->Reassembly->TickComponent(1.f/60,LEVELTICK_All,nullptr);
 auto* C=B->ScorpionCombat.Get();C->bEnabled=true;C->TickComponent(2.6f,LEVELTICK_All,nullptr);
 if(auto* AI=Cast<ACoreMorphAIController>(B->GetController()))if(AI->GetBrainComponent())AI->GetBrainComponent()->PauseLogic(TEXT("Deterministic integration checks"));
 return C->IsDrivingPose();
}
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCoreMorphScorpion,FAutomationTestBase*,Test);
bool FCheckCoreMorphScorpion::Update()
{
 TailMaxBend=TailMaxEndBend=TailLengthError=0;
 auto* B=ScorpionBoss();if(!Test->TestNotNull(TEXT("Scorpion boss in actual PIE"),B))return true;
 if(!Test->TestTrue(TEXT("Morph hands existing 301 parts to component motor"),PrepareScorpion(B)))return true;
 auto* C=B->ScorpionCombat.Get();auto* M=B->ScorpionMovement.Get();auto* ASC=B->GetAbilitySystemComponent();
 Test->TestTrue(TEXT("AI possesses the single boss Pawn"),Cast<ACoreMorphAIController>(B->GetController())!=nullptr);
 Test->TestEqual(TEXT("One owner ASC"),TInlineComponentArray<UAbilitySystemComponent*>(B).Num(),1);
 Test->TestEqual(TEXT("Flight, morph and phase strike each granted once"),ASC->GetActivatableAbilities().Num(),3);
 Test->TestEqual(TEXT("Eight articulated feet"),M->GetFeet().Num(),8);
 Test->TestEqual(TEXT("Final armor is reused"),B->Reassembly->GetPieces().Num(),455);
 const FVector Start=B->GetActorLocation();M->SetExternalDrive(M->GetGroundLocation()+FVector(8000,0,0),true,true);
 float MaxPlantSlip=0,MaxLengthError=0;int32 MinSupports=8;
 for(int32 I=0;I<480;++I)
 {
  const auto Before=M->GetFeet();C->TickComponent(1.f/60,LEVELTICK_All,nullptr);B->Reassembly->TickComponent(1.f/60,LEVELTICK_All,nullptr);
  MinSupports=FMath::Min(MinSupports,M->PlantedFeet);
  for(int32 L=0;L<8;++L)
  {
   const auto& F=M->GetFeet()[L];if(!Before[L].bSwing && !F.bSwing)MaxPlantSlip=FMath::Max(MaxPlantSlip,float(FVector::Dist(Before[L].Position,F.Position)));
   for(int32 J=1;J<7;++J)MaxLengthError=FMath::Max(MaxLengthError,FMath::Abs(float(FVector::Dist(M->GetSolvedNodes()[L*8+J],M->GetSolvedNodes()[L*8+J+1])-FVector::Dist(M->Layout->LegNodes[L*8+J],M->Layout->LegNodes[L*8+J+1]))));
  }
 }
 Test->TestTrue(TEXT("Motor advances the real boss while landing cue expires"),FVector::Dist2D(Start,B->GetActorLocation())>1000);
 Test->TestTrue(TEXT("At least four supporting feet"),MinSupports>=4);Test->TestTrue(TEXT("Planted contacts do not slide"),MaxPlantSlip<.01f);Test->TestTrue(TEXT("Anatomical leg lengths are preserved"),MaxLengthError<.05f);
 Test->TestFalse(TEXT("Old landing cue has expired during movement"),B->Reassembly->HasCue());
 M->SetExternalDrive(M->GetGroundLocation(),false,false);AdvanceScorpion(C,1.5f);
 FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Victim=B->GetWorld()->SpawnActor<AEnemyBase>(AEnemyBase::StaticClass(),M->GetGroundLocation()+B->GetActorForwardVector()*3300+FVector(0,0,90),FRotator::ZeroRotator,P);
 Victim->GetCharacterMovement()->DisableMovement();Victim->GetCharacterMovement()->SetComponentTickEnabled(false);SetHealth(Victim,1000);C->Target=Victim;
 Test->TestTrue(TEXT("Phase skill selection starts the full tail GA"),B->UseRandomSkill(Victim,EEnemySkillRange::Near));
 Test->TestTrue(TEXT("Attack state comes from a GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Attacking));
 Test->TestTrue(TEXT("Charge Cue creates owner FX"),B->TailEffects->IsCharging() && ASC->HasMatchingGameplayTag(TAG_GameplayCue_CoreMorph_TailCharge));
 Test->TestEqual(TEXT("One ground telegraph"),TInlineComponentArray<UDecalComponent*>(B).Num(),1);
 const FVector WarningCenter=B->TailEffects->GetWarningCenter();const float WarningRadius=B->TailEffects->GetWarningRadius();
 auto SpawnVictim=[&](FVector Location){auto* E=B->GetWorld()->SpawnActor<AEnemyBase>(AEnemyBase::StaticClass(),Location,FRotator::ZeroRotator,P);E->GetCharacterMovement()->DisableMovement();E->GetCharacterMovement()->SetComponentTickEnabled(false);SetHealth(E,1000);return E;};
 auto* Covered=SpawnVictim(WarningCenter+FVector(0,-650,90));
 auto* Cover=B->GetWorld()->SpawnActor<AStaticMeshActor>(WarningCenter+FVector(0,-350,150),FRotator::ZeroRotator,P);
 Cover->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Cover->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Cover->SetActorScale3D(FVector(5,.6,5));Cover->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
 auto* Inside=SpawnVictim(WarningCenter+FVector(0,550,90));auto* Outside=SpawnVictim(WarningCenter+FVector(0,1400,90));
 for(int32 I=0;I<3;++I){auto* Piece=NewObject<USphereComponent>(Inside);Piece->SetupAttachment(Inside->GetRootComponent());Piece->SetSphereRadius(100);Piece->SetRelativeLocation(FVector(I*80,0,0));Piece->SetCollisionProfileName(TEXT("Pawn"));Piece->RegisterComponent();}
 AdvanceScorpion(C,.4f);Test->TestEqual(TEXT("Charge does not apply health damage"),Inside->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),1000.f);
 Victim->AddActorWorldOffset(FVector(0,180,0));C->BlastRadius=500;
 Test->TestTrue(TEXT("Target motion does not move warning"),WarningCenter.Equals(B->TailEffects->GetWarningCenter()));
 Test->TestEqual(TEXT("Warning radius is locked for this cast"),B->TailEffects->GetWarningRadius(),WarningRadius);
 C->SetPaused(true);const auto Tip=C->TipPosition;const float Clock=C->GetCombatClock();C->TickComponent(.5f,LEVELTICK_All,nullptr);
 Test->TestEqual(TEXT("Pause freezes combat clock"),C->GetCombatClock(),Clock);Test->TestTrue(TEXT("Pause freezes tail"),Tip.Equals(C->TipPosition));C->SetPaused(false);
 if(auto* AI=Cast<ACoreMorphAIController>(B->GetController()))AI->GetBrainComponent()->PauseLogic(TEXT("Manual checks"));
 AdvanceScorpion(C,3.6f);
 Test->TestEqual(TEXT("One sweep resolves once"),C->HitCount,1);
 Test->TestEqual(TEXT("AOE damages multiple pieces on one ASC only once"),Inside->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),975.f);
 Test->TestEqual(TEXT("Outside warning radius is unharmed"),Outside->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),1000.f);
 Test->TestFalse(TEXT("Charge and burst expire after completed cast"),B->TailEffects->IsCharging() || B->TailEffects->IsBlasting());
 Test->TestFalse(TEXT("Burst Cue tag expires"),ASC->HasMatchingGameplayTag(TAG_GameplayCue_CoreMorph_TailBlast));
 Test->TestEqual(TEXT("Static cover shields a target inside the blast"),Covered->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),1000.f);
 Covered->Destroy();Cover->Destroy();Inside->Destroy();Outside->Destroy();C->BlastRadius=WarningRadius;Victim->AddActorWorldOffset(FVector(0,-180,0));
 Test->TestEqual(TEXT("Tail applies GE health damage once"),Victim->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),975.f);
 Test->TestFalse(TEXT("Recovery ends attack GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Attacking));
 Test->TestTrue(TEXT("Completed GA applies cooldown GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_TailCooldown));
 Test->TestFalse(TEXT("Cooldown prevents immediate repeat"),B->UseRandomSkill(Victim,EEnemySkillRange::Near));
 for(float Time:{.3f,2.1f,2.5f})
 {
  ClearCooldown(B);Test->TestTrue(TEXT("Strike restarts after clearing test cooldown"),B->UseRandomSkill(Victim,EEnemySkillRange::Near));AdvanceScorpion(C,Time);
  auto* Spec=ASC->FindAbilitySpecFromClass(UGA_CoreMorphTailStrike::StaticClass());ASC->CancelAbilityHandle(Spec->Handle);const int32 Hits=C->HitCount;
  Test->TestFalse(TEXT("Cancel destroys charge and blast"),B->TailEffects->IsCharging() || B->TailEffects->IsBlasting());
  Test->TestEqual(TEXT("Cancel removes telegraph and lights"),TInlineComponentArray<UDecalComponent*>(B).Num()+TInlineComponentArray<UPointLightComponent*>(B).Num(),0);
  Test->TestFalse(TEXT("Cancel releases attack GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Attacking));AdvanceScorpion(C,1.3f);
  Test->TestEqual(TEXT("Cancelled recovery cannot deal further damage"),C->HitCount,Hits);Test->TestTrue(TEXT("Cancelled tail recovers"),C->Action==ECoreMorphScorpionAction::Idle);
 }
 const float BeforeWall=Victim->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute());
 auto* Wall=B->GetWorld()->SpawnActor<AStaticMeshActor>(M->GetGroundLocation()+B->GetActorForwardVector()*2750+FVector(0,0,1100),FRotator::ZeroRotator,P);
 Wall->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);Wall->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));Wall->SetActorScale3D(FVector(1,30,30));Wall->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
 ClearCooldown(B);B->UseRandomSkill(Victim,EEnemySkillRange::Near);AdvanceScorpion(C,3.6f);
 Test->TestTrue(TEXT("Obstacle intercepts tail sweep"),C->BlockedStrikes>0);Test->TestEqual(TEXT("Wall shields target health"),Victim->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),BeforeWall);Wall->Destroy();
 for(float DeathTime:{.3f,2.1f,2.5f,3.4f})
 {
  auto* D=B->GetWorld()->SpawnActor<ACoreMorphBoss>(B->GetClass(),FVector(-16000,16000,2500),FRotator::ZeroRotator,P);
  if(!Test->TestTrue(TEXT("Fresh owner initializes both forms"),D && PrepareScorpion(D)))continue;
  auto* DC=D->ScorpionCombat.Get();Victim->SetActorLocation(D->ScorpionMovement->GetGroundLocation()+D->GetActorForwardVector()*3300+FVector(0,0,90));DC->Target=Victim;
  Test->TestTrue(TEXT("Death check activates strike GA"),D->UseRandomSkill(Victim,EEnemySkillRange::Near));AdvanceScorpion(DC,DeathTime);
  auto* Damage=NewObject<UGameplayEffect>();auto& Mod=Damage->Modifiers.AddDefaulted_GetRef();Mod.Attribute=UEnemyAttributeSetBase::GetHealthAttribute();Mod.ModifierOp=EGameplayModOp::Additive;Mod.ModifierMagnitude=FScalableFloat(-1000);
  auto* DA=D->GetAbilitySystemComponent();DA->ApplyGameplayEffectToSelf(Damage,1,DA->MakeEffectContext());
  Test->TestTrue(TEXT("Death interrupts every strike stage"),D->IsDead() && !DC->IsAttacking() && !DC->IsDrivingPose() && !DC->IsComponentTickEnabled());
  Test->TestEqual(TEXT("Death clears foot and tail solver state"),D->ScorpionMovement->GetFeet().Num()+DC->GetTailNodes().Num(),0);
  Test->TestEqual(TEXT("Death removes all Cue pools"),TInlineComponentArray<UInstancedStaticMeshComponent*>(D).Num(),0);
  D->Destroy();
 }
 B->SetCombatPhase(2);Test->TestTrue(TEXT("Phase switching preserves scorpion identity and grants"),B->CurrentForm==ECoreMorphForm::Scorpion && ASC->GetActivatableAbilities().Num()==3);B->SetCombatPhase(1);
 Victim->SetActorLocation(M->GetGroundLocation()+B->GetActorForwardVector()*3300+FVector(0,0,90));ClearCooldown(B);C->Target=Victim;
 Test->TestTrue(TEXT("Target-loss check starts GA"),B->UseRandomSkill(Victim,EEnemySkillRange::Near));AdvanceScorpion(C,.2f);Victim->Destroy();AdvanceScorpion(C,.1f);
 Test->TestFalse(TEXT("Target destruction cancels attack state"),C->IsAttacking());
 Test->TestTrue(TEXT("Tail joints stay below twenty degrees through every stage"),TailMaxBend<=20.01);
 Test->TestTrue(TEXT("Rigid stinger follows the arc without a folded wrist"),TailMaxEndBend<=16.01);
 Test->TestTrue(TEXT("All tail and stinger lengths remain rigid"),TailLengthError<.01);
 Test->AddInfo(FString::Printf(TEXT("Tail geometry: max joint %.3f deg, stinger %.3f deg, length error %.6f cm"),TailMaxBend,TailMaxEndBend,TailLengthError));
 B->ResetFlightPreview();
 Test->TestEqual(TEXT("Reset removes destination armor and transient effects"),TInlineComponentArray<UStaticMeshComponent*>(B).Num(),154);
 return true;
}
class FScorpionRealtime : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;double Started=0;bool WalkShot=false,WindShot=false,ThrustShot=false,BlastShot=false;
public:
 explicit FScorpionRealtime(FAutomationTestBase* T):Test(T){}
 bool Update() override
 {
  auto* B=ScorpionBoss();if(!B)return true;auto* C=B->ScorpionCombat.Get();
  if(!Started){Started=FPlatformTime::Seconds();B->ResetFlightPreview();for(TActorIterator<ACoreMorphFlightReview> I(B->GetWorld());I;++I){I->PlayFlight();break;}if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(1280,720);return false;}
  auto Shot=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("CoreMorphMigration")/Name),false,false);};
  if(!WalkShot && B->ScorpionMovement->DistanceTravelled>700){WalkShot=true;Shot(TEXT("Scorpion-Walk.png"));}
  if(!WindShot && C->Action==ECoreMorphScorpionAction::Windup && C->ActionTime>1.6f){WindShot=true;Shot(TEXT("Scorpion-Windup.png"));}
  if(!ThrustShot && C->Action==ECoreMorphScorpionAction::Thrust && C->ActionTime>.15f){ThrustShot=true;Shot(TEXT("Scorpion-Thrust.png"));}
  if(!BlastShot && B->TailEffects->IsBlasting()){BlastShot=true;Shot(TEXT("Scorpion-Thunder.png"));}
  if((C->StrikeCount<1 || C->IsAttacking()) && FPlatformTime::Seconds()-Started<100)return false;
  Test->TestTrue(TEXT("Saved main BT runs complete flight, morph, approach and GA strike"),C->StrikeCount>=1 && WalkShot && WindShot && ThrustShot && BlastShot);
  Test->TestTrue(TEXT("Realtime GA makes actual contact with the review target"),C->HitCount>=1);
  Test->TestTrue(TEXT("Runtime BT remains active on the possessed boss"),Cast<UBehaviorTreeComponent>(Cast<AAIController>(B->GetController())->GetBrainComponent())->IsRunning());
  Test->TestEqual(TEXT("No repeated grants after realtime form change"),B->GetAbilitySystemComponent()->GetActivatableAbilities().Num(),3);
  ClearCooldown(B);Test->TestTrue(TEXT("PIE exits with a new active strike"),B->UseRandomSkill(C->Target,EEnemySkillRange::Near) && C->IsAttacking());
  ExitOwner=B;if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(0,0);return true;
 }
};
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FScorpionExited,FAutomationTestBase*,Test);
bool FScorpionExited::Update(){Test->TestNull(TEXT("Scorpion PIE closed"),GEditor->PlayWorld);Test->TestFalse(TEXT("Active motor owner destroyed"),ExitOwner.IsValid());ExitOwner.Reset();return true;}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphScorpionBatch,"TheManTest.Enemy.CoreMorph.ScorpionBatch",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCoreMorphScorpionBatch::RunTest(const FString&)
{
 if(!FEditorFileUtils::LoadMap(FPaths::ProjectContentDir()/TEXT("Maps/CoreMorph/L_CoreMorphScorpion.umap"),false,false))return false;
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));ADD_LATENT_AUTOMATION_COMMAND(FCheckCoreMorphScorpion(this));ADD_LATENT_AUTOMATION_COMMAND(FScorpionRealtime(this));ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.3f));ADD_LATENT_AUTOMATION_COMMAND(FScorpionExited(this));return true;
}
#endif
