#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Core/_Shared/Feedback/HitStopSubsystem.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Enemy/_Shared/GAS/GameplayCues/GCN_EnemyHit.h"
#include "Enemy/_Shared/Audio/EnemyHitAudioComponent.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "HAL/PlatformTime.h"

namespace
{
class FHitStopPainCommand : public IAutomationLatentCommand
{
	FAutomationTestBase* Test;
	int Stage=0;
	double Start=0;
	TWeakObjectPtr<AExplosionGunBullet> Bullet;
	TWeakObjectPtr<ACharacter> EnemyA, EnemyB;
public:
	explicit FHitStopPainCommand(FAutomationTestBase* In):Test(In){}
	virtual bool Update() override
	{
		UWorld* World=GEditor?GEditor->PlayWorld:nullptr;
		if(!World){Test->AddError(TEXT("Missing PIE world"));return true;}
		auto* Stop=World->GetSubsystem<UHitStopSubsystem>();
		auto* WS=World->GetWorldSettings();
		const double Now=FPlatformTime::Seconds();
		if(Stage==0)
		{
			if(!Test->TestNotNull(TEXT("World feedback manager exists"),Stop))return true;
			FHitStopSettings Settings;
			Test->TestEqual(TEXT("Near blast full hit stop"),Settings.GetStrength(100),1.f);
			Test->TestEqual(TEXT("Far blast no hit stop"),Settings.GetStrength(1600),0.f);
			Test->TestEqual(TEXT("Mid distance attenuates"),Settings.GetStrength(850),.5f);
			Settings.bEnabled=false;
			Test->TestEqual(TEXT("Disabled preset never affects time"),Settings.GetStrength(0),0.f);
			Test->TestFalse(TEXT("Zero duration ignored"),Stop->RequestHitStop(0,.05f,.12f));
			WS->SetTimeDilation(.5f);
			Test->TestTrue(TEXT("Short hit stop starts"),Stop->RequestHitStop(.08f,.1f,.12f));
			Test->TestEqual(TEXT("Hit stop scales previous world speed"),WS->TimeDilation,.05f);
			Stop->RequestHitStop(.3f,.5f,.12f);
			Test->TestEqual(TEXT("Weaker overlapping request does not weaken current stop"),WS->TimeDilation,.05f);
			Start=Now;Stage=1;return false;
		}
		if(Stage==1)
		{
			if(Now-Start<.25)return false;
			Test->TestFalse(TEXT("Real-time cap expires despite slow game time"),Stop->IsHitStopActive());
			Test->TestEqual(TEXT("Restores previous 0.5, not hardcoded 1"),WS->TimeDilation,.5f);
			Stop->RequestHitStop(.1f,.2f,.12f);Stop->CancelHitStop();
			Test->TestEqual(TEXT("Cancellation restores original speed"),WS->TimeDilation,.5f);
			Stop->RequestHitStop(.1f,.2f,.12f);WS->SetTimeDilation(.75f);Stop->Tick(0);
			Test->TestFalse(TEXT("External speed change relinquishes ownership"),Stop->IsHitStopActive());
			Test->TestEqual(TEXT("Does not overwrite external speed"),WS->TimeDilation,.75f);
			WS->SetTimeDilation(1.f);Start=Now;Stage=2;return false;
		}
		if(Stage==2)
		{
			if(Now-Start<.1)return false;
			const FVector Origin=World->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation()+FVector(0,0,100);
			FHitStopSettings Settings;
			Test->TestFalse(TEXT("Distant actual request ignored"),Stop->RequestHitStopAtLocation(Origin+FVector(10000,0,0),Settings));
			FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto* Class=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
			auto* Shot=World->SpawnActor<AExplosionGunBullet>(Class,Origin,FRotator::ZeroRotator,Spawn);
			Bullet=Shot;Shot->ExplosionDelay=0;Shot->HitStop.Duration=.15f;Shot->HitStop.MaxContinuousDuration=.2f;
			Shot->ChaosRadius=0;Shot->ExplosionCueTag=FGameplayTag(); // Gameplay feedback works without a presentation Cue.
			FHitResult Hit;Hit.ImpactPoint=Origin;Hit.ImpactNormal=FVector::UpVector;
			Shot->ProcessHit(Hit,nullptr,nullptr);
			Test->TestFalse(TEXT("Initial environment attachment is not the hit-stop trigger"),Stop->IsHitStopActive());
			Start=Now;Stage=3;return false;
		}
		if(Stage==3)
		{
			if(Bullet.IsValid())return false;
			Test->TestTrue(TEXT("Detonation requests stop independently of destroyed bullet and missing GC"),Stop->IsHitStopActive());
			Start=Now;Stage=4;return false;
		}
		if(Stage==4)
		{
			if(Now-Start<.3)return false;
			Test->TestFalse(TEXT("Stop expires after projectile destruction"),Stop->IsHitStopActive());
			Test->TestEqual(TEXT("Gameplay speed fully restored"),WS->TimeDilation,1.f);
			FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			EnemyA=World->SpawnActor<ACharacter>(FVector(100,300,100),FRotator::ZeroRotator,Spawn);
			EnemyB=World->SpawnActor<ACharacter>(FVector(100,500,100),FRotator::ZeroRotator,Spawn);
			auto* Cue=LoadClass<UGCN_EnemyHit>(nullptr,TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"))->GetDefaultObject<UGCN_EnemyHit>();
			FGameplayCueParameters P;P.Normal=FVector::UpVector;
			P.Location=EnemyA->GetActorLocation();Cue->OnExecute_Implementation(EnemyA.Get(),P);
			P.Location=EnemyB->GetActorLocation();Cue->OnExecute_Implementation(EnemyB.Get(),P);
			auto* A=EnemyA->FindComponentByClass<UEnemyHitAudioComponent>();
			auto* B=EnemyB->FindComponentByClass<UEnemyHitAudioComponent>();
			if(!A||!B){Test->AddError(TEXT("Pain components not created by Hit Cue"));return true;}
			Test->TestTrue(TEXT("Two enemies can play independently"),A->GetPainVoice()&&B->GetPainVoice()&&A->GetPainVoice()!=B->GetPainVoice());
			Test->TestFalse(TEXT("Same enemy never overlaps pain"),A->TryPlayPain(Cue->PainSound,P.Location,1,0));
			if(A->GetPainVoice())A->GetPainVoice()->Stop();
			Test->TestFalse(TEXT("Cooldown survives voice stopping"),A->TryPlayPain(Cue->PainSound,P.Location,1,.6f));
			Start=Now;Stage=5;return false;
		}
		if(Now-Start<.7)return false;
		auto* Cue=LoadClass<UGCN_EnemyHit>(nullptr,TEXT("/Game/Enemy/_Shared/GAS/GameplayCues/GC_Character_Enemy_Hit.GC_Character_Enemy_Hit_C"))->GetDefaultObject<UGCN_EnemyHit>();
		auto* Audio=EnemyA->FindComponentByClass<UEnemyHitAudioComponent>();
		Test->TestTrue(TEXT("Pain can play again after cooldown"),Audio->TryPlayPain(Cue->PainSound,EnemyA->GetActorLocation(),1,.6f));
		TWeakObjectPtr<UAudioComponent> Voice=Audio->GetPainVoice();
		EnemyA->Destroy();EnemyB->Destroy();
		Test->TestTrue(TEXT("Owner destruction stops pain voice"),!Voice.IsValid()||!Voice->IsPlaying());
		return true;
	}
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHitStopPainTest,"TheManTest.Feedback.HitStopAndPain",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FHitStopPainTest::RunTest(const FString&)
{
	AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FHitStopPainCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
