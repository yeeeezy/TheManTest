#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Characters/Enemy/Components/EnemyMagazineComponent.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemy.h"
#include "Characters/Enemy/Humanoid/HumanoidAIController.h"
#include "Characters/Enemy/Humanoid/HumanoidEnemyAnimInstance.h"
#include "Characters/FPSCharacterBase/FPSCharacterBase.h"
#include "Characters/Components/EquipmentManagerComponent.h"
#include "Equipment/EquipmentBase/EquipmentBase.h"
#include "Characters/Enemy/Humanoid/Phantom/Phantom.h"
#include "Characters/Enemy/Cover/EnemyCoverPoint.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Tests/AutomationEditorCommon.h"
#include "Tests/AutomationCommon.h"
#include "Editor.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "ImageUtils.h"
#include "Components/SceneCaptureComponent2D.h"
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
	if (UBlendSpace* Patrol = LoadObject<UBlendSpace>(nullptr,
		TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_RelaxedPatrol2D.BS_Phantom_RelaxedPatrol2D")))
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
		TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Idle_IP.W2_Stand_Relaxed_Idle_IP")))
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
			HasOverride(*FString::Printf(TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/W2_Stand_Relaxed_Fgt_v%d_IP.W2_Stand_Relaxed_Fgt_v%d_IP"), Index, Index)));
	}

	UBlendSpace* Aim = LoadObject<UBlendSpace>(nullptr,
		TEXT("/Game/Enemy/Phantom/OriginalRifle/Animations/BlendSpace/BS_Phantom_AimLocomotion.BS_Phantom_AimLocomotion"));
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
				TEXT("/Game/Enemy/Phantom/Blueprint/BP_Phantom.BP_Phantom_C"));
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
	Capture->FOVAngle = 110.f;
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

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FValidatePlayerViewmodelPIECommand, FAutomationTestBase*, Test);
bool FValidatePlayerViewmodelPIECommand::Update()
{
	UWorld* World = GEditor ? GEditor->PlayWorld : nullptr;
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	AFPSCharacterBase* Player = PC ? Cast<AFPSCharacterBase>(PC->GetPawn()) : nullptr;
	if (!Player || !Player->GetHeadCamera() || !Player->GetViewmodelRoot() || !Player->GetArmsMesh()) return false;

	Test->TestTrue(TEXT("Gameplay camera remains 110 degree FOV"),
		FMath::IsNearlyEqual(Player->GetHeadCamera()->FieldOfView, 110.f));
	Test->TestEqual(TEXT("ViewmodelRoot is attached directly to HeadCamera"),
		Player->GetViewmodelRoot()->GetAttachParent(), static_cast<USceneComponent*>(Player->GetHeadCamera()));
	Test->TestEqual(TEXT("ArmsViewMesh is attached to ViewmodelRoot"),
		Player->GetArmsMesh()->GetAttachParent(), Player->GetViewmodelRoot());
	Test->TestTrue(TEXT("Final viewmodel location matches approved framing"),
		Player->GetViewmodelRoot()->GetRelativeLocation().Equals(FVector(-30.f, 3.f, -4.f), 0.01f));
	Test->TestTrue(TEXT("Final viewmodel rotation preserves imported pose orientation"),
		Player->GetViewmodelRoot()->GetRelativeRotation().Equals(FRotator(0.f, -12.f, 0.f), 0.01f));

	UEquipmentManagerComponent* EquipmentManager = Player->GetEquipmentManager();
	AEquipmentBase* Equipment = EquipmentManager ? EquipmentManager->GetCurrentEquipment() : nullptr;
	Test->TestNotNull(TEXT("Player has initial equipment"), Equipment);
	if (Equipment)
	{
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

#endif
