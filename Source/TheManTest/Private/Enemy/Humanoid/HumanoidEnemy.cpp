#include "Enemy/Humanoid/HumanoidEnemy.h"
#include "Enemy/Humanoid/HumanoidEnemyAnimInstance.h"
#include "Enemy/Humanoid/HumanoidAIController.h"
#include "Actors/PatrolPoint.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Enemy/Components/EnemyMagazineComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AHumanoidEnemy::AHumanoidEnemy()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AHumanoidAIController::StaticClass();

	// AEnemyBase 关闭了 Tick；转身逻辑依赖 Tick 旋转，必须重新开启
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh());
	WeaponMesh->CastShadow = false;
	MagazineComponent = CreateDefaultSubobject<UEnemyMagazineComponent>(TEXT("MagazineComponent"));
}

void AHumanoidEnemy::BeginPlay()
{
	Super::BeginPlay();
	SetDesiredMaxWalkSpeed(PatrolWalkSpeed);

	// 运行时重新按配置的 socket 名挂载武器，构造函数里只能 SetupAttachment 到 Mesh 根
	WeaponMesh->AttachToComponent(GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		WeaponAttachSocket);
}

void AHumanoidEnemy::ReactToProjectileHit(AActor* HitInstigator)
{
	Super::ReactToProjectileHit(HitInstigator);
	if (!IsValid(HitInstigator) || IsDead()) return;

	AimTargetWorld = HitInstigator->GetActorLocation();
	bIsAiming = true;
	SetAIState(EHumanoidEnemyAIState::Aim);

	if (AHumanoidAIController* AIC = Cast<AHumanoidAIController>(GetController()))
	{
		AIC->SetFocus(HitInstigator);
		if (UBlackboardComponent* Blackboard = AIC->GetBlackboardComponent())
		{
			Blackboard->SetValueAsObject(AHumanoidAIController::BB_TargetActor, HitInstigator);
		}
	}
}

void AHumanoidEnemy::AimAtTarget(AActor* Target)
{
	// 瞄准目标中心；UseRandomSkill 在激活技能前调用，写入 AimTargetWorld 供子弹方向 + AimIK
	if (Target)
	{
		AimTargetWorld = Target->GetActorLocation();
	}
}

void AHumanoidEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AAIController* AIC = Cast<AAIController>(NewController);
	if (!AIC) return;

	AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
		this, &AHumanoidEnemy::OnPatrolMoveCompleted);

	if (PatrolPoints.IsEmpty()) return;

	// 延迟启动，等 CharacterMovement 完成物理初始化进入 MOVE_Walking
	FTimerHandle StartTimer;
	GetWorldTimerManager().SetTimer(StartTimer, this, &AHumanoidEnemy::MoveToNextPatrolPoint, 0.1f, false);
}

void AHumanoidEnemy::SetAIState(EHumanoidEnemyAIState NewState)
{
	if (AIState == NewState) return;

	const EHumanoidEnemyAIState OldState = AIState;
	AIState = NewState;

	if (NewState == EHumanoidEnemyAIState::Aim)
	{
		// 切入战斗：清除巡逻计时器，停止当前移动，让 BT 接管
		GetWorldTimerManager().ClearTimer(PatrolWaitTimer);
		GetWorldTimerManager().ClearTimer(ScanDelayTimer);
		GetWorldTimerManager().ClearTimer(SearchScanTimer);
		bIsPatrolScanning  = false;
		bIsStoppingAtPoint = false;
		// 战斗朝向由 AIController Focus 驱动，关闭速度方向自动旋转
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw                         = true;
		SetDesiredMaxWalkSpeed(CombatWalkSpeed);
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->StopMovement();
		}
	}
	else if (NewState == EHumanoidEnemyAIState::SearchRush)
	{
		GetWorldTimerManager().ClearTimer(PatrolWaitTimer);
		GetWorldTimerManager().ClearTimer(ScanDelayTimer);
		GetWorldTimerManager().ClearTimer(SearchScanTimer);
		bIsPatrolScanning = false;
		bIsStoppingAtPoint = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		SetDesiredMaxWalkSpeed(SearchRushSpeed);
	}
	else if (NewState == EHumanoidEnemyAIState::SearchScan)
	{
		GetCharacterMovement()->StopMovementImmediately();
		SetDesiredMaxWalkSpeed(0.f);
		bIsStoppingAtPoint = true;
		bIsPatrolScanning = true;
	}
	else if (NewState == EHumanoidEnemyAIState::Patrol)
	{
		// 回到巡逻：恢复速度方向自动旋转
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw                         = false;
		SetDesiredMaxWalkSpeed(PatrolWalkSpeed);
		// 战斗或搜索结束后都从最近路点恢复，避免折返到旧索引。
		bNeedsPatrolResume = true;
	}
}

void AHumanoidEnemy::StartLostTargetSearch(const FVector& LastKnownLocation)
{
	if (IsDead()) return;

	SearchDestination = LastKnownLocation;
	// 先在旧状态下终止 BT/战斗遗留 Move，避免它的 Abort 回调被误判成搜索 Move 失败。
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
	}
	SetAIState(EHumanoidEnemyAIState::SearchRush);

	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		FAIMoveRequest MoveRequest(SearchDestination);
		MoveRequest.SetAcceptanceRadius(SearchAcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		MoveRequest.SetAllowPartialPath(true);
		MoveRequest.SetProjectGoalLocation(true);
		MoveRequest.SetCanStrafe(false);
		const FPathFollowingRequestResult Result = AIC->MoveTo(MoveRequest);
		ActiveSearchMoveRequestId = Result.Code == EPathFollowingRequestResult::RequestSuccessful
			? Result.MoveId : FAIRequestID::InvalidRequest;
		if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
		{
			HandleSearchArrival();
		}
	}
}

void AHumanoidEnemy::RequestTurn(float Angle)
{
	PendingTurnAngle = Angle;
	bPendingTurn = true;
	TargetTurnYaw = GetActorRotation().Yaw + Angle;
	PendingTurnElapsed = 0.f;
	PendingTurnTimeout = FMath::Abs(Angle) / FMath::Max(TurnRotationSpeed, 1.f) + TurnCompletionGraceSeconds;

	// 关闭自动朝向，防止 PhysicsRotation 每帧覆盖 SetActorRotation
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetDesiredMaxWalkSpeed(TurnWalkSpeed);
}

void AHumanoidEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bPendingTurn)
	{
		PendingTurnElapsed += DeltaSeconds;
		FRotator Current = GetActorRotation();
		Current.Yaw = FMath::FixedTurn(Current.Yaw, TargetTurnYaw, TurnRotationSpeed * DeltaSeconds);
		SetActorRotation(Current);
		if (FMath::Abs(FMath::FindDeltaAngleDegrees(Current.Yaw, TargetTurnYaw)) <= TurnCompletionTolerance
			|| PendingTurnElapsed >= PendingTurnTimeout)
		{
			OnTurnComplete();
		}
	}

	// 切换动作方案（已注释）

	// 摩擦力方案：接近路点时提前线性降速，让 AI 停车时角色已处于低速，blend space 有减速过程可播
	if (!bIsStoppingAtPoint && !bPendingTurn && PatrolPoints.IsValidIndex(CurrentPatrolIndex))
	{
		const float Dist = FVector::Dist2D(GetActorLocation(), PatrolPoints[CurrentPatrolIndex]->GetActorLocation());
		if (AIState == EHumanoidEnemyAIState::Patrol && ActivePatrolMoveRequestId.IsValid()
			&& Dist <= PatrolAcceptanceRadius + 10.f)
		{
			ActivePatrolMoveRequestId = FAIRequestID::InvalidRequest;
			bIsStoppingAtPoint = true;
			if (AAIController* AIC = Cast<AAIController>(GetController())) AIC->StopMovement();
			GetWorldTimerManager().SetTimerForNextTick(this, &AHumanoidEnemy::HandlePatrolArrival);
			return;
		}
		if (Dist < SlowdownRadius)
		{
			const float Alpha    = FMath::Clamp(Dist / SlowdownRadius, 0.f, 1.f);
			const float NewSpeed = FMath::Lerp(MinApproachSpeed, PatrolWalkSpeed, Alpha);
			SetDesiredMaxWalkSpeed(NewSpeed);
		}
		else
		{
			SetDesiredMaxWalkSpeed(PatrolWalkSpeed);
		}
	}
}

void AHumanoidEnemy::OnTurnComplete()
{
	if (!bPendingTurn) return;
	SetActorRotation(FRotator(0.f, TargetTurnYaw, 0.f));
	bPendingTurn = false;
	PendingTurnAngle = 0.f;
	PendingTurnElapsed = 0.f;
	PendingTurnTimeout = 0.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	MoveToNextPatrolPoint();
}

void AHumanoidEnemy::ConfigurePatrolPoints(const TArray<APatrolPoint*>& InPatrolPoints, bool bStartImmediately)
{
	if (AAIController* AIC = Cast<AAIController>(GetController())) AIC->StopMovement();
	ActivePatrolMoveRequestId = FAIRequestID::InvalidRequest;
	PatrolPoints.Reset(InPatrolPoints.Num());
	for (APatrolPoint* Point : InPatrolPoints)
	{
		if (IsValid(Point)) PatrolPoints.Add(Point);
	}
	CurrentPatrolIndex = FindNearestPatrolPointIndex();
	PatrolArrivalCount = 0;
	if (bStartImmediately && AIState == EHumanoidEnemyAIState::Patrol && !PatrolPoints.IsEmpty())
	{
		MoveToNextPatrolPoint();
	}
}

float AHumanoidEnemy::GetPatrolWaitRemaining() const
{
	return GetWorld() ? GetWorld()->GetTimerManager().GetTimerRemaining(PatrolWaitTimer) : -1.f;
}

FVector AHumanoidEnemy::GetCurrentPatrolTargetLocation() const
{
	return PatrolPoints.IsValidIndex(CurrentPatrolIndex) && IsValid(PatrolPoints[CurrentPatrolIndex])
		? PatrolPoints[CurrentPatrolIndex]->GetActorLocation() : FVector::ZeroVector;
}

void AHumanoidEnemy::MoveToNextPatrolPoint()
{
	if (AIState != EHumanoidEnemyAIState::Patrol) return;
	bIsStoppingAtPoint = false;
	if (IsDead() || !PatrolPoints.IsValidIndex(CurrentPatrolIndex)) return;

	if (GetCharacterMovement()->MovementMode != MOVE_Walking)
	{
		FTimerHandle RetryTimer;
		GetWorldTimerManager().SetTimer(RetryTimer, this, &AHumanoidEnemy::MoveToNextPatrolPoint, 0.1f, false);
		return;
	}

	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;

	SetDesiredMaxWalkSpeed(PatrolWalkSpeed);

	FAIMoveRequest MoveRequest(PatrolPoints[CurrentPatrolIndex]->GetActorLocation());
	MoveRequest.SetAcceptanceRadius(PatrolAcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);
	MoveRequest.SetAllowPartialPath(true);
	MoveRequest.SetProjectGoalLocation(true);
	MoveRequest.SetCanStrafe(false);
	const FPathFollowingRequestResult Result = AIC->MoveTo(MoveRequest);
	ActivePatrolMoveRequestId = Result.Code == EPathFollowingRequestResult::RequestSuccessful
		? Result.MoveId : FAIRequestID::InvalidRequest;
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// Never recurse synchronously through Arrival -> next MoveTo -> AlreadyAtGoal.
		// Closely projected/repeated patrol points must advance on a later frame.
		GetWorldTimerManager().SetTimerForNextTick(this, &AHumanoidEnemy::HandlePatrolArrival);
	}
}

void AHumanoidEnemy::TryTurnOrMove()
{
	if (PatrolPoints.IsValidIndex(CurrentPatrolIndex))
	{
		const FVector MyForward = GetActorForwardVector().GetSafeNormal2D();
		const FVector ToNext    = (PatrolPoints[CurrentPatrolIndex]->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		const float   DotVal    = FMath::Clamp(FVector::DotProduct(MyForward, ToNext), -1.f, 1.f);
		float         Angle     = FMath::RadiansToDegrees(FMath::Acos(DotVal));
		if (FVector::CrossProduct(MyForward, ToNext).Z < 0.f) Angle = -Angle;

		if (FMath::Abs(Angle) > TurnAngleThreshold)
		{
			RequestTurn(Angle);
			return; // OnTurnComplete 负责调 MoveToNextPatrolPoint
		}
	}
	MoveToNextPatrolPoint();
}

void AHumanoidEnemy::OnPatrolWaitFinished()
{
	bIsPatrolScanning = false;
	TryTurnOrMove();
}

void AHumanoidEnemy::OnSearchScanFinished()
{
	bIsPatrolScanning = false;
	bIsStoppingAtPoint = false;
	SetAIState(EHumanoidEnemyAIState::Patrol);
	ResumeNearestPatrol();
}

void AHumanoidEnemy::HandleSearchArrival()
{
	SetAIState(EHumanoidEnemyAIState::SearchScan);
	if (SearchScanDuration > 0.f)
	{
		GetWorldTimerManager().SetTimer(SearchScanTimer, this,
			&AHumanoidEnemy::OnSearchScanFinished, SearchScanDuration, false);
	}
	else
	{
		OnSearchScanFinished();
	}
}

void AHumanoidEnemy::HandlePatrolArrival()
{
	if (AIState != EHumanoidEnemyAIState::Patrol || PatrolPoints.IsEmpty() ||
		!PatrolPoints.IsValidIndex(CurrentPatrolIndex)) return;
	bIsStoppingAtPoint = true;
	++PatrolArrivalCount;

	const float WaitTime = PatrolPoints[CurrentPatrolIndex]->WaitTime;
	CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
	if (WaitTime > 0.f)
	{
		if (WaitTime >= MinScanWaitTime)
		{
			GetWorldTimerManager().SetTimer(ScanDelayTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				bIsPatrolScanning = true;
			}), 0.4f, false);
		}
		GetWorldTimerManager().SetTimer(PatrolWaitTimer, this, &AHumanoidEnemy::OnPatrolWaitFinished, WaitTime, false);
	}
	else
	{
		TryTurnOrMove();
	}
}

void AHumanoidEnemy::ResumeNearestPatrol()
{
	if (!bNeedsPatrolResume) return;
	bNeedsPatrolResume  = false;
	CurrentPatrolIndex  = FindNearestPatrolPointIndex();
	MoveToNextPatrolPoint();
}

int32 AHumanoidEnemy::FindNearestPatrolPointIndex() const
{
	if (PatrolPoints.IsEmpty()) return 0;

	float  MinDist      = TNumericLimits<float>::Max();
	int32  NearestIndex = 0;
	const FVector MyLoc = GetActorLocation();

	for (int32 i = 0; i < PatrolPoints.Num(); ++i)
	{
		if (!PatrolPoints[i]) continue;
		const float Dist = FVector::Dist2D(MyLoc, PatrolPoints[i]->GetActorLocation());
		if (Dist < MinDist)
		{
			MinDist      = Dist;
			NearestIndex = i;
		}
	}
	return NearestIndex;
}

void AHumanoidEnemy::OnPatrolMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (AIState == EHumanoidEnemyAIState::SearchRush)
	{
		if (!ActiveSearchMoveRequestId.IsEquivalent(RequestID)) return;
		ActiveSearchMoveRequestId = FAIRequestID::InvalidRequest;
		if (Result.IsSuccess())
		{
			HandleSearchArrival();
		}
		else
		{
			SetAIState(EHumanoidEnemyAIState::Patrol);
			ResumeNearestPatrol();
		}
		return;
	}

	if (AIState != EHumanoidEnemyAIState::Patrol) return;
	if (!ActivePatrolMoveRequestId.IsEquivalent(RequestID)) return;
	ActivePatrolMoveRequestId = FAIRequestID::InvalidRequest;
	if (Result.IsSuccess())
	{
		HandlePatrolArrival();
	}
	else
	{
		FTimerHandle RetryTimer;
		GetWorldTimerManager().SetTimer(RetryTimer, this, &AHumanoidEnemy::MoveToNextPatrolPoint, 0.2f, false);
	}
}
