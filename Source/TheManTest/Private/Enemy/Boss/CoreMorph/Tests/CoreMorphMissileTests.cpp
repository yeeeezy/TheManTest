#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"
#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphMissileCombat.h"
#include "Enemy/Boss/CoreMorph/Effects/CoreMorphMissileEffects.h"
#include "Enemy/Boss/CoreMorph/Combat/CoreMorphScorpionCombat.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphMissileBarrage.h"
#include "AbilitySystemComponent.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
namespace
{
ACoreMorphBoss* MissileBoss(){if(GEditor->PlayWorld)for(TActorIterator<ACoreMorphBoss> I(GEditor->PlayWorld);I;++I)return *I;return nullptr;}
TWeakObjectPtr<ACoreMorphBoss> MissileExitOwner;
void MissileHealth(AEnemyBase* B,float Value)
{
 auto* GE=NewObject<UGameplayEffect>();for(auto A:{UEnemyAttributeSetBase::GetMaxHealthAttribute(),UEnemyAttributeSetBase::GetHealthAttribute()}){auto& M=GE->Modifiers.AddDefaulted_GetRef();M.Attribute=A;M.ModifierOp=EGameplayModOp::Override;M.ModifierMagnitude=FScalableFloat(Value);}auto* ASC=B->GetAbilitySystemComponent();ASC->ApplyGameplayEffectToSelf(GE,1,ASC->MakeEffectContext());
}
float MissileHealthOf(AEnemyBase* B){return B->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute());}
void MissileAdvance(ACoreMorphBoss* B,float Seconds)
{for(float Left=Seconds;Left>KINDA_SMALL_NUMBER;Left-=1.f/120){const float Dt=FMath::Min(Left,1.f/120);B->Flight->TickComponent(Dt,LEVELTICK_All,nullptr);B->MissileCombat->TickComponent(Dt,LEVELTICK_All,nullptr);B->MissileEffects->TickComponent(Dt,LEVELTICK_All,nullptr);}}
void MissileCooldown(ACoreMorphBoss* B){B->GetAbilitySystemComponent()->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(TAG_State_CoreMorph_MissileCooldown));}
void CancelMissiles(ACoreMorphBoss* B){auto* S=B->GetAbilitySystemComponent()->FindAbilitySpecFromClass(UGA_CoreMorphMissileBarrage::StaticClass());if(S)B->GetAbilitySystemComponent()->CancelAbilityHandle(S->Handle);}
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckCoreMorphMissiles,FAutomationTestBase*,Test);
bool FCheckCoreMorphMissiles::Update()
{
 auto* B=MissileBoss();if(!Test->TestNotNull(TEXT("Manta boss in actual PIE"),B))return true;
 B->ResetFlightPreview();auto* C=B->MissileCombat.Get();auto* FX=B->MissileEffects.Get();auto* ASC=B->GetAbilitySystemComponent();C->RandomSeed=731;
 FActorSpawnParameters P;P.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto SpawnVictim=[&](FVector At){auto* V=B->GetWorld()->SpawnActor<AEnemyBase>(AEnemyBase::StaticClass(),At,FRotator::ZeroRotator,P);V->GetCharacterMovement()->DisableMovement();V->GetCharacterMovement()->SetComponentTickEnabled(false);MissileHealth(V,1000);return V;};
 auto* Target=SpawnVictim(FVector(0,0,90));
 Test->TestTrue(TEXT("Manta flight GA starts"),B->StartFlightPreview());Test->TestEqual(TEXT("Flight, morph, near strike and far barrage each granted once"),ASC->GetActivatableAbilities().Num(),4);
 Test->TestTrue(TEXT("Current phase far pool starts missile GA alongside flight"),B->UseRandomSkill(Target,EEnemySkillRange::Far));
 if(!C->IsActive()){Target->Destroy();return true;}
 const auto Plans=C->GetMissiles();Test->TestEqual(TEXT("Four distinct random bomb regions"),Plans.Num(),4);Test->TestEqual(TEXT("Four live red ground telegraphs"),FX->GetWarningCount(),Plans.Num());
 const float Radius=C->GetLockedRadius();for(int32 I=0;I<Plans.Num();++I)for(int32 J=I+1;J<Plans.Num();++J)Test->TestTrue(TEXT("Warning circles do not overlap"),FVector::Dist2D(Plans[I].Ground,Plans[J].Ground)>2*Radius);
 Target->SetActorLocation(FVector(8000,8000,90));C->ImpactRadius=100;MissileAdvance(B,.2f);
 for(int32 I=0;I<Plans.Num();++I)Test->TestTrue(TEXT("Moving target does not drag the warned regions"),C->GetMissiles()[I].Ground.Equals(Plans[I].Ground));
 Test->TestEqual(TEXT("Damage radius stays locked to displayed radius"),C->GetLockedRadius(),Radius);
 TArray<AEnemyBase*> Victims;for(const auto& Plan:Plans)Victims.Add(SpawnVictim(Plan.Ground+Plan.Normal*90));
 for(int32 I=0;I<3;++I){auto* S=NewObject<USphereComponent>(Victims[0]);S->SetupAttachment(Victims[0]->GetRootComponent());S->SetSphereRadius(100);S->SetRelativeLocation(FVector(I*60,0,0));S->SetCollisionProfileName(TEXT("Pawn"));S->RegisterComponent();}
 auto* Outside=SpawnVictim(FVector(13000,13000,90));
 B->Flight->SetPaused(true);const float Clock=C->GetClock();MissileAdvance(B,.5f);Test->TestEqual(TEXT("Pause freezes the missile timeline"),C->GetClock(),Clock);B->Flight->SetPaused(false);
 B->ProcessEvent(B->FindFunctionChecked(TEXT("HandleMidRoundStrengthIncrease")),nullptr);
 int32 Impacts=0,Launches=0;auto Hit=C->OnImpact.AddLambda([&](int32){++Impacts;});
 for(int32 Step=0;Step<240;++Step)
 {
  const auto Before=C->GetMissiles();MissileAdvance(B,1.f/120);
  for(int32 I=0;I<C->GetMissiles().Num();++I)if(!Before[I].bLaunched && C->GetMissiles()[I].bLaunched){++Launches;Test->TestTrue(TEXT("Each missile launches from the live moving energy core"),C->GetMissiles()[I].Launch.Equals(C->GetCoreLocation(),.01));}
 }
 Test->TestEqual(TEXT("Four missiles launched independently"),Launches,4);Test->TestEqual(TEXT("Warnings precede every explosion"),Impacts,0);
 for(const auto& M:C->GetMissiles())Test->TestTrue(TEXT("Projectile snapshots strength exactly once at launch"),FMath::IsNearlyEqual(M.Damage,24.f));
 for(int32 I=0;I<20;++I)B->ProcessEvent(B->FindFunctionChecked(TEXT("HandleMidRoundStrengthIncrease")),nullptr);
 MissileAdvance(B,4);C->OnImpact.Remove(Hit);Test->TestEqual(TEXT("Four real ground impacts"),Impacts,4);
 for(auto* V:Victims){Test->TestTrue(TEXT("Each region applies one launch-scaled GE, including multiple pieces"),FMath::IsNearlyEqual(MissileHealthOf(V),976.f,.01f));V->Destroy();}
 Test->TestEqual(TEXT("Outside regions receive no damage"),MissileHealthOf(Outside),1000.f);Outside->Destroy();
 Test->TestFalse(TEXT("Natural completion removes Cue and flight data"),FX->HasCue() || C->IsActive());Test->TestEqual(TEXT("Warnings and blast lights are gone"),FX->GetWarningCount()+TInlineComponentArray<UPointLightComponent*>(B).Num(),0);
 Test->TestTrue(TEXT("Barrage completion keeps the flight GA running"),B->Flight->IsFlying());Test->TestTrue(TEXT("Missile cooldown comes from GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_MissileCooldown));
 Test->TestFalse(TEXT("Cooldown blocks repeated barrage"),B->UseRandomSkill(Target,EEnemySkillRange::Far));C->ImpactRadius=Radius;
 for(float Time:{.2f,1.3f,3.3f})
 {
  MissileCooldown(B);Test->TestTrue(TEXT("Cancellation trial starts far GA"),B->UseRandomSkill(Target,EEnemySkillRange::Far));int32 Hits=0;auto H=C->OnImpact.AddLambda([&](int32){++Hits;});MissileAdvance(B,Time);for(const auto& M:C->GetMissiles())if(M.bLaunched)Test->TestTrue(TEXT("Capped strength snapshots base20 times2 without accumulation"),FMath::IsNearlyEqual(M.Damage,40.f));CancelMissiles(B);const int32 Before=Hits;MissileAdvance(B,4);C->OnImpact.Remove(H);
  Test->TestEqual(TEXT("Cancelled missiles cannot damage later"),Hits,Before);Test->TestFalse(TEXT("Cancel removes all missile visuals and simulation"),FX->HasCue() || C->IsActive());Test->TestTrue(TEXT("Cancelling attack preserves flight"),B->Flight->IsFlying());
 }
 MissileCooldown(B);Test->TestTrue(TEXT("Morph trial starts missile GA"),B->UseRandomSkill(Target,EEnemySkillRange::Far));MissileAdvance(B,1.2f);Test->TestTrue(TEXT("Manual morph accepts active barrage"),B->StartReassembly());
 Test->TestFalse(TEXT("Morph immediately clears barrage and its Cue"),C->IsActive() || FX->HasCue());Test->TestFalse(TEXT("Cannot shoot missiles while morphing"),B->UseRandomSkill(Target,EEnemySkillRange::Far));
 for(int32 I=0;I<330;++I)B->Reassembly->TickComponent(1.f/60,LEVELTICK_All,nullptr);
 Test->TestFalse(TEXT("Scorpion cannot use manta missiles"),B->UseRandomSkill(Target,EEnemySkillRange::Far));Test->TestEqual(TEXT("Morph grants no duplicate skills"),ASC->GetActivatableAbilities().Num(),4);
 for(float Time:{.2f,1.3f,3.3f})
 {
  auto* D=B->GetWorld()->SpawnActor<ACoreMorphBoss>(B->GetClass(),FVector(16000,0,6500),FRotator::ZeroRotator,P);D->Flight->FlightRoute=B->Flight->FlightRoute;D->StartFlightPreview();Test->TestTrue(TEXT("Death trial starts barrage"),D->UseRandomSkill(Target,EEnemySkillRange::Far));MissileAdvance(D,Time);for(const auto& Missile:D->MissileCombat->GetMissiles())if(Missile.bLaunched)Test->TestTrue(TEXT("Fresh boss missiles retain base20 damage"),FMath::IsNearlyEqual(Missile.Damage,20.f));
  auto* GE=NewObject<UGameplayEffect>();auto& M=GE->Modifiers.AddDefaulted_GetRef();M.Attribute=UEnemyAttributeSetBase::GetHealthAttribute();M.ModifierOp=EGameplayModOp::Additive;M.ModifierMagnitude=FScalableFloat(-100000);auto* DA=D->GetAbilitySystemComponent();DA->ApplyGameplayEffectToSelf(GE,1,DA->MakeEffectContext());
  Test->TestTrue(TEXT("Death cancels missiles at warning, flight and blast stages"),D->IsDead() && !D->MissileCombat->IsActive() && !D->MissileEffects->HasCue());Test->TestEqual(TEXT("Death removes all owner VFX pools"),TInlineComponentArray<UInstancedStaticMeshComponent*>(D).Num(),0);D->Destroy();
 }
 Target->Destroy();B->ResetFlightPreview();
 for(const auto& Piece:B->Flight->GetPieces())Test->TestTrue(TEXT("Reset after scorpion restores visible, undissolved manta armor"),Piece->IsVisible() && Piece->GetCustomPrimitiveData().Data[0]>1.f);
 return true;
}
class FMissileRealtime : public IAutomationLatentCommand
{
 FAutomationTestBase* Test;double Started=0;bool Warn=false,Launch=false,Impact=false,Fired=false;int32 Hits=0;FDelegateHandle Hit;
public:
 explicit FMissileRealtime(FAutomationTestBase* T):Test(T){}
 bool Update() override
 {
  auto* B=MissileBoss();if(!B)return true;auto* C=B->MissileCombat.Get();
  if(!Started){Started=FPlatformTime::Seconds();B->ResetFlightPreview();B->StartFlightPreview();if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(1280,720);Hit=C->OnImpact.AddLambda([this](int32){++Hits;});return false;}
  if(!Fired){Fired=true;Test->TestTrue(TEXT("Realtime far phase skill activates"),B->UseRandomSkill(B->ScorpionCombat->ReviewTarget,EEnemySkillRange::Far));}
  auto Shot=[&](const TCHAR* Name){FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("CoreMorphMigration")/Name),false,false);};
  if(!Warn && C->GetClock()>.3f){Warn=true;Shot(TEXT("MantaMissiles-Warning.png"));}
  if(!Launch && C->GetClock()>1.8f){Launch=true;Shot(TEXT("MantaMissiles-Flight.png"));}
  if(!Impact && Hits>0 && !C->GetMissiles().IsEmpty() && C->GetClock()-C->GetMissiles()[0].ImpactAt>.15f){Impact=true;Shot(TEXT("MantaMissiles-Impact.png"));}
  if(C->IsActive() && FPlatformTime::Seconds()-Started<25)return false;
  Test->TestTrue(TEXT("Rendered barrage completes all four impacts"),Hits==4 && Warn && Launch && Impact);C->OnImpact.Remove(Hit);
  MissileCooldown(B);Test->TestTrue(TEXT("PIE exits with active barrage"),B->UseRandomSkill(B->ScorpionCombat->ReviewTarget,EEnemySkillRange::Far));MissileAdvance(B,1.2f);MissileExitOwner=B;if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(0,0);return true;
 }
};
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FMissileExited,FAutomationTestBase*,Test);
bool FMissileExited::Update(){Test->TestNull(TEXT("Missile PIE closed"),GEditor->PlayWorld);Test->TestFalse(TEXT("Active missile owner destroyed"),MissileExitOwner.IsValid());MissileExitOwner.Reset();return true;}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphMissileBatch,"TheManTest.Enemy.CoreMorph.MissileBatch",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCoreMorphMissileBatch::RunTest(const FString&)
{
 if(!FEditorFileUtils::LoadMap(FPaths::ProjectContentDir()/TEXT("Maps/CoreMorph/L_CoreMorphMantaCombat.umap"),false,false))return false;
 ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));ADD_LATENT_AUTOMATION_COMMAND(FCheckCoreMorphMissiles(this));ADD_LATENT_AUTOMATION_COMMAND(FMissileRealtime(this));ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.3f));ADD_LATENT_AUTOMATION_COMMAND(FMissileExited(this));return true;
}
#endif
