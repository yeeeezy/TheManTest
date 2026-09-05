#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"
#include "Editor.h"
#include "Weapons/ExplosionGun/Bullets/ExplosionGunBullet.h"
#include "Weapons/ExplosionGun/GAS/GameplayCues/GCN_ExplosionGunExplosion.h"
#include "Core/_Shared/GAS/TheManGameplayTags.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/EnemyAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "NiagaraComponent.h"

namespace
{
class FExplosionDamageCommand : public IAutomationLatentCommand
{
	FAutomationTestBase* Test;
	int Stage=0;
	float Start=0, PlayerHealth=0;
	TArray<TWeakObjectPtr<APhantom>> Enemies;
	TWeakObjectPtr<AActor> Wall;
	TWeakObjectPtr<AExplosionGunBullet> Bullet;
	FVector PlayerLocation;
public:
	explicit FExplosionDamageCommand(FAutomationTestBase* In):Test(In){}
	virtual bool Update() override
	{
		UWorld* W=GEditor?GEditor->PlayWorld:nullptr;
		if(!W){Test->AddError(TEXT("Missing world"));return true;}
		APawn* Player=W->GetFirstPlayerController()->GetPawn();
		auto* PlayerASC=Cast<IAbilitySystemInterface>(Player)->GetAbilitySystemComponent();
		const FVector Origin(10000,10000,150);
		if(Stage==0)
		{
			FActorSpawnParameters Spawn;Spawn.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			auto* EnemyClass=LoadClass<APhantom>(nullptr,TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
			for(const FVector Offset:{FVector(200,0,0),FVector(0,220,0),FVector(650,0,0)})
			{
				auto* E=W->SpawnActor<APhantom>(EnemyClass,Origin+Offset,FRotator::ZeroRotator,Spawn);
				E->SetCloaked(false);E->GetCharacterMovement()->DisableMovement();Enemies.Add(E);
			}
			Wall=W->SpawnActor<AActor>(Origin+FVector(0,100,0),FRotator::ZeroRotator,Spawn);
			auto* Box=NewObject<UBoxComponent>(Wall.Get());Wall->SetRootComponent(Box);Wall->AddInstanceComponent(Box);
			Box->SetBoxExtent(FVector(70,10,200));Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Box->SetCollisionObjectType(ECC_WorldStatic);Box->SetCollisionResponseToAllChannels(ECR_Block);Box->RegisterComponent();
			Box->SetWorldLocation(Origin+FVector(0,100,0));
			PlayerLocation=Player->GetActorLocation();Player->SetActorLocation(Origin+FVector(-200,0,0));
			PlayerHealth=PlayerASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute());
			auto* Class=LoadClass<AExplosionGunBullet>(nullptr,TEXT("/Game/Weapons/ExplosionGun/Blueprint/BP_ExplosionGunBullet.BP_ExplosionGunBullet_C"));
			auto* Shot=W->SpawnActor<AExplosionGunBullet>(Class,Origin,FRotator::ZeroRotator,Spawn);Bullet=Shot;
			Test->TestEqual(TEXT("Default radial damage"),Shot->ExplosionDamage,20.f);
			Test->TestEqual(TEXT("Default damage radius"),Shot->ExplosionDamageRadius,400.f);
			Shot->ExplosionDelay=.15f;Shot->ChaosRadius=0;Shot->HitStop.bEnabled=false;
			FHitResult Hit;Hit.ImpactPoint=Origin;Hit.ImpactNormal=FVector::UpVector;
			Shot->ProcessHit(Hit,nullptr,nullptr);Shot->ProcessHit(Hit,nullptr,nullptr);
			Test->TestEqual(TEXT("Environment hit deals no immediate enemy damage"),Enemies[0]->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),100.f);
			// Original enemy impact category must suppress the ground effect even with a valid ground hit.
			auto* Cue=LoadClass<UGCN_ExplosionGunExplosion>(nullptr,TEXT("/Game/Weapons/ExplosionGun/GAS/GameplayCues/GC_Weapon_ExplosionGun_Explosion.GC_Weapon_ExplosionGun_Explosion_C"))->GetDefaultObject<UGCN_ExplosionGunExplosion>();
			Test->TestNull(TEXT("Enemy effect deliberately unassigned"),Cue->EnemyExplosionEffect.Get());
			FGameplayCueParameters P;P.Location=Origin;P.Normal=FVector::UpVector;P.AggregatedTargetTags.AddTag(TAG_Data_Explosion_EnemyImpact);
			P.EffectContext=FGameplayEffectContextHandle(new FGameplayEffectContext());P.EffectContext.AddHitResult(Hit);
			Cue->OnExecute_Implementation(Shot,P);
			int Count=0;for(TObjectIterator<UNiagaraComponent> It;It;++It)
				if(It->GetWorld()==W&&It->GetAsset()==Cue->ExplosionEffect)++Count;
			Test->TestEqual(TEXT("Enemy explosion never falls back to the environment ground decal"),Count,0);
			Start=W->GetTimeSeconds();Stage=1;return false;
		}
		if(W->GetTimeSeconds()-Start<.4f)return false;
		Test->TestFalse(TEXT("Detonation consumes projectile exactly once"),Bullet.IsValid());
		Test->TestEqual(TEXT("Visible enemy takes one 20 damage despite multiple overlapping components"),Enemies[0]->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),80.f);
		Test->TestEqual(TEXT("Wall blocks explosion damage"),Enemies[1]->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),100.f);
		Test->TestEqual(TEXT("Outside-radius enemy unaffected"),Enemies[2]->GetAbilitySystemComponent()->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),100.f);
		Test->TestEqual(TEXT("Player inside radius unaffected"),PlayerASC->GetNumericAttribute(UEnemyAttributeSetBase::GetHealthAttribute()),PlayerHealth);
		for(auto E:Enemies)if(E.IsValid())E->Destroy();
		if(Wall.IsValid())Wall->Destroy();Player->SetActorLocation(PlayerLocation);
		return true;
	}
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FExplosionDamageTest,"TheManTest.Player.Weapons.ExplosionRadialDamage",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FExplosionDamageTest::RunTest(const FString&)
{
	AutomationOpenMap(TEXT("/Game/Maps/VFXTest/VFXTestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FExplosionDamageCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}
#endif
