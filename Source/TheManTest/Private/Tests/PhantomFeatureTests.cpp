#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Characters/Enemy/Components/EnemyMagazineComponent.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Characters/Enemy/Humanoid/Phantom/Phantom.h"
#include "Characters/Enemy/Cover/EnemyCoverPoint.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationCommon.h"
#include "Editor.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/BlendSpace.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomAnimationOverridesTest,
	"TheManTest.Enemy.Phantom.AnimationOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomAnimationOverridesTest::RunTest(const FString& Parameters)
{
	UAnimBlueprint* Child = LoadObject<UAnimBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/Logic/ABP_Phantom_OriginalRifle.ABP_Phantom_OriginalRifle"));
	TestNotNull(TEXT("Phantom child AnimBP"), Child);
	if (!Child) return false;

	TSet<FString> OverridePaths;
	for (const FAnimParentNodeAssetOverride& Override : Child->ParentAssetOverrides)
	{
		TestNotNull(TEXT("Every stored Phantom override has an asset"), Override.NewAsset.Get());
		if (Override.NewAsset) OverridePaths.Add(Override.NewAsset->GetPathName());
	}
	auto HasOverride = [&OverridePaths](const TCHAR* Path) { return OverridePaths.Contains(Path); };
	TestTrue(TEXT("Relaxed idle override"), HasOverride(TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Idle_IP.W2_Stand_Relaxed_Idle_IP")));
	TestTrue(TEXT("Patrol directional BlendSpace override"), HasOverride(TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_RelaxedPatrol2D.BS_Phantom_RelaxedPatrol2D")));
	TestTrue(TEXT("Aim directional BlendSpace override"), HasOverride(TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_AimLocomotion.BS_Phantom_AimLocomotion")));
	for (int32 Index = 1; Index <= 4; ++Index)
	{
		TestTrue(*FString::Printf(TEXT("Patrol scan variant %d override"), Index),
			HasOverride(*FString::Printf(TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Fgt_v%d_IP.W2_Stand_Relaxed_Fgt_v%d_IP"), Index, Index)));
	}

	UBlendSpace* Aim = LoadObject<UBlendSpace>(nullptr,
		TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_AimLocomotion.BS_Phantom_AimLocomotion"));
	TestNotNull(TEXT("Aim 2D BlendSpace"), Aim);
	if (const UBlendSpace* AimBlendSpace = Cast<UBlendSpace>(Aim))
	{
		const TArray<FBlendSample>& Samples = AimBlendSpace->GetBlendSamples();
		bool bHasNegativeDirection = false;
		bool bHasPositiveDirection = false;
		bool bHasMovingSample = false;
		bool bHasIdleCenter = false;
		for (const FBlendSample& Sample : Samples)
		{
			bHasNegativeDirection |= Sample.SampleValue.X < -KINDA_SMALL_NUMBER || Sample.SampleValue.Y < -KINDA_SMALL_NUMBER;
			bHasPositiveDirection |= Sample.SampleValue.X > KINDA_SMALL_NUMBER || Sample.SampleValue.Y > KINDA_SMALL_NUMBER;
			bHasMovingSample |= !Sample.SampleValue.IsNearlyZero();
			bHasIdleCenter |= Sample.SampleValue.IsNearlyZero();
		}
		TestTrue(TEXT("Aim BlendSpace has enough samples for directional interpolation"), Samples.Num() >= 9);
		TestTrue(TEXT("Aim BlendSpace covers negative direction"), bHasNegativeDirection);
		TestTrue(TEXT("Aim BlendSpace covers positive direction"), bHasPositiveDirection);
		TestTrue(TEXT("Aim BlendSpace has moving samples"), bHasMovingSample);
		TestTrue(TEXT("Aim BlendSpace has idle center"), bHasIdleCenter);
		AddInfo(FString::Printf(TEXT("PHANTOM_AIM_BLENDSPACE samples=%d"), Samples.Num()));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomReusableCombatTest,
	"TheManTest.Enemy.Phantom.ReusableCombatModules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomReusableCombatTest::RunTest(const FString& Parameters)
{
	UEnemyMagazineComponent* Magazine = NewObject<UEnemyMagazineComponent>();
	TestEqual(TEXT("Default magazine capacity"), Magazine->GetCapacity(), 20);
	for (int32 Index = 0; Index < 20; ++Index) TestTrue(TEXT("Configured round can be consumed"), Magazine->ConsumeRound());
	TestTrue(TEXT("Magazine reports empty"), Magazine->IsEmpty());
	TestFalse(TEXT("Empty magazine rejects extra shot"), Magazine->ConsumeRound());
	Magazine->Reload();
	TestEqual(TEXT("Reload restores capacity"), Magazine->GetCurrentAmmo(), 20);

	TestNotNull(TEXT("Cover mesh asset"), LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Enemy/_Shared/Cover/Mesh/SM_PhantomCover.SM_PhantomCover")));
	TestNotNull(TEXT("Cover actor blueprint"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/_Shared/Cover/BP_EnemyCoverPoint.BP_EnemyCoverPoint")));
	TestNotNull(TEXT("Burst ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/GAS/GameplayAbility/BGA_PhantomBurst.BGA_PhantomBurst")));
	TestNotNull(TEXT("Suppressive ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/GAS/GameplayAbility/BGA_PhantomSuppressiveFire.BGA_PhantomSuppressiveFire")));
	TestNotNull(TEXT("Reload ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/GAS/GameplayAbility/BGA_PhantomReload.BGA_PhantomReload")));
	TestNotNull(TEXT("Cover ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/GAS/GameplayAbility/BGA_PhantomTakeCover.BGA_PhantomTakeCover")));
	TestNotNull(TEXT("Barrage ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Phantom/GAS/GameplayAbility/BGA_PhantomAreaBarrage.BGA_PhantomAreaBarrage")));

	UWorld* World = LoadObject<UWorld>(nullptr, TEXT("/Game/Maps/TestMap.TestMap"));
	TestNotNull(TEXT("Runtime test map"), World);
	if (World)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.ObjectFlags = RF_Transient;
		AHumanoidEnemy* Humanoid = World->SpawnActor<AHumanoidEnemy>(AHumanoidEnemy::StaticClass(),
			FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
		TestNotNull(TEXT("Reusable humanoid spawned"), Humanoid);
		if (Humanoid)
		{
			Humanoid->SetAIState(EHumanoidEnemyAIState::Aim);
			Humanoid->StartLostTargetSearch(FVector(500.f, 0.f, 0.f));
			TestEqual(TEXT("Lost target enters SearchRush"), Humanoid->GetAIState(), EHumanoidEnemyAIState::SearchRush);
		}

		UClass* PhantomClass = LoadClass<APhantom>(nullptr,
			TEXT("/Game/Enemy/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
		APhantom* Phantom = World->SpawnActor<APhantom>(PhantomClass ? PhantomClass : APhantom::StaticClass(),
			FVector(200.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParameters);
		TestNotNull(TEXT("Phantom spawned"), Phantom);
		if (Phantom)
		{
			Phantom->SetCombatPhase(2);
			TestTrue(TEXT("Phase two enables cloak"), Phantom->IsCloaked());
			TestTrue(TEXT("Cloak enables projectile pass-through"), Phantom->ShouldProjectilePassThrough());
			TestEqual(TEXT("Projectile channel ignored while cloaked"),
				Phantom->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_GameTraceChannel1), ECR_Ignore);
			Phantom->SetCombatPhase(1);
			TestFalse(TEXT("Phase one disables cloak"), Phantom->IsCloaked());
		}

		AEnemyCoverPoint* Cover = World->SpawnActor<AEnemyCoverPoint>(AEnemyCoverPoint::StaticClass(),
			FVector(1000.f, 0.f, 0.f), FRotator::ZeroRotator, SpawnParameters);
		TestNotNull(TEXT("Reusable cover point spawned"), Cover);
		if (Cover)
		{
			Cover->SetActorLocation(FVector(1000.f, 0.f, 0.f));
			Cover->UpdateComponentTransforms();
			AEnemyCoverPoint* Selected = AEnemyCoverPoint::FindBestCover(World,
				FVector(800.f, 0.f, 0.f), FVector(1500.f, 0.f, 0.f), 1000.f);
			TestEqual(TEXT("Occluding cover is selected"), Selected, Cover);
		}
		if (Humanoid) Humanoid->Destroy();
		if (Phantom) Phantom->Destroy();
		if (Cover) Cover->Destroy();
	}
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FValidatePhantomPIECommand, FAutomationTestBase*, Test);
bool FValidatePhantomPIECommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	if (!World) return false;

	FActorSpawnParameters Params;
	Params.ObjectFlags = RF_Transient;
	UClass* PhantomClass = LoadClass<APhantom>(nullptr,
		TEXT("/Game/Enemy/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
	APhantom* Phantom = World->SpawnActor<APhantom>(PhantomClass,
		FVector(0.f, 0.f, 150.f), FRotator::ZeroRotator, Params);
	Test->TestNotNull(TEXT("PIE Phantom spawned"), Phantom);
	if (Phantom)
	{
		Test->TestNotNull(TEXT("PIE original Rifle mesh"), Phantom->GetMesh()->GetSkeletalMeshAsset());
		Test->TestNotNull(TEXT("PIE original Rifle AnimInstance"), Phantom->GetMesh()->GetAnimInstance());
		Test->TestEqual(TEXT("PIE magazine starts at 20"),
			Phantom->GetMagazineComponent()->GetCurrentAmmo(), 20);
		Phantom->SetAIState(EHumanoidEnemyAIState::Aim);
		Phantom->StartLostTargetSearch(Phantom->GetActorLocation() + FVector(200.f, 0.f, 0.f));
		Test->TestNotEqual(TEXT("PIE lost target exits combat even when TestMap has no NavMesh"),
			Phantom->GetAIState(), EHumanoidEnemyAIState::Aim);
		Phantom->SetCombatPhase(2);
		Test->TestTrue(TEXT("PIE phase two cloak"), Phantom->IsCloaked());
		Test->TestEqual(TEXT("PIE projectile collision passes through"),
			Phantom->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_GameTraceChannel1), ECR_Ignore);
		Phantom->Destroy();
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomPIESmokeTest,
	"TheManTest.Enemy.Phantom.PIESmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomPIESmokeTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidatePhantomPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
