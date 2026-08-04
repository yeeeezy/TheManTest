#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Enemy/Components/EnemyMagazineComponent.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Enemy/Humanoid/HumanoidAIController.h"
#include "Enemy/BTTask_UseCombatSkill.h"
#include "Enemy/Humanoid/HumanoidEnemyAnimInstance.h"
#include "Characters/CharacterBase/FPSCharacterBase/FPSCharacterBase.h"
#include "Weapons/_Shared/Components/EquipmentManagerComponent.h"
#include "Weapons/_Shared/EquipmentBase/EquipmentBase.h"
#include "Enemy/Humanoid/Phantom/Phantom.h"
#include "Enemy/Nightmare/FlyingBug2/NightmareFlyingBug.h"
#include "Enemy/Cover/EnemyCoverPoint.h"
#include "Actors/PatrolPoint.h"
#include "Enemy/_Shared/GAS/Abilities/GA_EnemyShoot.h"
#include "Weapons/_Shared/Firearms/Firearm.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "UObject/UObjectIterator.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationCommon.h"
#include "Editor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "ImageUtils.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Animation/AnimBlueprint.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/PoseSnapshot.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimClassInterface.h"
#include "UObject/UnrealType.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "AnimNodes/AnimNode_BlendSpacePlayer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomAnimationOverridesTest,
	"TheManTest.Enemy.Phantom.AnimationOverrides",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomAnimationOverridesTest::RunTest(const FString& Parameters)
{
	UAnimBlueprint* Child = LoadObject<UAnimBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/Logic/ABP_Phantom_OriginalRifle.ABP_Phantom_OriginalRifle"));
	TestNotNull(TEXT("Phantom child AnimBP"), Child);
	if (!Child) return false;

	TSet<FString> OverridePaths;
	for (const FAnimParentNodeAssetOverride& Override : Child->ParentAssetOverrides)
	{
		TestNotNull(TEXT("Every stored Phantom override has an asset"), Override.NewAsset.Get());
		if (Override.NewAsset) OverridePaths.Add(Override.NewAsset->GetPathName());
	}
	auto HasOverride = [&OverridePaths](const TCHAR* Path) { return OverridePaths.Contains(Path); };
	TestTrue(TEXT("Relaxed idle override"), HasOverride(TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Idle_IP.W2_Stand_Relaxed_Idle_IP")));
	TestTrue(TEXT("Patrol directional BlendSpace override"), HasOverride(TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_RelaxedPatrol2D.BS_Phantom_RelaxedPatrol2D")));
	TestTrue(TEXT("Aim directional BlendSpace override"), HasOverride(TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_AimLocomotion.BS_Phantom_AimLocomotion")));
	if (UBlendSpace* Patrol = LoadObject<UBlendSpace>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_RelaxedPatrol2D.BS_Phantom_RelaxedPatrol2D")))
	{
		TestEqual(TEXT("Patrol BlendSpace X axis is Speed"), Patrol->GetBlendParameter(0).DisplayName, FString(TEXT("Speed")));
		TestEqual(TEXT("Patrol BlendSpace Y axis is Direction"), Patrol->GetBlendParameter(1).DisplayName, FString(TEXT("Direction")));
		TArray<FBlendSampleData> RuntimeSamples;
		int32 TriangulationIndex = INDEX_NONE;
		TestTrue(TEXT("Patrol BlendSpace resolves runtime samples at walk speed"),
			Patrol->GetSamplesFromBlendInput(FVector(150.f, 0.f, 0.f), RuntimeSamples, TriangulationIndex, true)
			&& !RuntimeSamples.IsEmpty());
		for (const FBlendSample& Sample : Patrol->GetBlendSamples())
		{
			TestNotNull(TEXT("Every patrol BlendSpace sample has animation data"), Sample.Animation.Get());
		}
	}
	if (UAnimSequence* Idle = LoadObject<UAnimSequence>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Idle_IP.W2_Stand_Relaxed_Idle_IP")))
	{
		const USkeleton* Skeleton = Idle->GetSkeleton();
		const int32 UpperArmIndex = Skeleton ? Skeleton->GetReferenceSkeleton().FindBoneIndex(TEXT("upperarm_l")) : INDEX_NONE;
		FTransform Sample = FTransform::Identity;
		if (UpperArmIndex != INDEX_NONE)
		{
			Idle->GetBoneTransform(Sample, FSkeletonPoseBoneIndex(UpperArmIndex), FAnimExtractContext(0.5), true);
		}
		const FTransform Reference = UpperArmIndex != INDEX_NONE
			? Skeleton->GetReferenceSkeleton().GetRefBonePose()[UpperArmIndex] : FTransform::Identity;
		const float IdleDelta = Sample.GetRotation().AngularDistance(Reference.GetRotation());
		AddInfo(FString::Printf(TEXT("PHANTOM_IDLE_SOURCE pose_delta=%.3f length=%.3f"), IdleDelta, Idle->GetPlayLength()));
		TestTrue(TEXT("Relaxed idle source contains a non-reference upper-arm pose"), IdleDelta > 0.1f);
	}
	for (int32 Index = 1; Index <= 4; ++Index)
	{
		TestTrue(*FString::Printf(TEXT("Patrol scan variant %d override"), Index),
			HasOverride(*FString::Printf(TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Fgt_v%d_IP.W2_Stand_Relaxed_Fgt_v%d_IP"), Index, Index)));
	}

	UBlendSpace* Aim = LoadObject<UBlendSpace>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_AimLocomotion.BS_Phantom_AimLocomotion"));
	TestNotNull(TEXT("Aim 2D BlendSpace"), Aim);
	if (UBlendSpace* AimBlendSpace = Cast<UBlendSpace>(Aim))
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
		TArray<FBlendSampleData> RuntimeSamples;
		int32 TriangulationIndex = INDEX_NONE;
		TestTrue(TEXT("Aim BlendSpace resolves runtime directional samples"),
			AimBlendSpace->GetSamplesFromBlendInput(FVector(45.f, 300.f, 0.f), RuntimeSamples, TriangulationIndex, true)
			&& !RuntimeSamples.IsEmpty());
		AddInfo(FString::Printf(TEXT("PHANTOM_AIM_BLENDSPACE samples=%d"), Samples.Num()));
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomReusableCombatTest,
	"TheManTest.Enemy.Phantom.ReusableCombatModules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomReusableCombatTest::RunTest(const FString& Parameters)
{
	UGA_EnemyShoot* AccuracyProbe = NewObject<UGA_EnemyShoot>();
	const FVector IdealDirection = FVector::ForwardVector;
	float MaxAngularError = 0.f;
	for (int32 Shot = 0; Shot < 12; ++Shot)
	{
		const FVector ShotDirection = AccuracyProbe->CalculateShotDirection(nullptr, IdealDirection);
		MaxAngularError = FMath::Max(MaxAngularError,
			FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(IdealDirection, ShotDirection), -1.f, 1.f))));
	}
	TestTrue(TEXT("Reusable enemy accuracy accumulates sustained-fire bloom"), AccuracyProbe->GetCurrentSpreadDegrees() > 0.f);
	TestTrue(TEXT("Reusable enemy accuracy produces non-perfect shot directions"), MaxAngularError > 0.1f);
	TestTrue(TEXT("Reusable enemy accuracy remains inside its configured maximum cone"), MaxAngularError <= 5.01f);

	AHumanoidAIController* TacticalController = NewObject<AHumanoidAIController>();
	const FVector Target = FVector::ZeroVector;
	const FVector TooClose(300.f, 0.f, 0.f);
	const FVector TooFar(1200.f, 0.f, 0.f);
	const FVector CloseMove = TacticalController->CalculateCombatMoveDestination(TooClose, Target, 1.f);
	const FVector FarMove = TacticalController->CalculateCombatMoveDestination(TooFar, Target, 1.f);
	const FVector LeftMove = TacticalController->CalculateCombatMoveDestination(FVector(700.f, 0.f, 0.f), Target, 1.f);
	const FVector RightMove = TacticalController->CalculateCombatMoveDestination(FVector(700.f, 0.f, 0.f), Target, -1.f);
	TestTrue(TEXT("Too-close tactical move retreats from target"), CloseMove.Size2D() > TooClose.Size2D());
	TestTrue(TEXT("Too-far tactical move closes distance"), FarMove.Size2D() < TooFar.Size2D());
	TestTrue(TEXT("Combat movement adds lateral displacement"), FMath::Abs(LeftMove.Y) > 1.f);
	TestTrue(TEXT("Strafe direction can alternate"), LeftMove.Y * RightMove.Y < 0.f);

	UEnemyMagazineComponent* Magazine = NewObject<UEnemyMagazineComponent>();
	TestEqual(TEXT("Default magazine capacity"), Magazine->GetCapacity(), 20);
	for (int32 Index = 0; Index < 20; ++Index) TestTrue(TEXT("Configured round can be consumed"), Magazine->ConsumeRound());
	TestTrue(TEXT("Magazine reports empty"), Magazine->IsEmpty());
	TestFalse(TEXT("Empty magazine rejects extra shot"), Magazine->ConsumeRound());
	Magazine->Reload();
	TestEqual(TEXT("Reload restores capacity"), Magazine->GetCurrentAmmo(), 20);
	const UBTTask_UseCombatSkill* CombatSkillTask = NewObject<UBTTask_UseCombatSkill>();
	TestTrue(TEXT("Behavior-tree combat skill node enforces at least three seconds between skills"),
		CombatSkillTask && CombatSkillTask->GetPostSkillDelay() >= 3.f);

	TestNotNull(TEXT("Cover mesh asset"), LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Enemy/_Shared/Cover/Mesh/SM_PhantomCover.SM_PhantomCover")));
	TestNotNull(TEXT("Cover actor blueprint"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/_Shared/Cover/BP_EnemyCoverPoint.BP_EnemyCoverPoint")));
	TestNotNull(TEXT("Burst ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomBurst.BGA_PhantomBurst")));
	TestNotNull(TEXT("Suppressive ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomSuppressiveFire.BGA_PhantomSuppressiveFire")));
	TestNotNull(TEXT("Reload ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomReload.BGA_PhantomReload")));
	TestNotNull(TEXT("Cover ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomTakeCover.BGA_PhantomTakeCover")));
	TestNotNull(TEXT("Barrage ability"), LoadObject<UBlueprint>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomAreaBarrage.BGA_PhantomAreaBarrage")));
	UNiagaraSystem* RepairMuzzle = LoadObject<UNiagaraSystem>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_Muzzle.NS_RepairGun_Muzzle"));
	UNiagaraSystem* HumanoidMuzzle = LoadObject<UNiagaraSystem>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/Effects/Muzzle/Systems/NS_HumanoidRifle_Muzzle.NS_HumanoidRifle_Muzzle"));
	TestNotNull(TEXT("RepairGun semantic muzzle system"), RepairMuzzle);
	TestNotNull(TEXT("Humanoid rifle semantic muzzle system"), HumanoidMuzzle);
	if (UClass* RepairClass = LoadClass<AFirearm>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Blueprint/BP_RepairGun.BP_RepairGun_C")))
	{
		const AFirearm* RepairCDO = RepairClass->GetDefaultObject<AFirearm>();
		TestEqual(TEXT("RepairGun inherits the shared energy muzzle default"), RepairCDO->MuzzleEffect.Get(), RepairMuzzle);
		TestTrue(TEXT("RepairGun muzzle effect has a visible production scale"), RepairCDO->MuzzleEffectScale.GetMin() >= 0.8f);
	}
	if (UClass* BurstClass = LoadClass<UGA_EnemyShoot>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomBurst.BGA_PhantomBurst_C")))
	{
		const UGA_EnemyShoot* BurstCDO = BurstClass->GetDefaultObject<UGA_EnemyShoot>();
		TestEqual(TEXT("Humanoid burst inherits the shared physical muzzle default"), BurstCDO->GetMuzzleEffect(), HumanoidMuzzle);
		TestTrue(TEXT("Humanoid muzzle effect has a visible production scale"), BurstCDO->GetMuzzleEffectScale().GetMin() >= 0.7f);
	}

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
			Humanoid->RequestTurn(180.f);
			TestTrue(TEXT("Patrol turn starts pending without requiring an animation notify"), Humanoid->IsPendingTurn());
			Humanoid->Tick(1.f);
			TestFalse(TEXT("Patrol turn completes from reached yaw when animation notify is absent"), Humanoid->IsPendingTurn());
			TestTrue(TEXT("Patrol turn reaches the requested reverse heading"),
				FMath::Abs(FMath::FindDeltaAngleDegrees(Humanoid->GetActorRotation().Yaw, 180.f)) <= 1.f);
			Humanoid->SetAIState(EHumanoidEnemyAIState::Aim);
			Humanoid->StartLostTargetSearch(FVector(500.f, 0.f, 0.f));
			TestEqual(TEXT("Lost target enters SearchRush"), Humanoid->GetAIState(), EHumanoidEnemyAIState::SearchRush);
		}

		UClass* PhantomClass = LoadClass<APhantom>(nullptr,
			TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
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

	UNiagaraSystem* RepairMuzzle = LoadObject<UNiagaraSystem>(nullptr,
		TEXT("/Game/Weapons/RepairGun/Effects/Muzzle/Systems/NS_RepairGun_Muzzle.NS_RepairGun_Muzzle"));
	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (AFPSCharacterBase* Player = Cast<AFPSCharacterBase>(PC->GetPawn())) Player->PrimaryFire();
	}
	bool bRepairMuzzleSpawned = false;
	for (TObjectIterator<UNiagaraComponent> It; It; ++It)
	{
		bRepairMuzzleSpawned |= It->GetWorld() == World && It->GetAsset() == RepairMuzzle;
	}
	Test->TestTrue(TEXT("PIE RepairGun shot spawns its visible muzzle Niagara component"), bRepairMuzzleSpawned);

	FActorSpawnParameters Params;
	Params.ObjectFlags = RF_Transient;
	UClass* PhantomClass = LoadClass<APhantom>(nullptr,
		TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
	APhantom* Phantom = World->SpawnActor<APhantom>(PhantomClass,
		FVector(0.f, 0.f, 150.f), FRotator::ZeroRotator, Params);
	Test->TestNotNull(TEXT("PIE Phantom spawned"), Phantom);
	if (Phantom)
	{
		Test->TestNotNull(TEXT("PIE original Rifle mesh"), Phantom->GetMesh()->GetSkeletalMeshAsset());
		Test->TestNotNull(TEXT("PIE original Rifle AnimInstance"), Phantom->GetMesh()->GetAnimInstance());
		Test->TestEqual(TEXT("PIE magazine starts at 20"),
			Phantom->GetMagazineComponent()->GetCurrentAmmo(), 20);
		Phantom->AimTargetWorld = Phantom->GetActorLocation() + FVector(1000.f, 0.f, 0.f);
		UClass* BurstClass = LoadClass<UGameplayAbility>(nullptr,
			TEXT("/Game/Enemy/Humanoid/Phantom/GAS/GameplayAbility/BGA_PhantomBurst.BGA_PhantomBurst_C"));
		Test->TestTrue(TEXT("PIE Phantom burst ability activates"), BurstClass &&
			Phantom->GetAbilitySystemComponent()->TryActivateAbilityByClass(BurstClass));
		UNiagaraSystem* EnemyMuzzle = LoadObject<UNiagaraSystem>(nullptr,
			TEXT("/Game/Enemy/Humanoid/Phantom/Effects/Muzzle/Systems/NS_HumanoidRifle_Muzzle.NS_HumanoidRifle_Muzzle"));
		bool bEnemyMuzzleSpawned = false;
		for (TObjectIterator<UNiagaraComponent> It; It; ++It)
		{
			bEnemyMuzzleSpawned |= It->GetWorld() == World && It->GetAsset() == EnemyMuzzle;
		}
		Test->TestTrue(TEXT("PIE Phantom shot spawns its visible muzzle Niagara component"), bEnemyMuzzleSpawned);
		Phantom->SetAIState(EHumanoidEnemyAIState::Aim);
		Phantom->StartLostTargetSearch(Phantom->GetActorLocation() + FVector(200.f, 0.f, 0.f));
		Test->TestNotEqual(TEXT("PIE lost target exits combat and begins the NavMesh search flow"),
			Phantom->GetAIState(), EHumanoidEnemyAIState::Aim);
		Phantom->SetCombatPhase(2);
		Test->TestTrue(TEXT("PIE phase two cloak"), Phantom->IsCloaked());
		Test->TestEqual(TEXT("PIE projectile collision passes through"),
			Phantom->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_GameTraceChannel1), ECR_Ignore);
		Phantom->Destroy();
	}
	return true;
}

class FValidateTacticalMovementPIECommand final : public IAutomationLatentCommand
{
public:
	explicit FValidateTacticalMovementPIECommand(FAutomationTestBase* InTest,
		float InStartDistance = 700.f, int32 InRangeMode = 0)
		: Test(InTest), StartDistance(InStartDistance), RangeMode(InRangeMode) {}

	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		if (!Phantom.IsValid())
		{
			APawn* Player = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
			if (!Player) return false;
			TargetLocation = Player->GetActorLocation();
			UClass* PhantomClass = LoadClass<APhantom>(nullptr,
				TEXT("/Game/Enemy/Humanoid/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
			FActorSpawnParameters Params;
			Params.ObjectFlags = RF_Transient;
			APhantom* Spawned = World->SpawnActor<APhantom>(PhantomClass,
				TargetLocation + FVector(StartDistance, 0.f, 0.f), FRotator(0.f, 180.f, 0.f), Params);
			Test->TestNotNull(TEXT("Tactical PIE Phantom spawned"), Spawned);
			if (!Spawned) return true;
			Phantom = Spawned;
			StartLocation = Spawned->GetActorLocation();
			PreviousLocation = StartLocation;
			StartTime = World->GetTimeSeconds();
			Spawned->SetAIState(EHumanoidEnemyAIState::Aim);
			if (AAIController* Controller = Cast<AAIController>(Spawned->GetController())) Controller->SetFocus(Player);
			return false;
		}

		APhantom* Enemy = Phantom.Get();
		AccumulatedTravel += FVector::Dist2D(PreviousLocation, Enemy->GetActorLocation());
		PreviousLocation = Enemy->GetActorLocation();
		const FVector Radial = (Enemy->GetActorLocation() - TargetLocation).GetSafeNormal2D();
		const FVector Tangent(-Radial.Y, Radial.X, 0.f);
		MaxLateralSpeed = FMath::Max(MaxLateralSpeed,
			FMath::Abs(FVector::DotProduct(Enemy->GetVelocity(), Tangent)));
		MinDistance = FMath::Min(MinDistance, FVector::Dist2D(Enemy->GetActorLocation(), TargetLocation));
		MaxDistance = FMath::Max(MaxDistance, FVector::Dist2D(Enemy->GetActorLocation(), TargetLocation));
		if (World->GetTimeSeconds() - StartTime < 1.1f) return false;

		Test->TestTrue(TEXT("Tactical PIE moved for a sustained interval"), AccumulatedTravel > 80.f);
		Test->TestTrue(TEXT("Tactical PIE produced lateral strafe speed"), MaxLateralSpeed > 80.f);
		Test->TestTrue(TEXT("Tactical PIE stayed in a human-like distance envelope"),
			MinDistance > 150.f && MaxDistance < 1400.f);
		const float FinalDistance = FVector::Dist2D(Enemy->GetActorLocation(), TargetLocation);
		if (RangeMode < 0)
		{
			Test->TestTrue(TEXT("Too-close tactical PIE retreats from the player"),
				FinalDistance > StartDistance + 30.f);
		}
		else if (RangeMode > 0)
		{
			Test->TestTrue(TEXT("Too-far tactical PIE closes distance diagonally"),
				FinalDistance < StartDistance - 30.f);
		}
		Test->TestTrue(TEXT("Tactical PIE remained in Aim"), Enemy->GetAIState() == EHumanoidEnemyAIState::Aim);
		Test->TestTrue(TEXT("Tactical PIE kept aiming"), Enemy->bIsAiming);
		Test->TestNotNull(TEXT("Tactical PIE uses humanoid AnimInstance"),
			Cast<UHumanoidEnemyAnimInstance>(Enemy->GetMesh()->GetAnimInstance()));
		Test->AddInfo(FString::Printf(TEXT("TACTICAL_PIE travel=%.1f displacement=%.1f lateral_max=%.1f distance=[%.1f,%.1f]"),
			AccumulatedTravel, FVector::Dist2D(StartLocation, Enemy->GetActorLocation()),
			MaxLateralSpeed, MinDistance, MaxDistance));
		Enemy->Destroy();
		return true;
	}

private:
	FAutomationTestBase* Test = nullptr;
	TWeakObjectPtr<APhantom> Phantom;
	FVector StartLocation = FVector::ZeroVector;
	FVector PreviousLocation = FVector::ZeroVector;
	FVector TargetLocation = FVector::ZeroVector;
	float StartTime = 0.f;
	float MaxLateralSpeed = 0.f;
	float MinDistance = TNumericLimits<float>::Max();
	float MaxDistance = 0.f;
	float AccumulatedTravel = 0.f;
	float StartDistance = 700.f;
	int32 RangeMode = 0;
};

class FAuditPlacedPhantomAnimationPIECommand final : public IAutomationLatentCommand
{
public:
	explicit FAuditPlacedPhantomAnimationPIECommand(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		if (StartTime <= 0.f)
		{
			StartTime = World->GetTimeSeconds();
			return false;
		}
		if (World->GetTimeSeconds() - StartTime < 1.f) return false;

		int32 PlacedCount = 0;
		for (TActorIterator<APhantom> It(World); It; ++It)
		{
			APhantom* Phantom = *It;
			if (!IsValid(Phantom)) continue;
			++PlacedCount;
			USkeletalMeshComponent* Mesh = Phantom->GetMesh();
			UHumanoidEnemyAnimInstance* Anim = Mesh ? Cast<UHumanoidEnemyAnimInstance>(Mesh->GetAnimInstance()) : nullptr;
			Test->TestTrue(TEXT("Placed Phantom uses a valid humanoid AnimInstance"), IsValid(Anim));
			if (!Mesh || !IsValid(Anim)) continue;

			const FString AnimClassPath = Anim->GetClass()->GetPathName();
			const FString MeshPath = GetPathNameSafe(Mesh->GetSkeletalMeshAsset());
			const FString SkeletonPath = Mesh->GetSkeletalMeshAsset()
				? GetPathNameSafe(Mesh->GetSkeletalMeshAsset()->GetSkeleton()) : TEXT("None");
			FPoseSnapshot Snapshot;
			Mesh->SnapshotPose(Snapshot);
			FString MachineDebug = TEXT("none");
			FString PlayerDebug;
			if (const IAnimClassInterface* RuntimeClass = IAnimClassInterface::GetFromClass(Anim->GetClass()))
			{
				for (const FStructProperty* NodeProperty : RuntimeClass->GetAnimNodeProperties())
				{
					void* NodeMemory = NodeProperty->ContainerPtrToValuePtr<void>(Anim);
					if (NodeProperty->Struct == FAnimNode_StateMachine::StaticStruct())
					{
						const FAnimNode_StateMachine* Machine = static_cast<const FAnimNode_StateMachine*>(NodeMemory);
						MachineDebug = FString::Printf(TEXT("%d@%.3f"), Machine->GetCurrentState(), Machine->GetCurrentStateElapsedTime());
					}
					else if (NodeProperty->Struct->IsChildOf(FAnimNode_AssetPlayerBase::StaticStruct()))
					{
						const FAnimNode_AssetPlayerBase* Player = static_cast<const FAnimNode_AssetPlayerBase*>(NodeMemory);
						if (Player->GetCachedBlendWeight() > KINDA_SMALL_NUMBER)
						{
							PlayerDebug += FString::Printf(TEXT("%s[w=%.2f,t=%.3f] "), *NodeProperty->GetName(),
								Player->GetCachedBlendWeight(), Player->GetAccumulatedTime());
							PlayerDebug += FString::Printf(TEXT("group=%s role=%d method=%d asset=%s "),
								*Player->GetGroupName().ToString(), static_cast<int32>(Player->GetGroupRole()),
								static_cast<int32>(Player->GetGroupMethod()), *GetPathNameSafe(Player->GetAnimAsset()));
							if (NodeProperty->Struct->IsChildOf(FAnimNode_BlendSpacePlayerBase::StaticStruct()))
							{
								const FAnimNode_BlendSpacePlayerBase* BlendPlayer = static_cast<const FAnimNode_BlendSpacePlayerBase*>(NodeMemory);
								PlayerDebug += FString::Printf(TEXT("pos=%s asset=%s playrate=%.2f loop=%d start=%.2f "),
									*BlendPlayer->GetPosition().ToString(), *GetPathNameSafe(BlendPlayer->GetBlendSpace()),
									BlendPlayer->GetPlayRate(), BlendPlayer->IsLooping(), BlendPlayer->GetStartPosition());
							}
						}
					}
				}
			}
			float MaxPoseAngularDelta = 0.f;
			for (const FName BoneName : {FName(TEXT("upperarm_l")), FName(TEXT("upperarm_r")), FName(TEXT("spine_03"))})
			{
				const int32 BoneIndex = Mesh->GetBoneIndex(BoneName);
				if (BoneIndex == INDEX_NONE) continue;
				const int32 SnapshotIndex = Snapshot.BoneNames.IndexOfByKey(BoneName);
				if (!Snapshot.LocalTransforms.IsValidIndex(SnapshotIndex)) continue;
				const FTransform& Current = Snapshot.LocalTransforms[SnapshotIndex];
				const FTransform& Reference = Mesh->GetSkeletalMeshAsset()->GetRefSkeleton().GetRefBonePose()[BoneIndex];
				MaxPoseAngularDelta = FMath::Max(MaxPoseAngularDelta,
					Current.GetRotation().AngularDistance(Reference.GetRotation()));
			}

			Test->AddInfo(FString::Printf(
				TEXT("PLACED_PHANTOM actor=%s mesh=%s skeleton=%s anim=%s mode=%d ai=%d pose_delta=%.3f velocity=%.1f tick=%d pause=%d rate=%.2f recent=%d visibility_tick=%d machine=%s players=%s"),
				*Phantom->GetName(), *MeshPath, *SkeletonPath, *AnimClassPath,
				static_cast<int32>(Mesh->GetAnimationMode()), static_cast<int32>(Phantom->GetAIState()),
				MaxPoseAngularDelta, Phantom->GetVelocity().Size2D(), Mesh->IsComponentTickEnabled(), Mesh->bPauseAnims,
				Mesh->GlobalAnimRateScale, Mesh->WasRecentlyRendered(), static_cast<int32>(Mesh->VisibilityBasedAnimTickOption),
				*MachineDebug, *PlayerDebug));
			Test->TestTrue(TEXT("Placed Phantom uses original Rifle child AnimBP"),
				AnimClassPath.Contains(TEXT("ABP_Phantom_OriginalRifle_C")));
			Test->TestEqual(TEXT("Placed Phantom mesh runs Animation Blueprint mode"),
				Mesh->GetAnimationMode(), EAnimationMode::AnimationBlueprint);
			Test->TestTrue(TEXT("Placed Phantom pose is not reference pose"), MaxPoseAngularDelta > 0.1f);
		}
		Test->TestTrue(TEXT("TestMap contains a placed Phantom"), PlacedCount > 0);
		return true;
	}
private:
	FAutomationTestBase* Test = nullptr;
	float StartTime = 0.f;
};

class FValidateTwoPointPatrolLoopPIECommand final : public IAutomationLatentCommand
{
public:
	explicit FValidateTwoPointPatrolLoopPIECommand(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		if (!Enemy.IsValid())
		{
			UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
			APawn* Player = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
			if (!Nav || !Player) return false;
			FNavLocation PointALocation;
			FNavLocation PointBLocation;
			const FVector Center = Player->GetActorLocation() + FVector(500.f, 500.f, 0.f);
			if (!Nav->ProjectPointToNavigation(Center + FVector(0.f, -250.f, 0.f), PointALocation)
				|| !Nav->ProjectPointToNavigation(Center + FVector(0.f, 250.f, 0.f), PointBLocation)
				|| FVector::Dist2D(PointALocation.Location, PointBLocation.Location) < 300.f)
			{
				Test->AddError(TEXT("Two-point patrol test could not project both points to the existing NavMesh"));
				return true;
			}
			FActorSpawnParameters Params;
			Params.ObjectFlags = RF_Transient;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			PointA = World->SpawnActor<APatrolPoint>(APatrolPoint::StaticClass(), PointALocation.Location, FRotator::ZeroRotator, Params);
			PointB = World->SpawnActor<APatrolPoint>(APatrolPoint::StaticClass(), PointBLocation.Location, FRotator::ZeroRotator, Params);
			AHumanoidEnemy* Spawned = World->SpawnActor<AHumanoidEnemy>(AHumanoidEnemy::StaticClass(), PointALocation.Location,
				FRotator::ZeroRotator, Params);
			Test->TestNotNull(TEXT("Two-point patrol humanoid probe spawned"), Spawned);
			if (!Spawned || !PointA.IsValid() || !PointB.IsValid()) return true;
			Enemy = Spawned;
			PreviousLocation = Spawned->GetActorLocation();
			if (AHumanoidAIController* Controller = Cast<AHumanoidAIController>(Spawned->GetController()))
			{
				Controller->UnPossess();
				Controller->Destroy();
			}
			AAIController* PatrolOnlyController = World->SpawnActor<AAIController>(AAIController::StaticClass(),
				Spawned->GetActorLocation(), FRotator::ZeroRotator, Params);
			Test->TestNotNull(TEXT("Two-point patrol uses an isolated navigation-only controller"), PatrolOnlyController);
			if (!PatrolOnlyController) return true;
			PatrolOnlyController->Possess(Spawned);
			Spawned->SetAIState(EHumanoidEnemyAIState::Patrol);
			TArray<APatrolPoint*> Points{PointA.Get(), PointB.Get()};
			Spawned->ConfigurePatrolPoints(Points, true);
			StartTime = World->GetTimeSeconds();
			return false;
		}

		Travel += FVector::Dist2D(PreviousLocation, Enemy->GetActorLocation());
		PreviousLocation = Enemy->GetActorLocation();
		const int32 Arrivals = Enemy->GetPatrolArrivalCount();
		if ((Arrivals < 5 || Travel < 1200.f) && World->GetTimeSeconds() - StartTime < 25.f) return false;
		Test->TestTrue(TEXT("Two-point NavMesh patrol completes more than two full reversals"), Arrivals >= 5);
		Test->TestTrue(TEXT("Two-point NavMesh patrol physically traverses the route repeatedly"), Travel >= 1200.f);
		Test->TestFalse(TEXT("Two-point NavMesh patrol is not stuck pending a turn"), Enemy->IsPendingTurn());
		Test->AddInfo(FString::Printf(TEXT("PATROL_LOOP arrivals=%d travel=%.1f elapsed=%.2f"),
			Arrivals, Travel, World->GetTimeSeconds() - StartTime));
		Test->AddInfo(FString::Printf(TEXT("PATROL_LOOP_POS enemy=%s A=%s B=%s distA=%.1f distB=%.1f velocity=%.1f"),
			*Enemy->GetActorLocation().ToString(), *PointA->GetActorLocation().ToString(), *PointB->GetActorLocation().ToString(),
			FVector::Dist2D(Enemy->GetActorLocation(), PointA->GetActorLocation()),
			FVector::Dist2D(Enemy->GetActorLocation(), PointB->GetActorLocation()), Enemy->GetVelocity().Size2D()));
		Enemy->Destroy();
		if (PointA.IsValid()) PointA->Destroy();
		if (PointB.IsValid()) PointB->Destroy();
		return true;
	}
private:
	FAutomationTestBase* Test = nullptr;
	TWeakObjectPtr<AHumanoidEnemy> Enemy;
	TWeakObjectPtr<APatrolPoint> PointA;
	TWeakObjectPtr<APatrolPoint> PointB;
	float StartTime = 0.f;
	float Travel = 0.f;
	FVector PreviousLocation = FVector::ZeroVector;
};

class FAuditPlacedPatrolProgressPIECommand final : public IAutomationLatentCommand
{
public:
	explicit FAuditPlacedPatrolProgressPIECommand(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		if (!Enemy.IsValid())
		{
			for (TActorIterator<APhantom> It(World); It; ++It)
			{
				Enemy = *It;
				break;
			}
			Test->TestTrue(TEXT("Placed-patrol audit found the TestMap Phantom"), Enemy.IsValid());
			if (!Enemy.IsValid()) return true;
			Test->AddInfo(FString::Printf(TEXT("PLACED_PATROL_ACTOR path=%s name=%s start=%s mesh_rel=%s"),
				*Enemy->GetPathName(), *Enemy->GetName(), *Enemy->GetActorLocation().ToString(),
				*Enemy->GetMesh()->GetRelativeLocation().ToString()));
			Test->TestEqual(TEXT("TestMap Phantom keeps its four authored patrol points"), Enemy->GetPatrolPointCount(), 4);
			AHumanoidAIController* RealController = Cast<AHumanoidAIController>(Enemy->GetController());
			Test->TestNotNull(TEXT("Placed Phantom keeps its production humanoid AI controller"), RealController);
			if (!RealController) return true;
			// Deliberately keep production perception enabled: this regression must match the
			// normal Play flow rather than an isolated patrol-only setup.
			Enemy->SetAIState(EHumanoidEnemyAIState::Patrol);
			StartTime = World->GetTimeSeconds();
			PreviousLocation = Enemy->GetActorLocation();
			LastArrivalCount = Enemy->GetPatrolArrivalCount();
			CaptureOverview(World, TEXT("PatrolPIE_00_Start.png"));
			return false;
		}

		Travel += FVector::Dist2D(PreviousLocation, Enemy->GetActorLocation());
		PreviousLocation = Enemy->GetActorLocation();
		const int32 Arrivals = Enemy->GetPatrolArrivalCount();
		if (Arrivals != LastArrivalCount)
		{
			Test->AddInfo(FString::Printf(TEXT("PLACED_PATROL arrival=%d next_index=%d elapsed=%.2f loc=%s target=%s"),
				Arrivals, Enemy->GetCurrentPatrolIndex(), World->GetTimeSeconds() - StartTime,
				*Enemy->GetActorLocation().ToString(), *Enemy->GetCurrentPatrolTargetLocation().ToString()));
			if (Arrivals == 2) CaptureOverview(World, TEXT("PatrolPIE_02_SecondPoint.png"));
			if (Arrivals == 3) CaptureOverview(World, TEXT("PatrolPIE_03_ThirdPoint.png"));
			LastArrivalCount = Arrivals;
		}
		if (Arrivals < 3 && World->GetTimeSeconds() - StartTime < 40.f) return false;

		Test->TestTrue(TEXT("Placed TestMap Phantom physically advances from patrol point two to point three"), Arrivals >= 3);
		Test->AddInfo(FString::Printf(
			TEXT("PLACED_PATROL_FINAL arrivals=%d index=%d state=%d pending=%d scanning=%d wait=%.2f travel=%.1f velocity=%.1f loc=%s target=%s"),
			Arrivals, Enemy->GetCurrentPatrolIndex(), static_cast<int32>(Enemy->GetAIState()), Enemy->IsPendingTurn(),
			Enemy->IsPatrolScanning(), Enemy->GetPatrolWaitRemaining(), Travel, Enemy->GetVelocity().Size2D(),
			*Enemy->GetActorLocation().ToString(), *Enemy->GetCurrentPatrolTargetLocation().ToString()));
		return true;
	}
private:
	static bool CaptureOverview(UWorld* World, const FString& FileName)
	{
		if (!World) return false;
		UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
		RenderTarget->RenderTargetFormat = RTF_RGBA8;
		RenderTarget->InitAutoFormat(1280, 720);
		RenderTarget->UpdateResourceImmediate(true);

		USceneCaptureComponent2D* Capture = NewObject<USceneCaptureComponent2D>(World->GetWorldSettings());
		Capture->RegisterComponentWithWorld(World);
		Capture->TextureTarget = RenderTarget;
		Capture->ProjectionType = ECameraProjectionMode::Perspective;
		Capture->FOVAngle = 90.f;
		Capture->SetWorldLocation(FVector(-3500.f, 400.f, 1000.f));
		Capture->SetWorldRotation(FRotator(-22.f, 0.f, 0.f));
		Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
		Capture->CaptureScene();

		TArray<FColor> Pixels;
		FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
		const bool bRead = RenderTargetResource && RenderTargetResource->ReadPixels(Pixels);
		TArray64<uint8> PNGData;
		if (bRead) FImageUtils::PNGCompressImageArray(1280, 720, Pixels, PNGData);
		const FString Path = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots"), FileName);
		const bool bSaved = bRead && !PNGData.IsEmpty() && FFileHelper::SaveArrayToFile(PNGData, *Path);
		Capture->DestroyComponent();
		return bSaved;
	}

	FAutomationTestBase* Test = nullptr;
	TWeakObjectPtr<APhantom> Enemy;
	FVector PreviousLocation = FVector::ZeroVector;
	float StartTime = 0.f;
	float Travel = 0.f;
	int32 LastArrivalCount = 0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomPIESmokeTest,
	"TheManTest.Enemy.Phantom.PIESmoke",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomPIESmokeTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidatePhantomPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateTacticalMovementPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlacedPhantomAnimationPIETest,
	"TheManTest.Enemy.Phantom.PIEPlacedAnimation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlacedPhantomAnimationPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FAuditPlacedPhantomAnimationPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomPatrolLoopPIETest,
	"TheManTest.Enemy.Phantom.PIEPatrolLoop",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomPatrolLoopPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateTwoPointPatrolLoopPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlacedPhantomPatrolProgressPIETest,
	"TheManTest.Enemy.Phantom.PIEPlacedPatrolProgress",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlacedPhantomPatrolProgressPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FAuditPlacedPatrolProgressPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomTacticalRetreatPIETest,
	"TheManTest.Enemy.Phantom.PIETacticalRetreat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomTacticalRetreatPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateTacticalMovementPIECommand(this, 300.f, -1));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhantomTacticalApproachPIETest,
	"TheManTest.Enemy.Phantom.PIETacticalApproach",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPhantomTacticalApproachPIETest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidateTacticalMovementPIECommand(this, 1200.f, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FPlayerFramingScreenshotCommand);

static bool SaveSceneCapture(UWorld* World, AActor* Owner, const FVector& CameraLocation,
	const FVector& TargetLocation, const FString& FileName)
{
	if (!World || !Owner) return false;
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->InitAutoFormat(1920, 1080);
	RenderTarget->UpdateResourceImmediate(true);
	USceneCaptureComponent2D* Capture = NewObject<USceneCaptureComponent2D>(Owner);
	Capture->RegisterComponentWithWorld(World);
	Capture->SetWorldLocationAndRotation(CameraLocation, (TargetLocation - CameraLocation).Rotation());
	Capture->FOVAngle = 65.f;
	Capture->CaptureSource = SCS_FinalColorLDR;
	Capture->TextureTarget = RenderTarget;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;
	UPointLightComponent* EvidenceLight = NewObject<UPointLightComponent>(Owner);
	EvidenceLight->RegisterComponentWithWorld(World);
	EvidenceLight->SetWorldLocation(CameraLocation + FVector(0.f, 0.f, 120.f));
	EvidenceLight->SetIntensity(45000.f);
	EvidenceLight->SetAttenuationRadius(1200.f);
	Capture->CaptureScene();
	TArray<FColor> Pixels;
	if (!RenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(Pixels)) return false;
	TArray64<uint8> PNGData;
	FImageUtils::PNGCompressImageArray(1920, 1080, Pixels, PNGData);
	const bool bSaved = FFileHelper::SaveArrayToFile(PNGData,
		*FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Screenshots/WindowsEditor"), FileName));
	EvidenceLight->DestroyComponent();
	Capture->DestroyComponent();
	return bSaved;
}

bool FPlayerFramingScreenshotCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	AFPSCharacterBase* Player = World ? Cast<AFPSCharacterBase>(World->GetFirstPlayerController()->GetPawn()) : nullptr;
	if (!Player || !Player->GetHeadCamera()) return false;
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->InitAutoFormat(1920, 1080);
	RenderTarget->UpdateResourceImmediate(true);
	USceneCaptureComponent2D* Capture = NewObject<USceneCaptureComponent2D>(Player);
	Capture->RegisterComponentWithWorld(World);
	Capture->AttachToComponent(Player->GetHeadCamera(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Capture->FOVAngle = Player->GetHeadCamera()->FieldOfView;
	Capture->CaptureSource = SCS_FinalColorLDR;
	Capture->TextureTarget = RenderTarget;
	Capture->bCaptureEveryFrame = false;
	Capture->bCaptureOnMovement = false;
	Capture->CaptureScene();
	TArray<FColor> Pixels;
	if (!RenderTarget->GameThread_GetRenderTargetResource()->ReadPixels(Pixels)) return false;
	const FIntPoint Size(1920, 1080);
	TArray64<uint8> PNGData;
	FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, PNGData);
	const FString ScreenshotPath = FPaths::Combine(FPaths::ProjectSavedDir(),
		TEXT("Screenshots/PlayerFramingCurrent.png"));
	const bool bSaved = FFileHelper::SaveArrayToFile(PNGData, *ScreenshotPath);
	Capture->DestroyComponent();
	return bSaved;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FShadowUpperBodyEvidenceCommand, FAutomationTestBase*, Test);
bool FShadowUpperBodyEvidenceCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = PC ? Cast<AFPSCharacterBase>(PC->GetPawn()) : nullptr;
	if (!Player || !Player->GetShadowBodyMesh() || !Player->GetShadowUpperBodyMesh()) return false;
	Test->TestEqual(TEXT("Deprecated split upper-body shadow has no mesh"),
		Player->GetShadowUpperBodyMesh()->GetSkeletalMeshAsset(), static_cast<USkeletalMesh*>(nullptr));
	Test->TestFalse(TEXT("Deprecated split upper-body shadow cannot cast"),
		Player->GetShadowUpperBodyMesh()->CastShadow);
	Test->TestEqual(TEXT("Complete shadow body follows the body animation source"),
		Player->GetShadowBodyMesh()->GetBaseComponent(),
		static_cast<const USkinnedMeshComponent*>(Player->GetMesh()));
	Test->TestTrue(TEXT("Complete shadow body is hidden geometry that still casts"),
		Player->GetShadowBodyMesh()->bHiddenInGame && Player->GetShadowBodyMesh()->bCastHiddenShadow);
	const FVector Target = Player->GetActorLocation() + FVector(0.f, 0.f, 80.f);
	Test->TestTrue(TEXT("Shadow upper-body evidence screenshot saved"), SaveSceneCapture(
		World, Player, Target + FVector(-260.f, 260.f, 260.f), Target,
		TEXT("TMT_ShadowUpperBody_Runtime.png")));
	return true;
}

class FNightmareCrawlEvidenceCommand final : public IAutomationLatentCommand
{
public:
	explicit FNightmareCrawlEvidenceCommand(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		ANightmareFlyingBug* Bug = SpawnedBug.Get();
		if (!Bug && !bStarted)
		{
			UClass* BugClass = LoadClass<ANightmareFlyingBug>(nullptr,
				TEXT("/Game/Enemy/Nightmare/FlyingBug2/Blueprint/BP_NightmareFlyingBug2.BP_NightmareFlyingBug2_C"));
			FVector SpawnLocation(10000.f, -10000.f, 300.f);
			FVector RouteEnd(11800.f, -10000.f, 350.f);
			for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
			{
				if (It->GetActorLabel() == TEXT("Validation_FlyingBugTerrain_Start"))
				{
					SpawnLocation = It->GetActorLocation() + FVector(0.f, 0.f, 180.f);
				}
				else if (It->GetActorLabel() == TEXT("Validation_FlyingBugTerrain_End"))
				{
					RouteEnd = It->GetActorLocation() + FVector(0.f, 0.f, 100.f);
				}
			}
			FHitResult GroundHit;
			if (World->LineTraceSingleByChannel(GroundHit, SpawnLocation + FVector(0.f, 0.f, 600.f),
				SpawnLocation - FVector(0.f, 0.f, 1200.f), ECC_Visibility))
			{
				SpawnLocation.Z = GroundHit.ImpactPoint.Z + 240.f;
			}
			Bug = BugClass ? World->SpawnActor<ANightmareFlyingBug>(BugClass, SpawnLocation, FRotator::ZeroRotator) : nullptr;
			SpawnedBug = Bug;
			if (Bug) { Bug->SetRoamDestinationForTesting(RouteEnd); }
		}
		if (!Bug) return false;
		if (!bStarted)
		{
			bStarted = true;
			StartLocation = Bug->GetActorLocation();
			StartTime = World->GetTimeSeconds();
			return false;
		}
		if (World->GetTimeSeconds() - StartTime < 18.f) return false;
		Test->TestEqual(TEXT("Nightmare uses walking movement"), Bug->GetCharacterMovement()->MovementMode, MOVE_Walking);
		Test->AddInfo(FString::Printf(TEXT("Rugged route planar distance: %.1f cm"),
			FVector::Dist2D(StartLocation, Bug->GetActorLocation())));
		Test->TestTrue(TEXT("Nightmare moved while crawling"),
			FVector::Dist2D(StartLocation, Bug->GetActorLocation()) > 1200.f);
		Test->TestTrue(TEXT("Nightmare mesh remains back-up across rugged terrain"),
			FVector::DotProduct(Bug->GetMesh()->GetUpVector(), FVector::UpVector) > 0.75f);
		const FVector Target = Bug->GetActorLocation() + FVector(0.f, 0.f, 35.f);
		Test->TestTrue(TEXT("Nightmare crawl evidence screenshot saved"), SaveSceneCapture(
			World, Bug, Target + FVector(-520.f, 360.f, 260.f), Target,
			TEXT("TMT_NightmareLocomotor_Crawl.png")));
		return true;
	}
private:
	FAutomationTestBase* Test = nullptr;
	bool bStarted = false;
	float StartTime = 0.f;
	FVector StartLocation = FVector::ZeroVector;
	TWeakObjectPtr<ANightmareFlyingBug> SpawnedBug;
};

class FNightmareSlopeEvidenceCommand final : public IAutomationLatentCommand
{
public:
	explicit FNightmareSlopeEvidenceCommand(FAutomationTestBase* InTest) : Test(InTest) {}
	virtual bool Update() override
	{
		UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
		if (!World) return false;
		if (!bStarted)
		{
			APlayerController* PC = World->GetFirstPlayerController();
			const FVector Base = PC && PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector;
			RampRotation = FRotator(-18.f, 0.f, 0.f);
			Ramp = World->SpawnActor<AStaticMeshActor>(Base + FVector(1100.f, -600.f, 130.f), RampRotation);
			UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			if (!Ramp.IsValid() || !Cube) return false;
			Ramp->GetStaticMeshComponent()->SetStaticMesh(Cube);
			Ramp->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
			Ramp->GetStaticMeshComponent()->SetWorldScale3D(FVector(6.f, 4.f, 0.2f));
			Ramp->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			const FVector Forward = RampRotation.RotateVector(FVector::ForwardVector);
			const FVector Normal = RampRotation.RotateVector(FVector::UpVector);
			const FVector Start = Ramp->GetActorLocation() - Forward * 180.f + Normal * 125.f;
			UClass* BugClass = LoadClass<ANightmareFlyingBug>(nullptr,
				TEXT("/Game/Enemy/Nightmare/FlyingBug2/Blueprint/BP_NightmareFlyingBug2.BP_NightmareFlyingBug2_C"));
			Bug = BugClass ? World->SpawnActor<ANightmareFlyingBug>(BugClass, Start, RampRotation) : nullptr;
			if (!Bug.IsValid()) return false;
			Bug->SetRoamDestinationForTesting(Start + Forward * 360.f);
			StartLocation = Bug->GetActorLocation();
			StartTime = World->GetTimeSeconds();
			bStarted = true;
			return false;
		}
		if (!Bug.IsValid() || World->GetTimeSeconds() - StartTime < 2.f) return false;
		const FVector RampNormal = RampRotation.RotateVector(FVector::UpVector);
		Test->TestEqual(TEXT("Nightmare remains walking on slope"), Bug->GetCharacterMovement()->MovementMode, MOVE_Walking);
		Test->TestTrue(TEXT("Nightmare crawls along slope"), FVector::Dist(StartLocation, Bug->GetActorLocation()) > 20.f);
		Test->TestTrue(TEXT("Nightmare collision actor remains upright on slope"),
			FVector::DotProduct(Bug->GetActorUpVector(), FVector::UpVector) > 0.99f);
		Test->TestTrue(TEXT("Nightmare visual mesh aligns to slope"),
			FVector::DotProduct(Bug->GetMesh()->GetUpVector(), RampNormal) > 0.9f);
		Test->TestTrue(TEXT("Nightmare mesh stays upright on slope"),
			FVector::DotProduct(Bug->GetMesh()->GetUpVector(), RampNormal) > 0.9f);
		const FVector Target = Bug->GetActorLocation() + FVector(0.f, 0.f, 40.f);
		Test->TestTrue(TEXT("Nightmare slope evidence screenshot saved"), SaveSceneCapture(
			World, Bug.Get(), Target + FVector(-520.f, 420.f, 280.f), Target,
			TEXT("TMT_NightmareLocomotor_Slope.png")));
		return true;
	}
private:
	FAutomationTestBase* Test = nullptr;
	bool bStarted = false;
	float StartTime = 0.f;
	FVector StartLocation = FVector::ZeroVector;
	FRotator RampRotation = FRotator::ZeroRotator;
	TWeakObjectPtr<AStaticMeshActor> Ramp;
	TWeakObjectPtr<ANightmareFlyingBug> Bug;
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FValidatePlayerViewmodelPIECommand, FAutomationTestBase*, Test);
bool FValidatePlayerViewmodelPIECommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = PC ? Cast<AFPSCharacterBase>(PC->GetPawn()) : nullptr;
	if (!Player || !Player->GetHeadCamera() || !Player->GetViewmodelRoot() || !Player->GetArmsMesh()) return false;

	Test->TestTrue(TEXT("Gameplay camera uses the original VFXPack 77 degree baseline FOV"),
		FMath::IsNearlyEqual(Player->GetHeadCamera()->FieldOfView, 77.f));
	Test->TestEqual(TEXT("ViewmodelRoot is attached directly to HeadCamera"),
		Player->GetViewmodelRoot()->GetAttachParent(), static_cast<USceneComponent*>(Player->GetHeadCamera()));
	Test->TestEqual(TEXT("ArmsViewMesh is attached to ViewmodelRoot"),
		Player->GetArmsMesh()->GetAttachParent(), Player->GetViewmodelRoot());
	Test->TestTrue(TEXT("BodyRotator-equivalent pivot remains at the camera origin"),
		Player->GetViewmodelRoot()->GetRelativeLocation().Equals(FVector::ZeroVector, 0.01f));
	Test->TestTrue(TEXT("Viewmodel root matches VFXPack BodyRotator identity rotation"),
		Player->GetViewmodelRoot()->GetRelativeRotation().Equals(FRotator::ZeroRotator, 0.01f));
	Test->TestTrue(TEXT("Arms rotation matches VFXPack SK_ArmMesh"),
		Player->GetArmsMesh()->GetRelativeRotation().Equals(FRotator(-3.f, -15.f, -1.f), 0.01f));
	Test->TestTrue(TEXT("Arms location matches VFXPack SK_ArmMesh beneath BodyRotator"),
		Player->GetArmsMesh()->GetRelativeLocation().Equals(FVector(-18.107912f, 18.852108f, -150.00795f), 0.01f));
	Test->TestNotNull(TEXT("Body uses an animation instance"), Player->GetMesh()->GetAnimInstance());
	Test->TestNotNull(TEXT("First-person arms use an animation instance"), Player->GetArmsMesh()->GetAnimInstance());
	if (Player->GetMesh()->GetAnimInstance() && Player->GetArmsMesh()->GetAnimInstance())
	{
		Test->TestEqual(TEXT("First- and third-person meshes use the same VFXPack AnimBP class"),
			Player->GetMesh()->GetAnimInstance()->GetClass(), Player->GetArmsMesh()->GetAnimInstance()->GetClass());
	}
	Test->TestEqual(TEXT("Shadow body follows the third-person animation source"),
		Player->GetShadowBodyMesh()->GetBaseComponent(), static_cast<const USkinnedMeshComponent*>(Player->GetMesh()));

	UEquipmentManagerComponent* EquipmentManager = Player->GetEquipmentManager();
	AEquipmentBase* Equipment = EquipmentManager ? EquipmentManager->GetCurrentEquipment() : nullptr;
	Test->TestNotNull(TEXT("Player has initial equipment"), Equipment);
	if (Equipment)
	{
		const FName VFXPackGripSocket(TEXT("GripPoint"));
		Test->TestEqual(TEXT("VFXPack arms expose the original GripPoint socket"),
			Player->GetArmsMesh()->DoesSocketExist(VFXPackGripSocket), true);
		Test->TestEqual(TEXT("Current equipment declares the original VFXPack GripPoint socket"),
			Equipment->GetEquipSocketName(), VFXPackGripSocket);
		Test->TestEqual(TEXT("Current equipment follows ArmsViewMesh socket"),
			Equipment->GetRootComponent()->GetAttachParent(), static_cast<USceneComponent*>(Player->GetArmsMesh()));
		Test->TestEqual(TEXT("Current equipment uses declared equip socket"),
			Equipment->GetRootComponent()->GetAttachSocketName(), Equipment->GetEquipSocketName());
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND(FStabilizePlayerViewmodelCommand);
bool FStabilizePlayerViewmodelCommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	AFPSCharacterBase* Player = World && World->GetFirstPlayerController()
		? Cast<AFPSCharacterBase>(World->GetFirstPlayerController()->GetPawn()) : nullptr;
	if (!Player || !Player->GetArmsMesh() || !Player->GetArmsMesh()->GetAnimInstance()) return false;
	Player->GetArmsMesh()->GetAnimInstance()->Montage_Stop(0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlayerFramingCaptureTest,
	"TheManTest.Player.Viewmodel.FramingCapture",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPlayerFramingCaptureTest::RunTest(const FString& Parameters)
{
	if (GEngine) GEngine->Exec(nullptr, TEXT("r.MotionBlurQuality 0"));
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.8f));
	ADD_LATENT_AUTOMATION_COMMAND(FValidatePlayerViewmodelPIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FStabilizePlayerViewmodelCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FPlayerFramingScreenshotCommand());
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShadowUpperBodyEvidenceTest,
	"TheManTest.Player.Shadow.UpperBodyEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FShadowUpperBodyEvidenceTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.2f));
	ADD_LATENT_AUTOMATION_COMMAND(FShadowUpperBodyEvidenceCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNightmareLocomotorCrawlEvidenceTest,
	"TheManTest.Enemy.Nightmare.LocomotorCrawlEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNightmareLocomotorCrawlEvidenceTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FNightmareCrawlEvidenceCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNightmareLocomotorSlopeEvidenceTest,
	"TheManTest.Enemy.Nightmare.LocomotorSlopeEvidence",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FNightmareLocomotorSlopeEvidenceTest::RunTest(const FString& Parameters)
{
	AutomationOpenMap(TEXT("/Game/Maps/TestMap"));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FNightmareSlopeEvidenceCommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

#endif
