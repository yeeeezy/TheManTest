#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Enemy/Boss/CoreMorph/CoreMorphBoss.h"
#include "Enemy/Boss/CoreMorph/Transformation/CoreMorphReassemblyComponent.h"
#include "Enemy/Boss/CoreMorph/Movement/CoreMorphFlightComponent.h"
#include "Enemy/Boss/CoreMorph/Review/CoreMorphFlightReview.h"
#include "Enemy/Boss/CoreMorph/GAS/Abilities/GA_CoreMorphReassemble.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"

namespace
{
ACoreMorphBoss* ReassemblyBoss(){if(GEditor->PlayWorld)for(TActorIterator<ACoreMorphBoss> It(GEditor->PlayWorld);It;++It)return *It;return nullptr;}
TWeakObjectPtr<ACoreMorphBoss> ClosingBoss;
void Advance(UCoreMorphReassemblyComponent* R,float Seconds)
{
    R->SetPaused(false);R->SetComponentTickEnabled(false);
    for(float Left=Seconds;Left>KINDA_SMALL_NUMBER;Left-=1.f/60.f)R->TickComponent(FMath::Min(Left,1.f/60.f),LEVELTICK_All,nullptr);
}
void Hurt(ACoreMorphBoss* Boss,float Amount)
{
    auto* GE=NewObject<UGameplayEffect>();GE->DurationPolicy=EGameplayEffectDurationType::Instant;
    auto& Mod=GE->Modifiers.AddDefaulted_GetRef();Mod.Attribute=UEnemyAttributeSetBase::GetHealthAttribute();Mod.ModifierOp=EGameplayModOp::Additive;Mod.ModifierMagnitude=FScalableFloat(-Amount);
    auto* ASC=Boss->GetAbilitySystemComponent();ASC->ApplyGameplayEffectToSelf(GE,1.f,ASC->MakeEffectContext());
}
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FCheckReassembly,FAutomationTestBase*,Test);
bool FCheckReassembly::Update()
{
    auto* Boss=ReassemblyBoss();if(!Test->TestNotNull(TEXT("Reassembly review boss exists in PIE"),Boss))return true;
    auto* R=Boss->Reassembly.Get();auto* ASC=Boss->GetAbilitySystemComponent();
    if(!Test->TestNotNull(TEXT("Cold-loaded reassembly layout"),R->Layout.Get()))return true;
    Test->TestEqual(TEXT("Layout contains both final assemblies"),R->Layout->Pieces.Num(),455);
    Test->TestEqual(TEXT("Flight and transformation are each granted once"),ASC->GetActivatableAbilities().Num(),3);
    Test->TestEqual(TEXT("One ASC for both forms"),TInlineComponentArray<UAbilitySystemComponent*>(Boss).Num(),1);
    Hurt(Boss,25);Boss->SetCombatPhase(2);
    for(float Time:{.2f,.8f,2.2f,4.8f})
    {
        Boss->ResetFlightPreview();
        const FTransform Before=Boss->Flight->GetPieces()[0]->GetComponentTransform();
        Test->TestTrue(TEXT("Transformation starts through the same ASC"),Boss->StartReassembly());
        Test->TestTrue(TEXT("Cue allocates the source ISM effects"),R->HasCue());
        Test->TestTrue(TEXT("Start preserves current manta pose without snapping"),Boss->Flight->GetPieces()[0]->GetComponentTransform().Equals(Before,.01));
        Test->TestTrue(TEXT("Transforming state is GE-owned"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Transforming));
        Test->TestFalse(TEXT("Reassembly cannot overlap itself"),Boss->StartReassembly());
        Test->TestFalse(TEXT("Flight cannot start during transformation"),Boss->StartFlightPreview());
        Advance(R,Time);
        R->SetPaused(true);const float Clock=R->GetSeconds();const FTransform Pose=R->GetPieces()[200]->GetComponentTransform();
        R->TickComponent(.3f,LEVELTICK_All,nullptr);
        Test->TestEqual(TEXT("Pause freezes construction and cue clock"),R->GetSeconds(),Clock);
        Test->TestTrue(TEXT("Pause freezes actual piece transforms"),R->GetPieces()[200]->GetComponentTransform().Equals(Pose));
        ASC->CancelAllAbilities();
        Test->TestFalse(TEXT("Cancellation removes transforming GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Transforming));
        Test->TestFalse(TEXT("Cancellation removes Cue tag"),ASC->HasMatchingGameplayTag(TAG_GameplayCue_CoreMorph_Reassembly));
        Test->TestEqual(TEXT("Cancellation destroys all transient instance pools"),TInlineComponentArray<UInstancedStaticMeshComponent*>(Boss).Num(),0);
        Test->TestEqual(TEXT("Cancellation destroys the impact light"),TInlineComponentArray<UPointLightComponent*>(Boss).Num(),0);
        Test->TestTrue(TEXT("Cancellation rolls back source form and exact pose"),Boss->CurrentForm==ECoreMorphForm::Manta && Boss->Flight->GetPieces()[0]->GetComponentTransform().Equals(Before,.01));
        for(int32 I=154;I<R->GetPieces().Num();++I)Test->TestTrue(TEXT("Cancelled destination is hidden and cannot be hit"),!R->GetPieces()[I]->IsVisible() && R->GetPieces()[I]->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
    }
    Boss->ResetFlightPreview();Boss->StartReassembly();Advance(R,3.f);
    UInstancedStaticMeshComponent* Wind=nullptr;
    for(auto* Pool:TInlineComponentArray<UInstancedStaticMeshComponent*>(Boss))
        if(Pool->GetMaterial(0)==R->Layout->SandWaveMaterial)Wind=Pool;
    if(Test->TestNotNull(TEXT("Cue owns the concentric wind walls"),Wind))
    {
        Test->TestEqual(TEXT("Three complete walls with body, crest and curl"),Wind->GetInstanceCount(),1152);
        Test->TestTrue(TEXT("Wind walls cannot intercept weapons"),Wind->GetCollisionEnabled()==ECollisionEnabled::NoCollision);
        FTransform Front[3];
        for(int32 Ring=0;Ring<3;++Ring)
        {
            Wind->GetInstanceTransform(Ring*384,Front[Ring]);
            Test->TestTrue(TEXT("Every ring is a tall vertical wall"),Front[Ring].GetScale3D().Z*100>1400);
            if(Ring>0)Test->TestTrue(TEXT("Concentric fronts stay separated"),FVector::Dist2D(Front[Ring-1].GetLocation(),Front[Ring].GetLocation())>1100);
        }
        R->SetPaused(true);R->TickComponent(.5f,LEVELTICK_All,nullptr);
        FTransform Paused;Wind->GetInstanceTransform(0,Paused);
        Test->TestTrue(TEXT("Pause freezes wind wall geometry"),Paused.Equals(Front[0]));
    }
    Advance(R,2.3f);
    Test->TestTrue(TEXT("Complete assembly commits Scorpion form"),Boss->CurrentForm==ECoreMorphForm::Scorpion && ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Form_Scorpion));
    Test->TestFalse(TEXT("Manta GE is removed at commit"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Form_Manta));
    Test->TestFalse(TEXT("Complete assembly releases transforming GE"),ASC->HasMatchingGameplayTag(TAG_State_CoreMorph_Transforming));
    Test->TestFalse(TEXT("Complete assembly ends GA"),ASC->FindAbilitySpecFromClass(UGA_CoreMorphReassemble::StaticClass())->IsActive());
    int32 Visible=0;
    for(const auto& C:R->GetPieces())
    {
        Visible+=C->IsVisible()?1:0;Test->TestTrue(TEXT("All surfaces retain original boss ownership"),C->GetOwner()==Boss);
        Test->TestFalse(TEXT("Every final pose remains finite"),C->GetComponentTransform().ContainsNaN());
    }
    Test->TestEqual(TEXT("Exactly 301 visible scorpion surfaces"),Visible,301);
    Test->TestEqual(TEXT("Morphing never heals or replaces Health"),ASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),75.f);
    Test->TestEqual(TEXT("Form and combat phase are independent"),Boss->GetCombatPhase(),2);
    Test->TestEqual(TEXT("No repeated ability grants across cancellations and forms"),ASC->GetActivatableAbilities().Num(),3);
    Test->TestTrue(TEXT("Successful commit keeps the source dust tail alive"),R->HasCue());
    Advance(R,4.2f);
    Test->TestFalse(TEXT("Dust tail expires its Cue"),R->HasCue() || ASC->HasMatchingGameplayTag(TAG_GameplayCue_CoreMorph_Reassembly));
    Test->TestEqual(TEXT("Expired effect pools leave no components"),TInlineComponentArray<UInstancedStaticMeshComponent*>(Boss).Num(),0);
    for(float DeathTime:{.3f,2.5f,5.5f})
    {
        FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* Victim=Boss->GetWorld()->SpawnActor<ACoreMorphBoss>(Boss->GetClass(),FVector(-16000,0,2500),FRotator::ZeroRotator,Spawn);
        if(!Test->TestNotNull(TEXT("Independent death-check boss"),Victim))continue;
        Test->TestTrue(TEXT("Blueprint component layout survives a fresh spawn"),Victim->StartReassembly());Advance(Victim->Reassembly,DeathTime);Hurt(Victim,1000);
        Test->TestTrue(TEXT("Death remains terminal in every construction stage"),Victim->IsDead());
        Test->TestFalse(TEXT("Death clears reassembly effects and ticking"),Victim->Reassembly->HasCue() || Victim->Reassembly->IsComponentTickEnabled());
        for(const auto& C:Victim->Reassembly->GetPieces())Test->TestTrue(TEXT("Death disables both assemblies without ragdoll"),C->GetCollisionEnabled()==ECollisionEnabled::NoCollision && !C->IsSimulatingPhysics());
        Victim->Destroy();
    }
    Boss->ResetFlightPreview();
    Test->TestEqual(TEXT("Review reset removes destination components"),TInlineComponentArray<UStaticMeshComponent*>(Boss).Num(),154);
    return true;
}
class FReassemblyRealtime : public IAutomationLatentCommand
{
    FAutomationTestBase* Test;double Started=0;int32 Shot=0;
public:
    explicit FReassemblyRealtime(FAutomationTestBase* In):Test(In){}
    bool Update() override
    {
        auto* Boss=ReassemblyBoss();if(!Boss){Test->AddError(TEXT("Missing realtime boss"));return true;}
        if(!Started)
        {
            Started=FPlatformTime::Seconds();Boss->ResetFlightPreview();
            for(TActorIterator<ACoreMorphFlightReview> It(Boss->GetWorld());It;++It){It->PlayFlight();break;}
            if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(1280,720);
            return false;
        }
        const float Seconds=Boss->Reassembly->GetSeconds();
        const float Times[]={.22f,.8f,1.5f,3.f,5.3f,9.15f};
        if(Shot<6 && Seconds>=Times[Shot])
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("CoreMorphMigration")/FString::Printf(TEXT("Reassembly-%d.png"),Shot)),false,false);++Shot;
        }
        if((Seconds<9.15f || Boss->Reassembly->HasCue()) && FPlatformTime::Seconds()-Started<65)return false;
        Test->TestEqual(TEXT("Actual world ticking reaches all construction review frames"),Shot,6);
        Test->TestTrue(TEXT("Full flight automatically hands off to reassembly"),Boss->CurrentForm==ECoreMorphForm::Scorpion);
        Test->TestFalse(TEXT("Realtime Cue tail is gone"),Boss->Reassembly->HasCue());
        Boss->ResetFlightPreview();Boss->StartReassembly();ClosingBoss=Boss;
        if(auto* V=GEngine->GameViewport->GetGameViewport())V->SetFixedViewportSize(0,0);
        return true;
    }
};
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FReassemblyExited,FAutomationTestBase*,Test);
bool FReassemblyExited::Update(){Test->TestNull(TEXT("PIE world closed"),GEditor->PlayWorld);Test->TestFalse(TEXT("Active reassembly owner destroyed on exit"),ClosingBoss.IsValid());ClosingBoss.Reset();return true;}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCoreMorphReassemblyPIE,"TheManTest.Enemy.CoreMorph.ReassemblyBatch",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FCoreMorphReassemblyPIE::RunTest(const FString& Parameters)
{
    if(!FEditorFileUtils::LoadMap(FPaths::ProjectContentDir()/TEXT("Maps/CoreMorph/L_CoreMorphReassembly.umap"),false,false))return false;
    ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.f));
    ADD_LATENT_AUTOMATION_COMMAND(FCheckReassembly(this));ADD_LATENT_AUTOMATION_COMMAND(FReassemblyRealtime(this));
    ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(.3f));ADD_LATENT_AUTOMATION_COMMAND(FReassemblyExited(this));return true;
}
#endif
