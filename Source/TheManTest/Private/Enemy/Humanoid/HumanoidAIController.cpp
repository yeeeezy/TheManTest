#include "Enemy/Humanoid/HumanoidAIController.h"
#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Enemy/Humanoid/HumanoidEnemyTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

const FName AHumanoidAIController::BB_TargetActor              = "TargetActor";
const FName AHumanoidAIController::BB_LastKnownPlayerLocation  = "LastKnownPlayerLocation";

AHumanoidAIController::AHumanoidAIController()
{
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius                              = 1500.f;
	SightConfig->LoseSightRadius                          = 1800.f;
	SightConfig->PeripheralVisionAngleDegrees             = 60.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*PerceptionComp);
}

FVector AHumanoidAIController::CalculateCombatMoveDestination(const FVector& EnemyLocation,
	const FVector& TargetLocation, float StrafeSign) const
{
	const FVector TargetToEnemy = (EnemyLocation - TargetLocation).GetSafeNormal2D();
	if (TargetToEnemy.IsNearlyZero()) return EnemyLocation;

	const float Distance = FVector::Dist2D(EnemyLocation, TargetLocation);
	const float MinDistance = FMath::Max(0.f, PreferredCombatDistance - CombatDistanceTolerance);
	const float MaxDistance = PreferredCombatDistance + CombatDistanceTolerance;
	float DesiredDistance = PreferredCombatDistance;
	if (Distance < MinDistance) DesiredDistance = MinDistance;
	else if (Distance > MaxDistance) DesiredDistance = MaxDistance;
	else DesiredDistance = Distance;

	// 正值表示当前过远，需要朝目标收拢；负值表示过近，需要后撤。
	const float RadialCorrection = FMath::Clamp(Distance - DesiredDistance,
		-CombatStrafeDistance, CombatStrafeDistance);
	const FVector Tangent(-TargetToEnemy.Y, TargetToEnemy.X, 0.f);
	const float SideScale = Distance < MinDistance ? 0.55f : (Distance > MaxDistance ? 0.65f : 1.f);
	return EnemyLocation - TargetToEnemy * RadialCorrection
		+ Tangent * FMath::Sign(StrafeSign) * CombatStrafeDistance * SideScale;
}

void AHumanoidAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}

	PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
		this, &AHumanoidAIController::OnTargetPerceptionUpdated);
}

void AHumanoidAIController::OnUnPossess()
{
	PerceptionComp->OnTargetPerceptionUpdated.RemoveDynamic(
		this, &AHumanoidAIController::OnTargetPerceptionUpdated);
	Super::OnUnPossess();
}

FPathFollowingRequestResult AHumanoidAIController::MoveTo(const FAIMoveRequest& MoveRequest,
	FNavPathSharedPtr* OutPath)
{
	// 公共 BT 的 MoveTo(TargetActor) 只作为进入技能序列的门槛。Aim 时真正的移动
	// 由距离环带战术落点控制，否则两套请求会互相覆盖并退化成直线追击。
	if (MoveRequest.IsMoveToActorRequest())
	{
		if (const AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetPawn());
			Enemy && Enemy->GetAIState() == EHumanoidEnemyAIState::Aim)
		{
			FPathFollowingRequestResult Result;
			Result.Code = EPathFollowingRequestResult::AlreadyAtGoal;
			return Result;
		}
	}
	return Super::MoveTo(MoveRequest, OutPath);
}

void AHumanoidAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetPawn());
	if (!Enemy) return;

	// 战斗中每帧把瞄准点更新为玩家位置：供 AimIK（FEAT-031）与子弹方向使用
	if (Enemy->GetAIState() == EHumanoidEnemyAIState::Aim)
	{
		if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			Enemy->AimTargetWorld = Player->GetActorLocation();
			Enemy->bIsAiming      = true;
			UpdateCombatMovement(*Enemy, *Player);
		}
	}
	else
	{
		Enemy->bIsAiming = false;
	}
}

void AHumanoidAIController::UpdateCombatMovement(AHumanoidEnemy& Enemy, AActor& Target)
{
	UWorld* World = GetWorld();
	if (!World) return;
	if (World->GetTimeSeconds() < NextCombatMoveDecisionTime) return;

	// 以短时连续侧移为主，偶尔换向，避免每个决策周期机械左右抖动。
	if (FMath::FRand() < 0.35f) CurrentStrafeSign *= -1.f;
	const FVector RawDestination = CalculateCombatMoveDestination(
		Enemy.GetActorLocation(), Target.GetActorLocation(), CurrentStrafeSign);
	FNavLocation Projected;
	if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World))
	{
		if (NavSystem->ProjectPointToNavigation(RawDestination, Projected, FVector(150.f, 150.f, 250.f)))
		{
			MoveToLocation(Projected.Location, CombatMoveAcceptanceRadius, true, true, true, false);
		}
	}

	const float MinInterval = FMath::Min(CombatMoveDecisionMinInterval, CombatMoveDecisionMaxInterval);
	const float MaxInterval = FMath::Max(CombatMoveDecisionMinInterval, CombatMoveDecisionMaxInterval);
	NextCombatMoveDecisionTime = World->GetTimeSeconds() + FMath::FRandRange(MinInterval, MaxInterval);
}

void AHumanoidAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 只关心玩家
	if (Actor != UGameplayStatics::GetPlayerPawn(this, 0)) return;

	AHumanoidEnemy* Enemy = Cast<AHumanoidEnemy>(GetPawn());
	if (!Enemy) return;

	UBlackboardComponent* BB = GetBlackboardComponent();

	if (Stimulus.WasSuccessfullySensed())
	{
		// 发现玩家：写黑板目标 + Focus 锁定朝向 + 切战斗（SetAIState(Aim) 内已停巡逻、转 Focus 朝向）
		if (BB) { BB->SetValueAsObject(BB_TargetActor, Actor); }
		SetFocus(Actor);
		NextCombatMoveDecisionTime = 0.f;
		Enemy->SetAIState(EHumanoidEnemyAIState::Aim);
	}
	else
	{
		// 丢失玩家：记录最后位置、清目标，然后启动公共 SearchRush → SearchScan → Patrol 流程。
		if (BB)
		{
			BB->SetValueAsVector(BB_LastKnownPlayerLocation, Stimulus.StimulusLocation);
			BB->ClearValue(BB_TargetActor);
		}
		ClearFocus(EAIFocusPriority::Gameplay);
		Enemy->StartLostTargetSearch(Stimulus.StimulusLocation);
	}
}
